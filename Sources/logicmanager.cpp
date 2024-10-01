#include "Headers/logicmanager.h"
#include <string>
logicmanager::logicmanager() {

    logic_network_manager = new networkmanager;

    logic_manager_socket = logic_network_manager->setup_socket("LOGIC_MANAGEMENT", -1);
}

int logicmanager::get_logic_manager_socket()
{
    return logic_manager_socket;
}


void logicmanager::async_handle_auth_and_infrastructure(const int &logic_manager_socket)
{
    //thread this to handle login and shit
    //thread to recieve
    char buffer[1024];
    std::string accumulated_data;
    while (true) {

        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(logic_manager_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "Error receiving data or connection closed by server" << std::endl;
            break;
        }

        accumulated_data.append(buffer, bytes_received);

        size_t pos;
        while ((pos = accumulated_data.find('\n')) != std::string::npos) {
            std::string message = accumulated_data.substr(0, pos + 1);
            accumulated_data.erase(0, pos + 1);

            if (!message.empty() && message.back() == '\n') {
                message.pop_back();
            }

            // Use Qt's signals/slots mechanism in the MainWindow class to update the UI
            QString qMessage = QString::fromStdString(message);
        }
    }
}

void logicmanager::verify_login(int client_socket, const std::string &verify_combined)
{
    //types are image, text, verify_user, new_user
    std::string buffer = "verify_user\n" + verify_combined;

    size_t buffer_size = buffer.size();

    if (send(client_socket, buffer.c_str(), buffer_size, 0) == -1)
    {
        std::cerr << "Error sending image" << std::endl;
    }

}

void logicmanager::verify_new_account(int conn_socket, const std::string &user_pw)
{
    //types are image, text, verify_user, new_user
    std::string buffer = "new_user\n" + user_pw;

    size_t buffer_size = buffer.size();

    if (send(conn_socket, buffer.c_str(), buffer_size, 0) == -1)
    {
        std::cerr << "Error sending image" << std::endl;
    }

}
std::string logicmanager::get_status(int client_socket) {
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
