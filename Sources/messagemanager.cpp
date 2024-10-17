#include "Headers/messagemanager.h"
#include "Headers/networkmanager.h"
#include "Headers/mainwindow.h"

std::string extract_between(const std::string& data, const std::string& start_delim, const std::string& end_delim) {
    size_t start = data.find(start_delim);
    if (start == std::string::npos) return "";
    start += start_delim.length();  // Move past the start delimiter

    size_t end = data.find(end_delim, start);
    if (end == std::string::npos) return "";

    return data.substr(start, end - start);
}

std::vector<std::vector<std::string>> pull_all_chat_messages(int client_socket)
{
    std::vector<std::vector<std::string>> empty_chat_log;
    std::vector<std::vector<std::string>> chat_log;

    const char* type = "get_all_chats\n";
    ssize_t bytes_sent = send(client_socket, type, strlen(type), 0);

    std::cout << "Bytes sent for chat retrieval: " << bytes_sent << std::endl;

    if (bytes_sent > -1)
    {
        std::string buffer_data;
        while (true)
        {
            char buffer[500] = {0};
            ssize_t status_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

            if (status_bytes < 0)
            {
                std::cerr << "Error receiving data from server." << std::endl;
                return empty_chat_log;
            }
            else if (status_bytes == 0)
            {
                std::cerr << "Chat log connection closed by server." << std::endl;
                return empty_chat_log;
            }

            buffer[status_bytes] = '\0';
            buffer_data += buffer;
            std::cout << "Received data chunk: " << buffer << std::endl;

            // Process complete messages in the buffer
            size_t pos;
            while ((pos = buffer_data.find("\\|")) != std::string::npos)
            {
                std::string message = buffer_data.substr(0, pos + 2); // Extract message including "\\|"
                buffer_data.erase(0, pos + 2); // Remove processed message from buffer

                // Check if it's the termination signal
                if (message == "-\\|" || "-")
                {
                    std::cout << "Termination signal received. Stopping reception of messages." << std::endl;
                    return chat_log;
                }

                // Parse the message
                std::string time_stamp = message.substr(0, message.find("\\+"));
                std::string sender_username = extract_between(message, "\\+", "\\-");
                std::string sender_id = extract_between(message, "\\-", "\\]");
                std::string receiver_1 = extract_between(message, "\\]", "\\[");
                std::string message_contents = extract_between(message, "\\[", "\\|");

                std::vector<std::string> temp_vct = {time_stamp, sender_username, sender_id, receiver_1, message_contents};
                chat_log.push_back(temp_vct);

                // Output parsed message details
                std::cout << "Parsed message - Timestamp: " << time_stamp
                          << ", Sender: " << sender_username
                          << ", Sender ID: " << sender_id
                          << ", Receiver ID: " << receiver_1
                          << ", Message: " << message_contents << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "Error sending chat retrieval request." << std::endl;
    }
    return empty_chat_log;
}


messagemanager::messagemanager(const int &user_id) {

    message_network_manager = new networkmanager();

    //pass user's user id into this
    message_manager_socket = message_network_manager->setup_socket("MESSAGE_MANAGEMENT", user_id);

    //here load all messages to data structure

    std::vector<std::vector<std::string>> chats_temp = pull_all_chat_messages(message_manager_socket);

    for(auto row : chats_temp)
    {
        std::cout << "Col 2: " << row[2] << " Col3: " << row[3] << std::endl;
        if(!row.empty())
        {
            int sender_id = std::stoi(row[2]);
            int receiver_id = std::stoi(row[3]);

            if(sender_id == user_id)
            {
                message_memory_structure.insert({receiver_id, row});
            }
            else
            {
                message_memory_structure.insert({sender_id, row});
            }
            std::cout << "Message Content: " << row[4] << std::endl;
        }
    }

}
void messagemanager::set_user_id(int user_id)
{
    message_manager_user_id = user_id;
}

int messagemanager::get_user_id()
{
    return message_manager_user_id;
}

int messagemanager::get_message_manager_socket()
{
    return message_manager_socket;
}


void messagemanager::async_receive_messages(const int &message_manager_socket, MainWindow* mainWindow)
{
    //if user currently in chat, immediately append to that chat
    //if not in chat, show notifcation and add to memory logs
    //thread to recieve
    // Thread to continuously receive messages from the server
    char buffer[1024];
    std::string accumulated_data;

    while (true) {
        int sender_id_global;
        std::string time_stamp;
        std::string sender_username;
        std::string message_contents;
        // Clear the buffer and receive incoming messages
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = recv(message_manager_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0) {
            std::cerr << "Error receiving data or connection closed by server" << std::endl;
            break;  // Exit the loop if there's an error or the connection is closed
        }

        buffer[bytes_received] = '\0';  // Null-terminate the buffer
        accumulated_data += buffer;     // Accumulate data from the buffer

        // Process complete messages, which end with "\\|"
        size_t pos;
        while ((pos = accumulated_data.find("\\|")) != std::string::npos) {
            // Extract the entire message up to the "\\|" delimiter
            std::string all_message_data = accumulated_data.substr(0, pos + 2);  // Include the "\\|"
            accumulated_data.erase(0, pos + 2);  // Remove the processed message from accumulated_data

            if (all_message_data.size() > 1) {
                // Parse the incoming message in the format: timestamp\\+username\\-user_id\\]receiver_id\\[message_contents\\|
                time_stamp = all_message_data.substr(0, all_message_data.find("\\+"));
                sender_username = extract_between(all_message_data, "\\+", "\\-");
                std::string sender_id = extract_between(all_message_data, "\\-", "\\]");
                std::string receiver_id = extract_between(all_message_data, "\\]", "\\[");
                message_contents = extract_between(all_message_data, "\\[", "\\|");
                sender_id_global = std::stoi(sender_id);
                // Create a vector to represent the parsed message data
                std::vector<std::string> temp_vct = {time_stamp, sender_username, sender_id, receiver_id, message_contents};

                // Insert the message into the message_memory_structure
                if (std::stoi(sender_id) == message_manager_user_id) {
                    message_memory_structure.insert({std::stoi(receiver_id), temp_vct});
                } else {
                    message_memory_structure.insert({std::stoi(sender_id), temp_vct});
                }

                // Print the extracted message for debugging
                std::cout << "Message Received - Time: " << time_stamp << ", Sender: " << sender_username
                          << ", Receiver: " << receiver_id << ", Message: " << message_contents << std::endl;

            }
        }
        if(mainWindow->get_push_button_embed_id() == sender_id_global)
        {
            // Use Qt's signals/slots mechanism in the MainWindow class to update the UI
            std::string message = time_stamp + " " + sender_username + " " + message_contents;
            QString qMessage = QString::fromStdString(message);
            QMetaObject::invokeMethod(mainWindow, "addMessageToTextBrowser", Qt::QueuedConnection, Q_ARG(QString, qMessage));

        }
        else
        {
            //somehow show notification, maybe move that chat to the top and highlight
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

std::vector<std::vector<std::string>> messagemanager::get_messages_from_memory(const int &non_client_user_id)
{
    std::vector<std::vector<std::string>> chat_specific_logs;

    for (const auto& row : message_memory_structure)
    {
        const std::vector<std::string>& message_data = row.second;  // row.second is the vector of strings (the message data)

        if (std::stoi(message_data[2]) == non_client_user_id || std::stoi(message_data[3]) == non_client_user_id)
        {
            std::vector<std::string> temp_vct = {message_data[0], message_data[1], message_data[4]};
            chat_specific_logs.push_back(temp_vct);
        }
    }

    return chat_specific_logs;
}





