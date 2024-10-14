#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include "Headers/networkmanager.h"
#include <iostream>
#include <string>
#include <sys/socket.h>

class MainWindow;

class messagemanager
{
public:
    messagemanager(const int &user_id);
    int get_message_manager_socket();
    void async_receive_messages(const int &message_manager_socket, MainWindow* mainWindow);
    void send_message(int client_socket, const QByteArray &data, const QString &type);
    std::vector<std::vector<std::string>> pull_init_chat_messages(int client_socket, int* participants);

private:
    int message_manager_socket;
    networkmanager* message_network_manager;
};

#endif // MESSAGEMANAGER_H
