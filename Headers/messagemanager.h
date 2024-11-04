#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include "Headers/networkmanager.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unordered_map>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <vector>
#include <ctime>

struct ChatMessage
{
    std::time_t timestamp;
    std::string sender_username;
    int sender_id;
    int conversation_id;
    std::string message_content;
    std::vector<char> image_arr;
    int message_id;

};

class MainWindow;

class messagemanager
{
public:
    messagemanager(const int &user_id);
    int get_message_manager_socket();
    void async_receive_messages(const int &message_manager_socket, MainWindow* mainWindow);
    void send_message(int client_socket, const QByteArray &data, const QString &type);
    std::vector<std::vector<std::string>> pull_init_chat_messages(int client_socket, const std::vector<int>& participants);
    std::vector<ChatMessage> get_messages_from_memory(const int &convo_id);
    void set_user_id(int user_id);
    int get_user_id();
    void append_to_mem_struct(int convo_id, ChatMessage mess);

private:
    int message_manager_socket;
    networkmanager* message_network_manager;
    std::unordered_multimap<int, ChatMessage> message_memory_structure;
    int message_manager_user_id;
    std::atomic<bool> is_receiving{true};
    std::vector<ChatMessage> pull_all_chat_messages(int client_socket);
};

#endif // MESSAGEMANAGER_H
