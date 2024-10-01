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

        if (status_message.find('|') != std::string::npos) { //npos represents end of string
            break;

        }
    }

    std::cerr << "Finished receiving status message: " << status_message << std::endl;

    // Return the message up to the delimiter
    return status_message.substr(0, status_message.find('|'));
}
