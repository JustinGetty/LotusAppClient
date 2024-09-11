#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "networkmanager.h"
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

/*
 TODO: add sql to server siddeeee
*/


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
    if ((!ui->create_user_input->text().isEmpty()) && (!ui->create_pwd_input->text().isEmpty()))
    {
        std::string username = (ui->create_user_input->text()).toStdString();
        std::string password = (ui->create_pwd_input->text()).toStdString();

        std::string user_pw = username + "|" + password;
        QByteArray user_pw_q = QByteArray::fromRawData(user_pw.data(), static_cast<int>(user_pw.size()));
        //eventually this needs to be actual client socket and not hard coded
        networkManager->send_message(networkManager->get_client_socket(), user_pw_q, "new_user");

    }
    else
    {
        qDebug() << "Error, please enter a valid username and password";
    }
}

std::string get_status(int client_socket) {
    char buffer[1024];
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


void MainWindow::on_login_account_button_clicked()
{
    if ((!ui->user_login_name->text().isEmpty()) && (!ui->user_login_password->text().isEmpty()))
    {
        std::string username = (ui->user_login_name->text()).toStdString();
        std::string password = (ui->user_login_password->text()).toStdString();

        std::string user_pw = username + "|" + password;
        QByteArray user_pw_q = QByteArray::fromRawData(user_pw.data(), static_cast<int>(user_pw.size()));
        //eventually this needs to be actual client socket and not hard coded
        networkManager->send_message(networkManager->get_client_socket(), user_pw_q, "verify_user");

    }
    else
    {
        qDebug() << "Error, please enter a valid username and password";
    }

    //get status of login
    std::cout << "Verifying..." << std::endl;
    std::string status_msg = get_status(networkManager->get_client_socket());
    std::cout << "Verification status: " + status_msg << std::endl;

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

    ui->create_account->hide();
    ui->login_window->show();
    ui->main_window->hide();
    networkManager = new networkmanager();
    int client_socket = networkManager->get_client_socket();

    QIcon icon("/Users/justin/the_harbor/Chat/ChatWidget/ChatApplication/Resources/send_message_icon.jpeg");
    ui->Send_Message_Button->setIcon(icon);
    ui->Send_Message_Button->setIconSize(QSize(ui->Send_Message_Button->width() + 4, ui->Send_Message_Button->height()));

    std::thread receive_thread([this, client_socket]() {
        networkManager->receive_messages(client_socket, this);
    });
    receive_thread.detach();

    connect(ui->Send_Message_Button, &QPushButton::clicked, this, &MainWindow::on_Send_Message_Button_clicked);
    connect(this, &MainWindow::appendMessageToTextBrowser, this, &MainWindow::onAppendMessageToTextBrowser);
    connect(ui->uploadButton, &QPushButton::clicked, this, &MainWindow::on_uploadButton_clicked);
    connect(ui->create_account_button, &QPushButton::clicked, this, &MainWindow::on_create_account_button_clicked);
    connect(ui->login_account_button, &QPushButton::clicked, this, &MainWindow::on_login_account_button_clicked);


    ui->textBrowser->setFocus();
    ui->Message_Input_Label->installEventFilter(this);

    // **SETUP END** ---------------------------------------------------------------------------------------------------

    std::string name = "Justin"; //this will need to be user input laterr


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
