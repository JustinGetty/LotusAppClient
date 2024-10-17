#include "Headers/mainwindow.h"
#include "ui_mainwindow.h"



/*
 TODO:
    2. Make friends request system async in its own thread and fix crashing issues of get status for adding a friend
    3. Implement sending messages
    4. Add loading buffers and animations while content/data is loading/connecting
    5. Comprehensive error handling
    6. change naming convention of all buttons, auto connecting slots likely causing issues
    7. The last action to happen when client logs in is they should be shown as online on server side

    left off:
*/

QByteArray std_string_to_qbytearray(std::string x){

    QByteArray y = QByteArray::fromRawData(x.data(), static_cast<int>(x.size()));

    return y;
}
std::string get_status(int client_socket) {
    char buffer[45];
    std::string status_message;

    while (true) {
        std::cout << "Getting status good" << std::endl;
        ssize_t status_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (status_bytes < 0) {

            return "";
        } else if (status_bytes == 0) {
            std::cerr << "Connection closed by the server." << std::endl;
            return "";
        }
        buffer[status_bytes] = '\0';

        std::cout << buffer << std::endl;
        status_message += buffer;

        if (status_message.find('|') != std::string::npos) { //npos represents end of string
            break;

        }
    }

    std::cerr << "Finished receiving status message: " << status_message << std::endl;

    // Return the message up to the delimiter
    return status_message.substr(0, status_message.find('|'));
}



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
//super important here, this needs to go into send_message_button_clicked9)        messageManager->send_message(3, byte_array, "image");

        //QMessageBox::information(this, tr("Success"), tr("Image uploaded and displayed successfully."));
    }
}


void MainWindow::handleSendMessageButtonClicked()
{
    static bool isProcessing = false;  // Static variable to persist between function calls

    if (isProcessing) {
        return;  // Prevent re-entry if already processing
    }

    isProcessing = true;  // Set the flag

    ui->Send_Message_Button->setEnabled(false);
    std::cout << "SEND MESSAGE BUTTON CLICKED" << std::endl;
    //this sucks I hate plain text type REFIG asap
    user_message = ui->Message_Input_Label->toPlainText();
    ui->Message_Input_Label->clear();

    std::string messi = user_message.toStdString();

    QVariant embed_id = ui->Send_Message_Button->property("userID");
    QString other_user_id = embed_id.toString();
    std::string user_id_std_string = other_user_id.toStdString();
    std::cout << "embed id: " << user_id_std_string << std::endl;

    // types are text or image, that's it. Dont mess that up
    std::string header = "incoming_message\n";
    ssize_t bytes_sent = send(messageManager->get_message_manager_socket(), header.c_str(), header.size(), 0);

    std::string test_message = active_user->get_active_user_username() + "\\-" + user_id_std_string + "\\+" + messi + "\\|";
    std::cout << test_message << std::endl;

    int bytes_sent_sec = send(messageManager->get_message_manager_socket(), test_message.c_str(), test_message.size(), 0);

    ui->Send_Message_Button->setEnabled(true);
    isProcessing = false;  // Reset the flag when done
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

        //eventually this needs to be actual client socket and not hard coded
        logicManager->verify_new_account(logicManager->get_logic_manager_socket(), user_pw);

        std::cout << "Verifying creation" << std::endl;
        std::string account_create_status = logicManager->get_status(logicManager->get_logic_manager_socket());
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

        //eventually this needs to be actual client socket and not hard coded
        logicManager->verify_login(logicManager->get_logic_manager_socket(), user_pw);
        //get status of login
        std::cout << "Verifying..." << std::endl;

        std::string status_msg = logicManager->get_status(logicManager->get_logic_manager_socket());
        std::cout << "Verification status: " + status_msg << std::endl;

        std::cout << "Fetching user id" << std::endl;
        //int fetched_user_id = relationsManager->fetch_user_id_from_server(username);
        int fetched_user_id = logicManager->fetch_user_id_from_server(username);

        std::cout << "Fetched user id" << std::endl;

        //need to pull in user id here too, split on pipe. server work later, simple fix
        if (status_msg == "verification_succeeded" && fetched_user_id != -1)
        {
            int set_user_id_status = active_user->set_user_id(fetched_user_id);
            if(active_user->get_user_id() == -1)
            {
                std::cerr << "ERROR GETTING USER ID, LOGIN FAILED" << std::endl;
            }
            std::cout << "USER ID AT LOGIN: " << active_user->get_user_id();
            int set_user_status = active_user->set_active_user_username(username);

            std::cout << "Set user status: " << set_user_status << std::endl;
//------CREATE THE MESSAGE AND RELATION THREADS HERE---------------
            messageManager = new messagemanager(active_user->get_user_id());
            int message_manager_socket = messageManager->get_message_manager_socket();
            messageManager->set_user_id(active_user->get_user_id());
            std::thread message_management_thread([this, message_manager_socket] () {
                messageManager->async_receive_messages(message_manager_socket, this);
            });
            message_management_thread.detach();

            relationsManager = new relationmanager(active_user->get_user_id());
            int relations_manager_socket = relationsManager->get_relation_manager_socket();
            std::thread relations_management_thread([this, relations_manager_socket] () {
                relationsManager->async_manage_requests(relations_manager_socket, this);
            });

            relations_management_thread.detach();



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

        } else if(fetched_user_id == -1)
        {
            ui->login_error_message->setText("<font color='red'>Error: Failed to get USER ID. Login failed");
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
    set_friends_main_page();


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

void MainWindow::on_refresh_friend_requests_btn_clicked()
{
    std::cout << "Refreshing friend requests" << std::endl;
    setup_friend_requests();
    std::cout << "Fetched Incoming Requests" << std::endl;
    setup_outbound_friend_requests();
    std::cout << "Fetched Outbound Requests" << std::endl;
}

void MainWindow::switch_to_friends_view()
{
    std::cout << "Switching to friends view" << std::endl;
    setup_friend_requests();
    std::cout << "Fetched Incoming Requests" << std::endl;
    setup_outbound_friend_requests();
    std::cout << "Fetched Outbound Requests" << std::endl;
    ui->main_window->hide();
    ui->friends_window->show();
}
void MainWindow::on_to_main_from_friends_btn_clicked()
{
    ui->friends_window->hide();
    ui->main_window->show();
}

void MainWindow::send_friend_request()
{
    if (!ui->username_lookup_line_edit->text().isEmpty())
    {
        std::string receiver_username = (ui->username_lookup_line_edit->text()).toStdString();
        if (receiver_username == active_user->get_active_user_username())
        {
            QString error_msg_to_display = QString::fromStdString("<font color='red'>You cannot add yourself");
            return;
        }
        receiver_username = receiver_username + "+";
        std::string data = receiver_username + std::to_string((active_user->get_user_id())) + "-" + active_user->get_active_user_username() + "|";

        relationsManager->send_friend_request(relationsManager->get_relation_manager_socket(), data);

        //crashing here, not recieving correctly
        std::string result = relationsManager->get_request_status(relationsManager->get_relation_manager_socket());
        std::cout << "Result: " << result << std::endl;

        QString error_msg_to_display = QString::fromStdString("<font color='red'>" + result);

        // Calling setText on QLabel safely in the main thread
        QMetaObject::invokeMethod(ui->friend_request_error_message,
                                  "setText",               // Method to call (as a string)
                                  Qt::QueuedConnection,    // Ensure it's queued to run in the main thread
                                Q_ARG(QString, error_msg_to_display));

        setup_outbound_friend_requests();
    }

    else {
        //display error
    }
}

QWidget* MainWindow::createWidgetWithFrame(const QString &labelText, const int &user_id) {
    QFrame *frame = new QFrame;
    frame->setFrameStyle(QFrame::Box);

    // New style to make it look nicer
    frame->setStyleSheet(
        "QFrame { "
        "background-color: #f9f9f9; "
        "border: 1px solid #dcdcdc; "
        "border-radius: 8px; "
        "padding: 8px; }"
        );

    QVBoxLayout *frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(5, 5, 5, 5); // Reduce margins to make the widget smaller

    QLabel *label = new QLabel(labelText);
    label->setStyleSheet("QLabel { color: #333333; font-size: 14px; }");

    QPushButton *accept_button = new QPushButton("Accept");
    QPushButton *decline_button = new QPushButton("Decline");

    // Style for the buttons
    accept_button->setStyleSheet(
        "QPushButton { "
        "color: white; "
        "background-color: #4CAF50; "
        "border-radius: 6px; "
        "padding: 6px 12px; }"
        "QPushButton:hover { background-color: #45a049; }"
        );

    decline_button->setStyleSheet(
        "QPushButton { "
        "color: white; "
        "background-color: #f44336; "
        "border-radius: 6px; "
        "padding: 6px 12px; }"
        "QPushButton:hover { background-color: #e53935; }"
        );

    frameLayout->addWidget(label);
    frameLayout->addWidget(accept_button);
    frameLayout->addWidget(decline_button);

    connect(accept_button, &QPushButton::clicked, this, &MainWindow::handle_accept_friend_request_button);
    connect(decline_button, &QPushButton::clicked, this, &MainWindow::handle_decline_friend_request_button);

    frame->setProperty("embed_user_id", QVariant(user_id));

    return frame;
}


QWidget* MainWindow::createWidgetNoButtons(const QString &labelText, const int &user_id) {
    QFrame *frame = new QFrame;
    frame->setFrameStyle(QFrame::Box);

    // Improved style for the frame
    frame->setStyleSheet(
        "QFrame { "
        "background-color: #f9f9f9; "
        "border: 1px solid #dcdcdc; "
        "border-radius: 8px; "
        "padding: 8px; }"
        );

    QVBoxLayout *frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(5, 5, 5, 5); // Smaller margins to make the widget more compact

    QLabel *label = new QLabel(labelText);
    label->setStyleSheet("QLabel { color: #333333; font-size: 14px; }");

    frameLayout->addWidget(label);

    frame->setProperty("embed_user_id", QVariant(user_id));

    return frame;
}

QWidget* MainWindow::createFriendWidget(const QString &labelText, const int &user_id) {
    QFrame *frame = new QFrame;
    frame->setFrameStyle(QFrame::Box);

    // New style to make it look nicer
    frame->setStyleSheet(
        "QFrame { "
        "background-color: #f9f9f9; "
        "border: 1px solid #dcdcdc; "
        "border-radius: 8px; "
        "padding: 8px; }"
        );

    QVBoxLayout *frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(5, 5, 5, 5); // Reduce margins to make the widget smaller

    QLabel *label = new QLabel(labelText);
    label->setStyleSheet("QLabel { color: #333333; font-size: 14px; }");

    QPushButton *switch_to_chat_button = new QPushButton("Switch to chat");

    // Style for the buttons
    switch_to_chat_button->setStyleSheet(
        "QPushButton { "
        "color: white; "
        "background-color: #4CAF50; "
        "border-radius: 6px; "
        "padding: 6px 12px; }"
        "QPushButton:hover { background-color: #45a049; }"
        );

    frameLayout->addWidget(label);
    frameLayout->addWidget(switch_to_chat_button);

    connect(switch_to_chat_button, &QPushButton::clicked, this, [this, user_id]() {
        handle_switch_to_chat_button(user_id);
    });


    frame->setProperty("embed_user_id", QVariant(user_id));

    return frame;
}


void MainWindow::handle_accept_friend_request_button(){
    //pass
    std::cout << "Friend Request Accepted" << std::endl;

    //recast to ensure object stays as button
    QPushButton *clicked_accept_button = qobject_cast<QPushButton *>(sender());

    if (clicked_accept_button)
    {
        QFrame *parent_frame = qobject_cast<QFrame *>(clicked_accept_button->parent());

        if (parent_frame)
        {
            QVariant user_id_variant = parent_frame->property("embed_user_id");
            int user_id_int = user_id_variant.toInt();
            std::cout << "User ID Associated with Accept: " << user_id_int << std::endl;
            int friend_update_status = relationsManager->send_friend_update(relationsManager->get_relation_manager_socket(), user_id_int, "accept_friend_request\n");
        }
    }
    setup_friend_requests();
}

void MainWindow::handle_decline_friend_request_button(){
    //pass
    std::cout << "Friend Request Decline" << std::endl;

    QPushButton *clicked_declined_button = qobject_cast<QPushButton *>(sender());

    if (clicked_declined_button)
    {
        QFrame *parent_frame = qobject_cast<QFrame *>(clicked_declined_button->parent());

        if (parent_frame)
        {
            QVariant user_id_variant = parent_frame->property("embed_user_id");
            int user_id_int = user_id_variant.toInt();
            std::cout << "User ID Associated with Accept: " << user_id_int << std::endl;
            int friend_update_status = relationsManager->send_friend_update(relationsManager->get_relation_manager_socket(), user_id_int, "decline_friend_request\n");
        }
    }
    setup_friend_requests();
}

void MainWindow::setup_friend_requests()
{
    std::vector<std::pair<std::string, int>> friend_requests = relationsManager->pull_inbound_friend_requests(relationsManager->get_relation_manager_socket());

    QWidget* scrollWidget = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

    if(friend_requests.empty())
    {
        ui->scroll_area_incoming_requests->setWidget(scrollWidget);
        return;
    }

    for (const auto &data : friend_requests)
    {
        QString labelText = QString::fromStdString(data.first);
        scrollLayout->addWidget(createWidgetWithFrame(labelText, data.second));
    }

    scrollWidget->setLayout(scrollLayout);
    ui->scroll_area_incoming_requests->setWidget(scrollWidget);  // Incoming requests scroll area
}

void MainWindow::setup_outbound_friend_requests()
{
    std::vector<std::pair<std::string, int>> outgoing_requests = relationsManager->pull_outbound_friend_requests(relationsManager->get_relation_manager_socket());

    QWidget* scrollWidget = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

    if(outgoing_requests.empty())
    {
        ui->scroll_area_outgoing_requests->setWidget(scrollWidget);  // Use a different scroll area
        return;
    }

    for (const auto &data : outgoing_requests)
    {
        QString labelText = QString::fromStdString(data.first);
        scrollLayout->addWidget(createWidgetNoButtons(labelText, data.second));
    }

    scrollWidget->setLayout(scrollLayout);
    ui->scroll_area_outgoing_requests->setWidget(scrollWidget);  // Outgoing requests scroll area
}
void MainWindow::set_friends_main_page()
{

    std::vector<std::pair<std::string, int>> friends_list = relationsManager->pull_friends_list(relationsManager->get_relation_manager_socket());

    QWidget* scrollWidget = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

    if(friends_list.empty())
    {
        ui->scrollAreaMainPageFriends->setWidget(scrollWidget);
        return;
    }

    for (const auto &data : friends_list)
    {
        std::cout << "Friend list Username: " << data.first << ", ID: " << data.second << std::endl;
        QString labelText = QString::fromStdString(data.first);
        scrollLayout->addWidget(createFriendWidget(labelText, data.second));
    }
    //scrollAreaMainPageFriends

    scrollWidget->setLayout(scrollLayout);
    ui->scrollAreaMainPageFriends->setWidget(scrollWidget);
}

void MainWindow::handle_switch_to_chat_button(const int &user_id_chat)
{
    //refactor this to pull only from memory instead of reemote server. All logs will be pulled once from server in seperate function
    std::cout << "switched to chat" << std::endl;
    /*
    std::vector<int> members_in_chat;
    members_in_chat.push_back(user_id_chat);
    */

    //this pulls any message that has either client_id as sender or receiver, with the relevant chat member as the other side
    std::vector<std::vector<std::string>> chat_logs = messageManager->get_messages_from_memory(user_id_chat);
    std::cout << "starting layout setup" << std::endl;

    if(chat_logs.empty())
    {
        //empty layout
        //maybe set text in middle that says "this chat is empty, start a conversation"
        ui->EmptyChatLabel->setText("Start a conversation! :)");
    }
    QLayout* tempLayout = ui->TextBrowserParentWidget->layout();
    if (tempLayout) {
        QLayoutItem* item;
        while ((item = tempLayout->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->setParent(nullptr);
            }
            delete item;
        }
        delete tempLayout;
    }

    // add new textBrowser to ui->TextBrowserParentWidget with chats
    currentChatTextBrowser = new QTextBrowser;
    currentChatTextBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->TextBrowserParentWidget->layout());
    layout = new QVBoxLayout(ui->TextBrowserParentWidget);
    ui->TextBrowserParentWidget->setLayout(layout);

    layout->addWidget(currentChatTextBrowser);
    currentChatTextBrowser->setStyleSheet("QTextBrowser{ color: #000000; }");

    //embed the user_id of the other user in the chat to the send message button for later use
    ui->Send_Message_Button->setProperty("userID", user_id_chat);

    for(auto row : chat_logs)
    {
        std::string sender;
        if(row[1] == active_user->get_active_user_username())
        {
            sender = "Me";
        }
        else{sender = row[1];}

        std::string data = "[" + sender + "] " + row[2];
        currentChatTextBrowser->append(QString::fromStdString(data));
    }

}

int MainWindow::get_push_button_embed_id()
{
    QVariant embedded_id = ui->Send_Message_Button->property("userID");
    int embedded_id_int = embedded_id.toInt();
    return embedded_id_int;
}

void MainWindow::addMessageToTextBrowser(const QString &message) {
    currentChatTextBrowser->append(message);  // Example implementation
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

    //refactor here. This creates a new thread for sending and recievung messages.
    active_user = new user();

    //this will be the logic manager instead, call this with fake user_id, then get it later on server side
    //server logic thread will already have the user id when verifying so can find it easy
    logicManager = new logicmanager();
    int logic_manager_socket = logicManager->get_logic_manager_socket();

    QIcon icon("/Users/justin/the_harbor/Chat/ChatWidget/ChatApplication/Resources/send_message_icon.jpeg");
    ui->Send_Message_Button->setIcon(icon);
    ui->Send_Message_Button->setIconSize(QSize(ui->Send_Message_Button->width() + 4, ui->Send_Message_Button->height()));

    std::thread logic_management_thread([this, logic_manager_socket]() {
        logicManager->async_handle_auth_and_infrastructure(logic_manager_socket);
    });
    logic_management_thread.detach();

    //std::thread manage_friends_thread([this]() { });

    //eventually connect and render these as neede, i.e in the function that loads them
    connect(ui->Send_Message_Button, &QPushButton::clicked, this, &MainWindow::handleSendMessageButtonClicked);
    connect(ui->uploadButton, &QPushButton::clicked, this, &MainWindow::on_uploadButton_clicked);
    disconnect(ui->create_account_button, &QPushButton::clicked, this, &MainWindow::on_create_account_button_clicked);
    connect(ui->create_account_button, &QPushButton::clicked, this, &MainWindow::on_create_account_button_clicked);
    connect(ui->login_account_button, &QPushButton::clicked, this, &MainWindow::on_login_account_button_clicked);
    connect(ui->switch_to_create_account_btn, &QPushButton::clicked, this, &MainWindow::switch_to_create_account_view);
    connect(ui->switch_to_login_view, &QPushButton::clicked, this, &MainWindow::switch_to_login_account_view);
    connect(ui->add_friend_window_btn, &QPushButton::clicked, this, &MainWindow::switch_to_friends_view);
    connect(ui->user_search_button, &QPushButton::clicked, this, &MainWindow::send_friend_request);
    bool isConnected = connect(ui->refresh_friend_requests_btn, &QPushButton::clicked, this, &MainWindow::on_refresh_friend_requests_btn_clicked);
    if (!isConnected) {
        qDebug() << "Connection to refresh_friend_requests_btn failed!";
    }
    connect(ui->to_main_from_friends_btn, &QPushButton::clicked, this, &MainWindow::on_to_main_from_friends_btn_clicked);

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

    //this needs to be ran for every connection (logic, relation, etc.)
    networkManager->close_connection(messageManager->get_message_manager_socket());
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event){
    qDebug() << "Main window close event triggered";
    event->accept(); //event->ignore() to cancel close lmao
}
