#include <Headers/networkmanager.h>
#include <iostream>
#include <string>
#include <sys/socket.h>

#ifndef RELATIONMANAGER_H
#define RELATIONMANAGER_H

class relationmanager
{
public:
    relationmanager(int user_id);
    int get_relation_manager_socket();
    void async_manage_requests(const int &relation_manager_socket, MainWindow* mainwindow);
    void send_friend_request(int socket, const std::string &data_to_send);
    std::string get_request_status(int client_socket);

private:
    int relation_manager_socket;
    networkmanager* relation_network_manager;
};

#endif // RELATIONMANAGER_H
