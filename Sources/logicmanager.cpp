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

int logicmanager::fetch_user_id_from_server(const std::string &username)
{
    char buffer[12];
    std::string received_data;
    std::string type = "get_user_id\n";

    std::string username_tot = type + username + "|";
    std::cout << "Sending message: " << username_tot << std::endl;

    send(logic_manager_socket, username_tot.c_str(), username_tot.size(), 0);

    while (true)
    {
        int bytes_received = recv(logic_manager_socket, buffer, sizeof(buffer) - 1, 0);
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
int logicmanager::send_profile_picture_change(const QByteArray &imgByteArray)
{
    std::string header = "upload_pfp\n";
    if (send(logic_manager_socket, header.c_str(), header.size(), 0) < 0)
    {
        std::cerr << "Failed to send the header." << std::endl;
        return -1;
    }

    uint64_t image_size = imgByteArray.size();
    if (send(logic_manager_socket, &image_size, sizeof(image_size), 0) < 0)
    {
        std::cerr << "Failed to send image size." << std::endl;
        return -1;
    }

    if (send(logic_manager_socket, imgByteArray.data(), imgByteArray.size(), 0) < 0)
    {
        std::cerr << "Failed to send image data." << std::endl;
        return -1;
    }

    std::cout << "Profile picture sent successfully" << std::endl;
    return 0;
}

QPixmap logicmanager::pull_profile_picture()
{
    QPixmap profile_picture;

    int image_size = 0;
    // Send the request header
    std::string header = "get_profile_pic\n";
    if (send(logic_manager_socket, header.c_str(), header.size(), 0) < 0)
    {
        std::cerr << "Failed to send the header." << std::endl;
        return profile_picture;
    }

    // Receive the image size as a string
    char image_size_buffer[16]; // Buffer to hold the image size as a string
    memset(image_size_buffer, 0, sizeof(image_size_buffer));
    if (recv(logic_manager_socket, image_size_buffer, sizeof(image_size_buffer) - 1, 0) <= 0)
    {
        std::cerr << "Failed to receive image size." << std::endl;
        return profile_picture;
    }

    std::string image_size_str(image_size_buffer);
    image_size_str.erase(std::remove(image_size_str.begin(), image_size_str.end(), '\n'), image_size_str.end()); // Remove any newline characters

    if (image_size_str == "0")
    {
        std::cerr << "No profile picture available." << std::endl;
        return profile_picture; // Return an empty QPixmap if there is no profile picture
    }

    std::cerr << "Successfully received image size: " << image_size_str << std::endl;

    try {
        image_size = std::stoi(image_size_str);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid image size received: " << image_size_str << std::endl;
        return profile_picture; // Return an empty QPixmap on invalid input
    }

    // Allocate a buffer for the image data
    std::vector<char> buffer(image_size);
    size_t total_bytes_rec = 0;
    while (total_bytes_rec < image_size)
    {
        ssize_t bytes_received = recv(logic_manager_socket, buffer.data() + total_bytes_rec, image_size - total_bytes_rec, 0);
        if (bytes_received <= 0)
        {
            std::cerr << "Failed to receive image data." << std::endl;
            return profile_picture; // Return empty QPixmap on failure
        }
        total_bytes_rec += bytes_received;
        std::cout << "Total Bytes Received: " << total_bytes_rec << std::endl;
    }

    // Check if all the data was received
    if (image_size == total_bytes_rec)
    {
        std::cout << "Image data received successfully." << std::endl;

        // Load the image data into a QPixmap
        if (!profile_picture.loadFromData(reinterpret_cast<const unsigned char*>(buffer.data()), buffer.size()))
        {
            std::cerr << "Failed to load image data into QPixmap." << std::endl;
        }
    }
    else
    {
        std::cerr << "Mismatch between expected and received image size." << std::endl;
    }

    return profile_picture;
}








