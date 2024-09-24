#include "Headers/mainwindow.h"
#include "ui_mainwindow.h"
#include "Headers/networkmanager.h"
#include "Headers/user.h"
#include <QLabel>
#include <QString>
#include <QKeyEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QBuffer>
#include <QTextBrowser>
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <ctime>
#include <QTimer>


/*
 TODO: add sql to server siddeeee
*/

QByteArray std_string_to_qbytearray(std::string x){

    QByteArray y = QByteArray::fromRawData(x.data(), static_cast<int>(x.size()));

    return y;
}
std::string get_status(int client_socket) {
    char buffer[30];
    std::string status_message;

    while (true) {
        ssize_t status_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (status_bytes < 0) {

            return "";
        } else if (status_bytes == 0) {
            std::cerr << "Connection closed by the server." << std::endl;
            return "";
        }
        buffer[status_bytes] = '\0';

        status_message += buffer;

        if (status_message.find('|') != std::string::npos) { //npos represents end of string
            break;
        }
    }

    std::cerr << "Finished receiving status message: " << status_message << std::endl;

    // Return the message up to the delimiter
    return status_message.substr(0, status_message.find('|'));
}

void MainWindow::onAppendMessageToTextBrowser(const QString& message) {
    ui->textBrowser->append(message);
}
//upload images


void MainWindow::on_uploadButton_clicked()
{
    //tr alloweds multi language support
    //close this when done somehow
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp *.gif *.jpeg)"));

    if (!fileName.isEmpty())
    {
        QImage image;
        if (!image.load(fileName))
        {
            QMessageBox::warning(this, tr("Error"), tr("Failed to load image :("));
            return;
        }

        QString imageHTML = QString("<img src=\"%1\" width=\"400\"/>").arg(fileName);

        //ui->textBrowser->insertHtml(imageHTML);

        //RESIZE INPUT LABEL HEREEEEE!!!!!!!!
        // int height = image.height();
        //ui->Message_Input_Label->setFixedHeight(height);
        ui->Message_Input_Label->insertHtml(imageHTML);

        //time to send that shi
        QByteArray byte_array;
        QBuffer buffer(&byte_array);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");  // format is png, change that later to be dynamic
//super important here, this needs to go into send_message_button_clicked9)        networkManager->send_message(3, byte_array, "image");

        //QMessageBox::information(this, tr("Success"), tr("Image uploaded and displayed successfully."));
    }
}


void MainWindow::on_Send_Message_Button_clicked()
{
    //fuck this fag ass plain text type REFIG asap
    user_message = ui->Message_Input_Label->toPlainText();
    ui->textBrowser->append(user_message);
    ui->Message_Input_Label->clear();

    QByteArray Ronaldo = user_message.toUtf8();

    //needs std string not  ass QString
    //std::string messi = user_message.toStdString();

    //3 is client socket, too lazy to put in scope
    // types are text or image, that's it. Dont mess that up

}


bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{

    if (watched == ui->Message_Input_Label && event->type() == QEvent::FocusIn)
    {
        ui->Message_Input_Label->clear();
        //qDebug() << "FocusIn event triggered on Message_Input_Label";
        return true;
    }
    else if (watched == ui->Message_Input_Label && event->type() == QEvent::FocusOut)
    {
        ui->Message_Input_Label->setText("Type your message here...");
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::on_create_account_button_clicked()
{
    //prevent the double messages
    if (isProcessing) {
        return; // Prevent re-execution
    }
    isProcessing = true;

    if ((!ui->create_user_input->text().isEmpty()) && (!ui->create_pwd_input->text().isEmpty()))
    {
        std::string username = (ui->create_user_input->text()).toStdString();
        std::string password = (ui->create_pwd_input->text()).toStdString();

        std::string user_pw = username + "|" + password;
        QByteArray user_pw_q = QByteArray::fromRawData(user_pw.data(), static_cast<int>(user_pw.size()));
        //eventually this needs to be actual client socket and not hard coded
        networkManager->send_message(networkManager->get_client_socket(), user_pw_q, "new_user");

        std::cout << "Verifying creation" << std::endl;
        std::string account_create_status = get_status(networkManager->get_client_socket());
        std::cout << "Account Creation Status" << account_create_status << std::endl;

        QTimer::singleShot(1000, this, [this]() { isProcessing = false; });
    }
    else
    {
        qDebug() << "Error, please enter a valid username and password";
    }

}

void MainWindow::on_login_account_button_clicked()
{
    std::cout << "here" << std::endl;
    if (isLoginProcessing) {
        return;  // Prevent double click handling
    }

    isLoginProcessing = true;

    if ((!ui->user_login_name->text().isEmpty()) && (!ui->user_login_password->text().isEmpty()))
    {
        std::string username = (ui->user_login_name->text()).toStdString();
        std::string password = (ui->user_login_password->text()).toStdString();

        std::string user_pw = username + "|" + password;
        QByteArray user_pw_q = QByteArray::fromRawData(user_pw.data(), static_cast<int>(user_pw.size()));
        //eventually this needs to be actual client socket and not hard coded
        networkManager->send_message(networkManager->get_client_socket(), user_pw_q, "verify_user");

        //get status of login
        std::cout << "Verifying..." << std::endl;

        std::string status_msg = get_status(networkManager->get_client_socket());
        std::cout << "Verification status: " + status_msg << std::endl;


        //need to pull in user id here too, split on pipe. server work later, simple fix
        if (status_msg == "verification_succeeded")
        {
            int set_user_id_status = active_user->set_user_id(69);
            int set_user_status = active_user->set_active_user_username(username);
            //more robust error handling here
            if (set_user_status == 0) {
                set_mainview_objects_tot();
                switch_to_main_view_after_login();
            }
        }
        else if (status_msg == "verification_failed")
        {
            ui->login_error_message->setText("<font color='red'>Error: Username or Password is incorrect. Try again.");


        QTimer::singleShot(1000, this, [this]() { isLoginProcessing = false; });

        }
    }
    else
    {
        qDebug() << "Error, please enter a valid username and password";
        ui->login_error_message->setText("<font color='red'>Error: login failed, enter a username and password.");
    }
}
//all main window setup should be handled here
void MainWindow::set_mainview_objects_tot()
{
    QString username_text = "User signed in: " + QString::fromStdString(active_user->get_active_user_username());
    ui->active_user_label->setText(username_text);


}

void MainWindow::switch_to_create_account_view()
{
    ui->login_window->hide();
    ui->create_account->show();
}

void MainWindow::switch_to_login_account_view()
{
    ui->create_account->hide();
    ui->login_window->show();
}

void MainWindow::switch_to_main_view_after_login()
{
    ui->login_window->hide();
    ui->main_window->show();
}

void MainWindow::switch_to_friends_view()
{
    ui->main_window->hide();
    ui->friends_window->show();
}

void MainWindow::send_friend_request()
{
    if (!ui->username_lookup_line_edit->text().isEmpty())
    {

        std::string receiver_username = (ui->username_lookup_line_edit->text()).toStdString();
        receiver_username = receiver_username + "+";
        std::string data = receiver_username + std::to_string((active_user->get_user_id())) + "|";

        QByteArray username_to_send = std_string_to_qbytearray(data);

        networkManager->send_message(networkManager->get_client_socket(), username_to_send, "new_friend_request");

        std::string result = get_status(networkManager->get_client_socket());
        std::cout << "Result: " << result << std::endl;

        QString error_msg_to_display = QString::fromStdString("<font color='red'>" + result);

        // Calling setText on QLabel safely in the main thread
        QMetaObject::invokeMethod(ui->friend_request_error_message,
                                  "setText",               // Method to call (as a string)
                                  Qt::QueuedConnection,    // Ensure it's queued to run in the main thread


    }

    else {
        //display error
    }


}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // **SETUP** ---------------------------------------------------------------------------------------------------

    ui->setupUi(this);

    if (!ui->create_account) std::cerr << "create_account is null!" << std::endl;
    if (!ui->login_window) std::cerr << "login_window is null!" << std::endl;
    if (!ui->main_window) std::cerr << "main_window is null!" << std::endl;

    ui->friends_window->hide();
    ui->main_window->hide();
    ui->create_account->hide();
    ui->login_window->show();

    networkManager = new networkmanager();
    active_user = new user();
    int client_socket = networkManager->get_client_socket();

    QIcon icon("/Users/justin/the_harbor/Chat/ChatWidget/ChatApplication/Resources/send_message_icon.jpeg");
    ui->Send_Message_Button->setIcon(icon);
    ui->Send_Message_Button->setIconSize(QSize(ui->Send_Message_Button->width() + 4, ui->Send_Message_Button->height()));

    std::thread receive_thread([this, client_socket]() {
        networkManager->receive_messages(client_socket, this);
    });
    receive_thread.detach();


    //eventually connect and render these as neede, i.e in the function that loads them
    connect(ui->Send_Message_Button, &QPushButton::clicked, this, &MainWindow::on_Send_Message_Button_clicked);
    connect(this, &MainWindow::appendMessageToTextBrowser, this, &MainWindow::onAppendMessageToTextBrowser);
    connect(ui->uploadButton, &QPushButton::clicked, this, &MainWindow::on_uploadButton_clicked);
    disconnect(ui->create_account_button, &QPushButton::clicked, this, &MainWindow::on_create_account_button_clicked);
    connect(ui->create_account_button, &QPushButton::clicked, this, &MainWindow::on_create_account_button_clicked);
    connect(ui->login_account_button, &QPushButton::clicked, this, &MainWindow::on_login_account_button_clicked);
    connect(ui->switch_to_create_account_btn, &QPushButton::clicked, this, &MainWindow::switch_to_create_account_view);
    connect(ui->switch_to_login_view, &QPushButton::clicked, this, &MainWindow::switch_to_login_account_view);
    connect(ui->add_friend_window_btn, &QPushButton::clicked, this, &MainWindow::switch_to_friends_view);
    connect(ui->user_search_button, &QPushButton::clicked, this, &MainWindow::send_friend_request);


    ui->textBrowser->setFocus();
    ui->Message_Input_Label->installEventFilter(this);

    // **SETUP END** ---------------------------------------------------------------------------------------------------


    //this will be the recieve logic eventually
    /*while (true) {

        Message message;
        set_message(&message, mess, name);
        std::string complete_message = message.date + " " + message.sender + ": " + message.message + "\n";

        //print message

        //send the message: send_message(client_socket, complete_message);

        if (strcmp(mess, "exit\n") == 0)
            break;
        memset(mess, 0, sizeof(mess));
    }
    */
}

MainWindow::~MainWindow()
{
    qDebug() << "Main window destructor called";
    networkManager->close_connection();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event){
    qDebug() << "Main window close event triggered";
    event->accept(); //event->ignore() to cancel close lmao
}
