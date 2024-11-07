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
std::vector<ChatMessage> messagemanager::pull_all_chat_messages(int client_socket)
{
    is_receiving = false;
    std::cout << "pull_all_chat_messages" << std::endl;
    std::vector<ChatMessage> empty_chat_log;
    std::vector<ChatMessage> chat_log;

    const char* type = "get_all_chats\n";
    ssize_t bytes_sent = send(client_socket, type, strlen(type), 0);
    std::cout << "Bytes sent for chat retrieval: " << bytes_sent << std::endl;

    if (bytes_sent <= -1)
    {
        std::cerr << "Error sending chat retrieval request." << std::endl;
        is_receiving = true;
        return empty_chat_log;
    }

    std::string buffer_data;

    while (true)
    {
        char buffer[20] = {0};
        ssize_t status_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (status_bytes < 0)
        {
            std::cerr << "Error receiving data from server." << std::endl;
            is_receiving = true;
            return empty_chat_log;
        }
        else if (status_bytes == 0)
        {
            std::cerr << "Chat log connection closed by server." << std::endl;
            is_receiving = true;
            return empty_chat_log;
        }
        else if (status_bytes == 1)
        {
            std::cout << "Termination signal received. Ending reception." << std::endl;
            is_receiving = true;
            return chat_log;

        }

        buffer[status_bytes] = '\0';
        buffer_data += buffer;
        if (buffer == "\\#" || buffer == "\#" || buffer_data[0] == '#')
        {
            std::cout << "Termination signal received. Ending reception." << std::endl;
            is_receiving = true;
            return chat_log;
        }

        size_t pos;
        while ((pos = buffer_data.find("\\|")) != std::string::npos)
        {
            std::string message = buffer_data.substr(0, pos); // Extract message data
            buffer_data.erase(0, pos + 2); // Remove message data and delimiter

            std::cout << "Processing message: " << message << std::endl;
            try
            {

                message += "\\|";
                std::string unix_timestamp_str = message.substr(0, message.find("\\+"));
                std::string sender_username = extract_between(message, "\\+", "\\-");
                std::string sender_id = extract_between(message, "\\-", "\\]");
                std::string conversation_id = extract_between(message, "\\]", "\\[");
                std::string message_contents = extract_between(message, "\\[", "\\$");
                std::string message_id = extract_between(message, "\\$", "\\~");
                std::string image_size_str = extract_between(message, "\\~", "\\|");

                // Debugging output to check extracted fields
                std::cout << "Extracted fields:" << std::endl;
                std::cout << "Timestamp: " << unix_timestamp_str << std::endl;
                std::cout << "Username: " << sender_username << std::endl;
                std::cout << "Sender ID: " << sender_id << std::endl;
                std::cout << "Conversation ID: " << conversation_id << std::endl;
                std::cout << "Message contents: " << message_contents << std::endl;
                std::cout << "Message ID: " << message_id << std::endl;
                std::cout << "Image size: " << image_size_str << std::endl;

                // Check if fields are empty before conversion
                if (unix_timestamp_str.empty() || sender_id.empty() || conversation_id.empty() || message_id.empty() || image_size_str.empty())
                {
                    throw std::invalid_argument("One or more fields are empty.");
                }

                // Convert string fields to appropriate types
                int image_size = std::stoi(image_size_str);
                ChatMessage chat_message;

                chat_message.timestamp = static_cast<std::time_t>(std::stoll(unix_timestamp_str));
                chat_message.sender_username = sender_username;
                chat_message.sender_id = std::stoi(sender_id);
                chat_message.conversation_id = std::stoi(conversation_id);
                chat_message.message_content = message_contents;
                chat_message.message_id = std::stoi(message_id);

                if (image_size > 0)
                {
                    std::cout << "Receiving image" << std::endl;
                    std::vector<char> image_data(image_size);
                    size_t total_received = 0;
                    while (total_received < image_size)
                    {
                        ssize_t bytes_received = recv(client_socket, &image_data[total_received], image_size - total_received, 0);
                        if (bytes_received <= 0)
                        {
                            std::cerr << "ERROR: Failed to receive image data." << std::endl;
                            is_receiving = true;
                            return empty_chat_log;
                        }
                        total_received += bytes_received;
                    }
                    chat_message.image_arr = std::move(image_data); // issue in debugger
                }

                chat_log.push_back(chat_message); // Add completed message to the log
                std::cout << "Added message with ID " << chat_message.message_id << " to chat log." << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error parsing message: " << e.what() << std::endl;
                continue;
            }

            // Check for termination condition
            if (message == "\\#" || message == "\#")
            {
                std::cout << "Termination signal received. Ending reception." << std::endl;
                is_receiving = true;
                return chat_log;
            }
        }
    }
    is_receiving = true;
    return chat_log;
}



messagemanager::messagemanager(const int &user_id) {

    message_network_manager = new networkmanager();

    //pass user's user id into this
    message_manager_socket = message_network_manager->setup_socket("MESSAGE_MANAGEMENT", user_id);

    //here load all messages to data structure

    std::vector<ChatMessage> chats_temp = pull_all_chat_messages(message_manager_socket);

    for(auto row : chats_temp)
    {
        int conversation_id = row.conversation_id;
        message_memory_structure.insert({conversation_id, row});
        std::cout << "Message Content: " << row.message_content << std::endl;
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

    std::cout << "async thread up" << std::endl;
    std::string buffer_data;

    while (true) {

        std::cout << "ASYNC THREAD CALLED" << std::endl;

        while (!is_receiving) {
            std::cout << "Sleepign async thread" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        // Receiving data in chunks
        char buffer[999] = {0};
        ssize_t status_bytes = recv(message_manager_socket, buffer, sizeof(buffer) - 1, 0);

        std::cout << "Message Received" << std::endl;

        if (status_bytes < 0) {
            std::cerr << "Error receiving data from server." << std::endl;
            continue;
        } else if (status_bytes == 0) {
            std::cerr << "Chat log connection closed by server." << std::endl;
            break;
        }

        // Append received data to buffer_data
        buffer[status_bytes] = '\0';
        buffer_data += buffer;
        std::cout << "Received data chunk: " << buffer << std::endl;

        // Process complete messages delimited by "\\|"
        size_t pos;
        while ((pos = buffer_data.find("\\|")) != std::string::npos) {
            // Extract and remove the message from buffer_data
            std::string message = buffer_data.substr(0, pos + 3);

            if (message[0] == '#')
            {
                message.erase(0, 1);
            }

            // Parse message components
            std::string unix_timestamp_str = message.substr(0, message.find("\\+"));
            std::string sender_username = extract_between(message, "\\+", "\\-");
            std::string sender_id_str = extract_between(message, "\\-", "\\]");
            std::string conversation_id_str = extract_between(message, "\\]", "\\[");
            std::string message_content = extract_between(message, "\\[", "\\$");
            std::string image_size_str = extract_between(message, "\\$", "\\&");
            std::string message_id_str = extract_between(message, "\\&", "\\|");

            // Convert parsed strings to appropriate types
            ChatMessage chat_message;
            chat_message.timestamp = static_cast<std::time_t>(std::stoll(unix_timestamp_str));
            chat_message.sender_username = sender_username;
            chat_message.sender_id = std::stoi(sender_id_str);
            chat_message.conversation_id = std::stoi(conversation_id_str);
            chat_message.message_content = message_content;
            chat_message.message_id = std::stoi(message_id_str);

            int image_size = std::stoi(image_size_str);
            if (image_size > 0) {
                // Receive image data if image_size > 0
                std::vector<char> image_data(image_size);
                size_t total_received = 0;
                while (total_received < image_size) {
                    ssize_t bytes_received = recv(message_manager_socket, &image_data[total_received], image_size - total_received, 0);
                    if (bytes_received <= 0) {
                        std::cerr << "ERROR: Failed to receive image data" << std::endl;
                        break;
                    }
                    total_received += bytes_received;
                }
                chat_message.image_arr = std::move(image_data);  // Transfer image data to chat_message
            }

            append_to_mem_struct(chat_message.conversation_id, chat_message);
            // Display or notify user based on sender and chat status
            int sender_id_global = std::stoi(sender_id_str);  // Assuming sender_id_global is required here

            QMetaObject::invokeMethod(mainWindow, "appendMessageToChat", Qt::QueuedConnection, Q_ARG(ChatMessage, chat_message));

            if (mainWindow->get_push_button_embed_id() == sender_id_global) {
                // Use Qt's signal-slot mechanism to update UI in main thread

                //QMetaObject::invokeMethod(mainWindow, "addMessageToChat", Qt::QueuedConnection, Q_ARG(ChatMessage, chat_message));
            } else {
                // If not in chat, show notification and add to memory logs (custom logic here)
                //mainWindow->showNotification(sender_username, message_content);
            }
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

std::vector<ChatMessage> messagemanager::get_messages_from_memory(const int &convo_id)
{
    std::vector<ChatMessage> chat_specific_logs;

    for (const auto& row : message_memory_structure)
    {
        const ChatMessage& message_data = row.second;  // row.second is the vector of strings (the message data)

        //reconfig this to only get th chats with those in the user_list
        if (row.first == convo_id)
        {
            chat_specific_logs.push_back(row.second);
        }
    }

    return chat_specific_logs;
}

void messagemanager::append_to_mem_struct(int convo_id, ChatMessage mess)
{
    message_memory_structure.insert({convo_id, mess});
}





