#include "Headers/messagemanager.h"
#include "Headers/networkmanager.h"
#include "Headers/mainwindow.h"


messagemanager::messagemanager(const int &user_id) {

    message_network_manager = new networkmanager();

    //pass user's user id into this
    message_manager_socket = message_network_manager->setup_socket("MESSAGE_MANAGEMENT", user_id);
}

int messagemanager::get_message_manager_socket()
{
    return message_manager_socket;
}

std::string extract_between(const std::string& data, const std::string& start_delim, const std::string& end_delim) {
    size_t start = data.find(start_delim);
    if (start == std::string::npos) return "";
    start += start_delim.length();  // Move past the start delimiter

    size_t end = data.find(end_delim, start);
    if (end == std::string::npos) return "";

    return data.substr(start, end - start);
}

void messagemanager::async_receive_messages(const int &message_manager_socket, MainWindow* mainWindow)
{
    //thread to recieve
    char buffer[1024];
    std::string accumulated_data;
    while (true) {

        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(message_manager_socket, buffer, sizeof(buffer), 0);
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
            QMetaObject::invokeMethod(mainWindow, "appendMessageToTextBrowser", Qt::QueuedConnection, Q_ARG(QString, qMessage));
        }
    }
}

void messagemanager::send_message(int client_socket, const QByteArray &data, const QString &type)
{
    //types are image, text, verify_user, new_user
    QByteArray header = type.toUtf8();
    header.append("\n");
    QByteArray buffer = header + data;

    const char *raw_data = buffer.constData();
    size_t buffer_size = buffer.size();

    qDebug() << "Sending message of type: " << type;

    if (send(client_socket, raw_data, buffer_size, 0) == -1)
    {
        qDebug() << "Error sending image";
    }

}

std::vector<std::vector<std::string>> messagemanager::pull_init_chat_messages(int client_socket, const std::vector<int>& participants)
{
    int member_count = participants.size();
    int user_id_one = participants[0];
    std::string user_id_pipe = std::to_string(user_id_one) + "|";

    std::vector<std::vector<std::string>> empty_chat_log;
    std::vector<std::vector<std::string>> chat_log;

    const char* type = "init_chat\n";
    ssize_t bytes_sent = send(client_socket, type, strlen(type), 0);

    std::cout << "Bytes sent for chat retrieval: " << bytes_sent << std::endl;

    if(bytes_sent > -1)
    {

    std::cout << "sending chat member id's" << std::endl;
    ssize_t sec_bytes_sent = send(client_socket, user_id_pipe.c_str(), user_id_pipe.size(), 0);
    if (sec_bytes_sent > -1)
    {
        while (true)
        {
            char buffer[500] = {0};
            std::string all_message_data;

            while (true)
            {
                std::cout << "retrieving chat messages" << std::endl;
                ssize_t status_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

                if (status_bytes < 0)
                {
                    std::cerr << "Error receiving data from server" << std::endl;
                    return empty_chat_log;
                }
                else if (status_bytes == 0)
                {
                    std::cerr << "Chat log init connection closed by server" << std::endl;
                    return empty_chat_log;
                }

                buffer[status_bytes] = '\0';
                all_message_data += buffer;
                std::cout << "Message data: " << all_message_data << std::endl;

                if (all_message_data.find("\\|") != std::string::npos || all_message_data.find("-") != std::string::npos)
                {
                    break;
                }
            }
            if(all_message_data == "-\0")
            {
                std::cout << "Termination signal received. Stopping reception of messages" << std::endl;
                break;
            }

            if(all_message_data.size() > 1)
            {
                std::string time_stamp = all_message_data.substr(0, all_message_data.find("\\+"));
                std::string sender_username = extract_between(all_message_data, "\\+", "\\-");
                std::string message_contents = extract_between(all_message_data, "\\-", "\\|");

                std::vector<std::string> temp_vct = {time_stamp, sender_username, message_contents};
                chat_log.push_back(temp_vct);
            }
        }

        return chat_log;
    }
    }
    else{qDebug() << "Error getting chat messages";}
    return empty_chat_log;
}





