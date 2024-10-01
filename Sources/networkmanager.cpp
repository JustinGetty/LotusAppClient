#include "Headers/networkmanager.h"
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <ctime>
#include <vector>
#include "Headers/mainwindow.h"
#include <QString>

/*


 */
#define SERVER_PORT 4000
#define SERVER_IP "192.168.1.192"

std::mutex clients_mutex;
std::vector<int> client_sockets;


networkmanager::networkmanager() {

}


//refactor to take in a connection to close as argument
void networkmanager::close_connection(int client_socket){
    if (client_socket != -1){
        close(client_socket);
        client_socket = -1;
        qDebug() << "Network connection closed.";
    }
}

int networkmanager::setup_socket(const std::string &handshake_msg, const int &user_id)
{

    //create sockets and shit here, this is constructor

    int temp_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (temp_socket == -1) {
        std::cerr << "Error creating the socket ..." << std::endl;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(temp_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Connection failed" << std::endl;

    }

    //implement send here to send provided handshake, send user_id so sockets can be managed by user.
    // When message is sent, can check if user is online,if yes, store and send directly to them. else, store

    std::string combined_handshake_userid = handshake_msg + "+" + std::to_string(user_id) + "|";

    if (send(temp_socket, combined_handshake_userid.c_str(), combined_handshake_userid.size(), 0) == -1)
    {
        qDebug() << "Error sending image";
    }

    return temp_socket;

}









