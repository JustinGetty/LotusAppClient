#ifndef LOGICMANAGER_H
#define LOGICMANAGER_H
#include "Headers/networkmanager.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
class logicmanager
{
public:
    logicmanager();
    void async_handle_auth_and_infrastructure(const int &logic_manager_socket);
    int get_logic_manager_socket();
    void verify_login(int client_socket, const std::string &verify_combined);
    std::string get_status(int client_socket);
    void verify_new_account(int conn_socket, const std::string &user_pw);

private:
    int logic_manager_socket;
    networkmanager* logic_network_manager;
};

#endif // LOGICMANAGER_H
