#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>
#include <QString>
#include <mutex>
struct Message
{
    std::string sender;
    std::string message;
    std::string date;
};

class MainWindow;

class networkmanager
{

    public:
        //constructor
        networkmanager();
        Message set_message(Message* message, const std::string &mess, const std::string &name);
        void receive_messages(int client_socket);
        //int set_connect_socket();
        void send_message(int client_socket, const QByteArray &data, const QString &type);
        int get_client_socket();
        void receive_messages(int client_socket, MainWindow* mainWindow);
        void close_connection(int client_socket);
        int setup_socket(const std::string &handshake_msg, const int &user_id);


    private:
        int client_socket;
    };

#endif // NETWORKMANAGER_H
