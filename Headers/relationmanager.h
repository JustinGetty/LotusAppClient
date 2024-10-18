#include <Headers/networkmanager.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <vector>
#include <utility>

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
    std::vector<std::pair<std::string, int>> pull_inbound_friend_requests(int client_socket);
    int fetch_user_id_from_server(const std::string &username);
    int send_friend_update(const int &relation_socket, const int &sender_id, const std::string &type);
    std::vector<std::pair<std::string, int>> pull_outbound_friend_requests(int client_socket);
    std::vector<std::pair<std::string, int>> pull_friends_list(int client_socket);
    void set_friends_list_mem(std::vector<std::pair<std::string, int>> friends_list);
    std::vector<std::pair<std::string, int>> get_friends_list_mem();
    int update_friends_list_mem();

private:
    int relation_manager_socket;
    networkmanager* relation_network_manager;
    std::vector<std::pair<std::string, int>> friends_list_in_memory;
};

#endif // RELATIONMANAGER_H
