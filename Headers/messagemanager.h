#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include "Headers/networkmanager.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unordered_map>

class MainWindow;

class messagemanager
{
public:
    messagemanager(const int &user_id);
    int get_message_manager_socket();
    void async_receive_messages(const int &message_manager_socket, MainWindow* mainWindow);
    void send_message(int client_socket, const QByteArray &data, const QString &type);
    std::vector<std::vector<std::string>> pull_init_chat_messages(int client_socket, const std::vector<int>& participants);
    std::vector<std::vector<std::string>> get_messages_from_memory(const std::vector<int> &user_list);
    void set_user_id(int user_id);
    int get_user_id();

private:
    int message_manager_socket;
    networkmanager* message_network_manager;
    std::unordered_multimap<int, std::vector<std::string>> message_memory_structure;
    int message_manager_user_id;
};

#endif // MESSAGEMANAGER_H
