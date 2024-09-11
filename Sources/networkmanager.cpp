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
std::mutex clients_mutex;
std::vector<int> client_sockets;


networkmanager::networkmanager() {

    //create sockets and shit here, this is constructor

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket == -1) {
        std::cerr << "Error creating the socket ..." << std::endl;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(4000);
    server_address.sin_addr.s_addr = inet_addr("129.213.59.230");

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Connection failed" << std::endl;

    }
}


//functions here

//get method for client_socket
int networkmanager::get_client_socket(){
    //on other side, get rthis with networkmanabger->get_client...; if -1, fucked
    return client_socket;
}

//rework or delete
Message networkmanager::set_message(Message* message, const std::string& mess, const std::string& name) {
    std::time_t raw_time = std::time(nullptr);
    std::string date = std::ctime(&raw_time);

    if (!date.empty() && date.back() == '\n') {
        date.pop_back();
    }

    std::string day_date = date.substr(4, 6);
    if (day_date[4] == ' ') {
        day_date[4] = '0';
    }
    std::string day_time = "[" + day_date + " " + date.substr(11, 5) + "]";

    message->date = day_time;
    message->message = mess;
    message->sender = name;

    return *message;
}


//goes into a thread, rec_thread.. blah blah
void networkmanager::receive_messages(int client_socket, MainWindow* mainWindow) {
    char buffer[1024];
    std::string accumulated_data;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "Error receiving data or connection closed by server" << std::endl;
            break;
        }

        accumulated_data.append(buffer, bytes_received);

        size_t pos;
        while ((pos = accumulated_data.find('\n')) != std::string::npos) {
            std::string message = accumulated_data.substr(0, pos + 1);
            accumulated_data.erase(0, pos + 1);

            if (!message.empty() && message.back() == '\n') {
                message.pop_back();
            }

            // Use Qt's signals/slots mechanism in the MainWindow class to update the UI
            QString qMessage = QString::fromStdString(message);
            mainWindow->appendMessageToTextBrowser(qMessage);
        }
    }
}


void networkmanager::send_message(int client_socket, const QByteArray &data, const QString &type)
{
    //types are image, text, verify_user, new_user
    QByteArray header = type.toUtf8();
    header.append("\n");
    QByteArray buffer = header + data;

    const char *raw_data = buffer.constData();
    size_t buffer_size = buffer.size();

    if (send(client_socket, raw_data, buffer_size, 0) == -1)
    {
        qDebug() << "Error sending image";
    }

}

void networkmanager::close_connection(){
    if (client_socket != -1){
        close(client_socket);
        client_socket = -1;
        qDebug() << "Network connection closed.";
    }
}









