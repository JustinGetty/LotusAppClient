#include "Headers/relationmanager.h"
#include "Headers/networkmanager.h"
relationmanager::relationmanager(int user_id) {

    relation_network_manager = new networkmanager();
    relation_manager_socket = relation_network_manager->setup_socket("RELATION_MANAGEMENT", user_id);


}

void relationmanager::async_manage_requests(const int &relation_manager_socket, MainWindow* mainwindow)
{

    //listen and make updates accordingly
}


int relationmanager::get_relation_manager_socket()
{
    return relation_manager_socket;
}

void relationmanager::send_friend_request(int socket, const std::string &data_to_send)
{
    //types are image, text, verify_user, new_user
    std::string buffer = "new_friend_request\n" + data_to_send;

    size_t buffer_size = buffer.size();

    if (send(socket, buffer.c_str(), buffer_size, 0) == -1)
    {
        std::cerr << "Error sending image" << std::endl;
    }

}

std::string relationmanager::get_request_status(int client_socket) {
    char buffer[45];
    std::string status_message;

    while (true) {
        std::cout << "Getting status good" << std::endl;
        ssize_t status_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (status_bytes < 0) {

            return "";
        } else if (status_bytes == 0) {
            std::cerr << "Connection closed by the server." << std::endl;
            return "";
        }
        buffer[status_bytes] = '\0';

        std::cout << buffer << std::endl;
        status_message += buffer;

        if (status_message.find('|') != std::string::npos) { //npos is not found return type of find()
            break;

        }
    }

    std::cerr << "Finished receiving status message: " << status_message << std::endl;

    // Return the message up to the delimiter
    return status_message.substr(0, status_message.find('|'));
}

std::vector<std::pair<std::string, int>> relationmanager::pull_inbound_friend_requests(int client_socket)
{
    std::string msg_type = "get_incoming_friends\n";
    std::vector<std::pair<std::string, int>> friend_requests;
    std::vector<std::pair<std::string, int>> empty_friend_requests;
    if (send(client_socket, msg_type.c_str(), msg_type.size(), 0) == -1)
    {
        std::cerr << "Unable to establish connect with server: ERROR receiving friend requests" << std::endl;

        //error handle on the other end by checking .empty() on return of this function
        return empty_friend_requests;
    }

    while (true)
    {
        char buffer[100];
        std::string user_id_data;

        while (true)
        {
            std::cout << "Receiving friend requests" << std::endl;
            ssize_t status_bytes = recv(client_socket, &buffer, sizeof(buffer) - 1, 0);
            if(status_bytes < 0)
            {
                return empty_friend_requests;
            } else if (status_bytes == 0)
            {
                std::cerr << "Friend request connection close by server" << std::endl;
                return empty_friend_requests;
            } else if (status_bytes == 1 && friend_requests.empty())
            {
                return empty_friend_requests;
            }
            buffer[status_bytes] = '\0';
            std::cout << "friend buffer: " << buffer << std::endl;
            user_id_data += buffer;

            //incoming date needs to look like ID+USERNAME| and at the end of receiving send a - character
            if (user_id_data.find("|") != std::string::npos) //npos = not found
            {
                break;
            }
        }
        if (user_id_data.size() > 1)
        {
            int friend_id = std::stoi(user_id_data.substr(0, user_id_data.find("+")));
            std::string friend_username = user_id_data.substr(user_id_data.find("+") + 1, user_id_data.find("|") - user_id_data.find("+") - 1);
            friend_requests.push_back({friend_username, friend_id});
        }

        if (user_id_data.find("-") != std::string::npos)
        {
            break;
        }

    }

    return friend_requests;
}

int relationmanager::fetch_user_id_from_server(const std::string &username)
{
    char buffer[8];
    std::string received_data;
    std::string type = "get_user_id\n";

    std::string username_tot = type + username + "|";
    std::cout << "Sending message: " << username_tot << std::endl;

    std::cout << "relation manager socket: " << relation_manager_socket << std::endl;
    send(relation_manager_socket, username_tot.c_str(), username_tot.size(), 0);

    while (true)
    {
        int bytes_received = recv(relation_manager_socket, buffer, sizeof(buffer) - 1, 0);
        std::cerr << "Fetching user id, bytes received: " << bytes_received << std::endl;

        if (bytes_received < 0)
        {
            std::cerr << "Error receiving data" << std::endl;
            return -1;
        }
        else if (bytes_received == 0)
        {
            std::cerr << "Connection closed by server" << std::endl;
            return -1;
        }

        received_data.append(buffer, bytes_received);

        std::cout << "Received user_id data from server: " << received_data << std::endl;

        if (received_data.find("|") != std::string::npos)
        {
            break;
        }
    }

    // Extract the user id from the received data
    int id = std::stoi(received_data.substr(0, received_data.find("|")));
    return id;
}

int relationmanager::send_friend_update(const int &relation_socket, const int &sender_id, const std::string &type)
{
    send(relation_socket, type.c_str(), type.size(), 0);
    std::string msg = std::to_string(sender_id) + "|";
    send(relation_socket, msg.c_str(), msg.size(), 0);

    //maybe needs object call
    std::string result = get_request_status(relation_socket);
    if (result == "accept_failed")
    {
        return -1;
    } else if (result == "accept_succeeded")
    {
        return 0;
    }
}



















