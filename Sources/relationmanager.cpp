#include "Headers/relationmanager.h"
#include "Headers/networkmanager.h"
relationmanager::relationmanager(int user_id) {

    relation_network_manager = new networkmanager();
    relation_manager_socket = relation_network_manager->setup_socket("RELATION_MANAGEMENT", user_id);


}

void relationmanager::async_manage_requests(const int &relation_manager_socket, MainWindow* mainwindow)
{

    //listen and make updates accordingly

    //should a signal be recieved, just call setup_friend_requests()
}

std::string getValueByKey(const std::vector<std::pair<std::string, int>> &pairs, const int key)
{
    for (const auto&p : pairs)
    {
        if (p.second == key)
        {
            return std::to_string(p.second);
        }
    }
    return "ERROR GETTING KEY";
}

int relationmanager::get_relation_manager_socket()
{
    return relation_manager_socket;
}
void relationmanager::set_friends_list_mem(std::vector<std::pair<std::string, int> > friends_list)
{
    friends_list_in_memory = friends_list;
}

std::vector<std::pair<std::string, int>> relationmanager::get_friends_list_mem()
{
    return friends_list_in_memory;
}

void relationmanager::set_temp_convo_list(std::vector<int> convo_arr)
{
    temp_conversation_list = convo_arr;
}

std::vector<int> relationmanager::get_temp_convo_list()
{
    return temp_conversation_list;
}

void relationmanager::clear_temp_convo_list()
{
    std::vector<int> temp;
    temp_conversation_list = temp;
}

void relationmanager::add_to_temp_convo_list(int user_id)
{
    temp_conversation_list.push_back(user_id);
}

void relationmanager::remove_from_temp_convo_list(int user_id)
{
    temp_conversation_list.erase(std::remove(temp_conversation_list.begin(), temp_conversation_list.end(), user_id), temp_conversation_list.end());
}

int relationmanager::update_friends_list_mem()
{
    std::vector<std::pair<std::string, int>> temp_list = pull_friends_list(get_relation_manager_socket());
    if(temp_list == friends_list_in_memory && !temp_list.empty())
    {
        std::cout << "No new friends" << std::endl;
        return 1;
    }
    else if (temp_list.empty())
    {
        std::cerr << "Failed to update friends list" << std::endl;
        return -1;
    }
    else
    {
        friends_list_in_memory = temp_list;
        std::cout << "Successfully updated friends list" << std::endl;
        return 0;
    }
}
int relationmanager::insert_new_conversation(std::vector<int> member_list)
{
    //send to server
    std::string msg_type = "new_conversation\n";

    for(auto &m : member_list)
    {
        std::cout << "m: " << m << std::endl;
    }

    std::string temp_buffer = "";
    for(auto k : member_list)
    {
        temp_buffer += std::to_string(k) + "-";
    }
    temp_buffer += "|";
    std::cout << "Buffer to be sent: " << temp_buffer << std::endl;

    ssize_t bytes_sent = send(relation_manager_socket, msg_type.c_str(), msg_type.size(), 0);

    if (bytes_sent >  -1)
    {
        ssize_t bytes_sent_sec = send(relation_manager_socket, temp_buffer.c_str(), temp_buffer.size(), 0);
        //get confirmation as error handling
        if(bytes_sent_sec > -1)
        {
            std::cout << "Successfull insertion" << std::endl;
            return 0;
        }
    }

    else {
        std::cout << "Failed to insert new conversation" << std::endl;
        return -1;
    }
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
    bool last_iteration = false;

    // Send the request to the server
    if (send(client_socket, msg_type.c_str(), msg_type.size(), 0) == -1)
    {
        std::cerr << "Unable to establish connection with server: ERROR receiving friend requests" << std::endl;
        return empty_friend_requests;
    }

    while (true)
    {
        char buffer[100] = {0};  // Ensure buffer is initialized to 0
        std::string user_id_data;

        // Inner loop to collect data until "|" is found
        while (true)
        {
            std::cout << "Receiving friend requests" << std::endl;
            ssize_t status_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

            if (status_bytes < 0)
            {
                std::cerr << "Error receiving data from server" << std::endl;
                return empty_friend_requests;
            }
            else if (status_bytes == 0)
            {
                std::cerr << "Friend request connection closed by server" << std::endl;
                return empty_friend_requests;
            }

            buffer[status_bytes] = '\0';
            user_id_data += buffer;

            std::cout << "User ID Data: " << user_id_data << std::endl;

            // Check if the message contains the delimiter "|"
            if (user_id_data.find("|") != std::string::npos || user_id_data.find("-") != std::string::npos)
            {
                break;
            }
        }

        if(user_id_data.back() == '-')
        {
            last_iteration = true;
        }

        // If termination signal "-" is received, break the outer loop
        if (user_id_data == "-")
        {
            std::cout << "Termination signal received. Stopping reception of friend requests." << std::endl;
            break;
        }

        // Process the friend request if it's not empty or the termination signal
        if (user_id_data.size() > 1)
        {
            int friend_id = std::stoi(user_id_data.substr(0, user_id_data.find("+")));
            std::string friend_username = user_id_data.substr(user_id_data.find("+") + 1, user_id_data.find("|") - user_id_data.find("+") - 1);
            friend_requests.push_back({friend_username, friend_id});
        }

        // Clear the user_id_data after processing each entry
        user_id_data.clear();
    }

    return friend_requests;
}

std::vector<std::pair<std::string, int>> relationmanager::pull_outbound_friend_requests(int client_socket)
{
    std::string msg_type = "get_outbound_friends\n";
    std::vector<std::pair<std::string, int>> friend_requests;
    std::vector<std::pair<std::string, int>> empty_friend_requests;

    // Send the request to the server
    if (send(client_socket, msg_type.c_str(), msg_type.size(), 0) == -1)
    {
        std::cerr << "Unable to establish connection with server: ERROR receiving friend requests" << std::endl;
        return empty_friend_requests;
    }

    while (true)
    {
        char buffer[100] = {0};  // Ensure buffer is initialized to 0
        std::string user_id_data;

        // Inner loop to collect data until "|" is found
        while (true)
        {
            std::cout << "Receiving friend requests" << std::endl;
            ssize_t status_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

            if (status_bytes < 0)
            {
                std::cerr << "Error receiving data from server" << std::endl;
                return empty_friend_requests;
            }
            else if (status_bytes == 0)
            {
                std::cerr << "Friend request connection closed by server" << std::endl;
                return empty_friend_requests;
            }

            buffer[status_bytes] = '\0';
            user_id_data += buffer;

            std::cout << "User ID Data: " << user_id_data << std::endl;

            // Check if the message contains the delimiter "|"
            if (user_id_data.find("|") != std::string::npos || user_id_data.find("-") != std::string::npos)
            {
                break;
            }
        }


        // If termination signal "-" is received, break the outer loop
        if (user_id_data == "-")
        {
            std::cout << "Termination signal received. Stopping reception of friend requests." << std::endl;
            break;
        }

        // Process the friend request if it's not empty or the termination signal
        if (user_id_data.size() > 1)
        {
            int friend_id = std::stoi(user_id_data.substr(0, user_id_data.find("+")));
            std::string friend_username = user_id_data.substr(user_id_data.find("+") + 1, user_id_data.find("|") - user_id_data.find("+") - 1);
            friend_requests.push_back({friend_username, friend_id});
        }

        // Clear the user_id_data after processing each entry
        user_id_data.clear();
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
    else {return -1;}
}

std::vector<std::pair<std::string, int>> relationmanager::pull_friends_list(int client_socket)
{
    std::string msg_type = "get_friends\n";
    std::vector<std::pair<std::string, int>> friends_list;
    std::vector<std::pair<std::string, int>> empty_friends_list;
    std::string user_id_data;  // Declare outside to accumulate data across recv() calls

    // Send the request to the server
    if (send(client_socket, msg_type.c_str(), msg_type.size(), 0) == -1)
    {
        std::cerr << "Unable to establish connection with server: ERROR receiving friends list" << std::endl;
        return empty_friends_list;
    }

    while (true)
    {
        std::cout << "pull_friends_list LOOP" << std::endl;
        char buffer[100] = {0};  // Ensure buffer is initialized to 0

        // Receive data from the server
        ssize_t status_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (status_bytes < 0)
        {
            std::cerr << "Error receiving data from server" << std::endl;
            return empty_friends_list;
        }
        else if (status_bytes == 0)
        {
            std::cerr << "Friends list connection closed by server" << std::endl;
            return empty_friends_list;
        }

        buffer[status_bytes] = '\0';
        user_id_data += buffer;

        std::cout << "User ID Data: " << user_id_data << std::endl;

        // Process all complete messages in user_id_data
        size_t pos;
        while ((pos = user_id_data.find("|")) != std::string::npos)
        {
            std::string message = user_id_data.substr(0, pos);
            user_id_data.erase(0, pos + 1);  // Remove processed message including '|'

            if (message == "-")
            {
                std::cout << "Termination signal received. Stopping reception of friends list." << std::endl;
                return friends_list;
            }

            if (!message.empty())
            {
                size_t plus_pos = message.find("+");
                if (plus_pos == std::string::npos)
                {
                    std::cerr << "Invalid message format: " << message << std::endl;
                    continue;
                }

                int friend_id = std::stoi(message.substr(0, plus_pos));
                std::string friend_username = message.substr(plus_pos + 1);

                friends_list.push_back({friend_username, friend_id});
                std::cout << "Parsed Friend - ID: " << friend_id << ", Username: " << friend_username << std::endl;
            }
        }

        if (user_id_data == "-")
        {
            std::cout << "Termination signal received. Stopping reception of friends list." << std::endl;
            return friends_list;
        }

        // If user_id_data contains other data without a complete message, continue to the next iteration to receive more data
    }

    // This point should never be reached
    return friends_list;
}

std::vector<std::vector<std::pair<std::string, int>>> relationmanager::pull_user_conversations()
{
    std::string msg_type = "get_user_convos\n";
    std::vector<std::vector<std::pair<std::string, int>>> conversation_list;
    std::vector<std::vector<std::pair<std::string, int>>> empty_conversation_list;
    std::string inbound_data;

    if (send(get_relation_manager_socket(), msg_type.c_str(), msg_type.size(), 0) == -1)
    {
        std::cerr << "Unable to establish connection with server: ERROR receiving conversations" << std::endl;
        return empty_conversation_list;
    }
    bool flag = false;

    while (!flag)
    {
        std::vector<std::pair<std::string, int>> trunc_convo_vectr;

        while (true)
        {
            char buffer[999] = {0};
            ssize_t status_bytes = recv(get_relation_manager_socket(), buffer, sizeof(buffer) - 1, 0);

            if (status_bytes < 0)
            {
                std::cerr << "Error receiving data from the server" << std::endl;
                return empty_conversation_list;
            }
            else if (status_bytes == 0)
            {
                std::cerr << "Conversation pull connection closed by server" << std::endl;
                return empty_conversation_list;
            }

            buffer[status_bytes] = '\0';
            inbound_data += buffer;

            size_t pos;
            while ((pos = inbound_data.find("|")) != std::string::npos)
            {
                std::string mess = inbound_data.substr(0, pos);
                inbound_data.erase(0, pos + 1);

                if (mess == "--")
                {
                    flag = true; // Signal to stop receiving conversations
                    break;
                }

                if (mess == "-")
                {
                    // End of the current conversation, add it to the list
                    if (!trunc_convo_vectr.empty())
                    {
                        conversation_list.push_back(trunc_convo_vectr);
                    }
                    break; // Exit the inner while loop to process the next conversation
                }

                if (!mess.empty())
                {
                    size_t plus_pos = mess.find("+");
                    if (plus_pos == std::string::npos)
                    {
                        std::cerr << "Invalid message format: " << mess << std::endl;
                        continue;
                    }

                    // Parse the ID and username
                    int conversation_user_id = std::stoi(mess.substr(0, plus_pos));
                    std::string conversation_user_username = mess.substr(plus_pos + 1);
                    trunc_convo_vectr.push_back({conversation_user_username, conversation_user_id});
                    std::cout << "Parsed Friend - ID: " << conversation_user_id << ", Username: " << conversation_user_username << std::endl;
                }
            }

            // If we received the "-" termination signal, break out of the outer loop
            if (inbound_data == "-")
            {
                break;
            }

            // If we received the "--" termination signal, break out of both loops
            if (inbound_data == "--")
            {
                flag = true;
                break;
            }
        }
    }

    return conversation_list;
}



int relationmanager::update_conversations_glob()
{
    std::vector<std::vector<std::pair<std::string, int>>> temp_conversations = pull_user_conversations();
    if (temp_conversations == conversations_mem_load)
    {
        //nothing to update/broken update
        return -1;
    }
    else
    {
        conversations_mem_load = temp_conversations;
        return 0;
    }
}




















