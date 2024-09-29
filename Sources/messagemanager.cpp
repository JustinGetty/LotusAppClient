#include "Headers/messagemanager.h"
#include "Headers/networkmanager.h"
#include "Headers/mainwindow.h"


messagemanager::messagemanager(const int &user_id) {

    message_network_manager = new networkmanager();

    //pass user's user id into this
    message_manager_socket = message_network_manager->setup_socket("MESSAGE_MANAGEMENT", user_id);
}

int messagemanager::get_message_manager_socket()
{
    return message_manager_socket;
}

void messagemanager::async_receive_messages(const int &message_manager_socket, MainWindow* mainWindow)
{
    //thread to recieve
    char buffer[1024];
    std::string accumulated_data;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(message_manager_socket, buffer, sizeof(buffer), 0);
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
