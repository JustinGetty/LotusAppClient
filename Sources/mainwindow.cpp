#include "Headers/mainwindow.h"
#include "ui_mainwindow.h"



/*
 TODO:
    2. Add logo to background of chat interface faded
    3. Implement async thread for inbound friend requests, same as messages, online flag on server side
    4. Add loading buffers and animations while content/data is loading/connecting
    5. error handling (where comments denote)
    6. change naming convention of all buttons, auto connecting slots likely causing issues
    7. The last action to happen when client logs in is they should be shown as online on server side
    8. Have thread that can check if user is online, if so, display symbol
    9. Longterm, switch to json and api's lmao

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

void MainWindow::set_image_to_send(QByteArray image)
{
    image_send_temp = image;
}

QByteArray MainWindow::get_image_to_send()
{
    return image_send_temp;
}

void MainWindow::set_active_image_display_flag(bool status)
{
    active_image_display_flag = status;
}
bool MainWindow::get_active_image_display_flag()
{
    return active_image_display_flag;
}


void MainWindow::handle_upload_button_clicked()
{
    //tr alloweds multi language support
    //close this when done somehow
    std::cout << "Upload Button Clicked" << std::endl;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp *.gif *.jpeg)"));

    if (!fileName.isEmpty())
    {
        QTimer::singleShot(0, this, [=]() {
            QImage image;
            if (!image.load(fileName))
            {
                QMessageBox::warning(this, tr("Error"), tr("Failed to load image :("));
                return;
            }

            QString imageHTML = QString("<img src=\"%1\" width=\"400\"/>").arg(fileName);

            QByteArray byte_array;
            QBuffer buffer(&byte_array);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG");  // format is png, change that later to be dynamic

            set_image_to_send(byte_array);

            QPixmap sendable;
            sendable.loadFromData(byte_array);
            display_image_before_send(sendable);

        });
    }
}

void MainWindow::display_image_before_send(QPixmap pixmap)
{
    std::cout << "Displaying image" << std::endl;
    set_active_image_display_flag(true);

    //ui->TextBrowserParentWidget->setFixedHeight(500);
    ui->image_display_label->setPixmap(pixmap.scaled(ui->image_display_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->display_image_widget->show();

}

void MainWindow::destruct_image_display()
{
    //delete to save memory here, FIX
    ui->display_image_widget->hide();
    ui->TextBrowserParentWidget->setFixedHeight(658);
}

void MainWindow::handle_delete_image_btn()
{
    std::cout << "Delete image button pressed" << std::endl;
    destruct_image_display();
}


void MainWindow::handleSendMessageButtonClicked()
{
    QByteArray image_data;
    int image_size = 0;
    ChatMessage message;

    // Check if an image is to be sent
    if (get_active_image_display_flag() == true) {
        image_data = get_image_to_send();
        image_size = image_data.size();
        destruct_image_display();
        message.image_arr = std::vector<char>(image_data.constBegin(), image_data.constEnd());
    }

    // Get conversation ID
    QVariant embed_id = ui->Send_Message_Button->property("embed_convo_id");
    QString convo_id_qstr = embed_id.toString();
    std::string convo_id_str = convo_id_qstr.toStdString();
    if (convo_id_str.empty()) {
        return;
    }

    static bool isProcessing = false;
    if (isProcessing) {
        return;
    }
    isProcessing = true;

    ui->Send_Message_Button->setEnabled(false);
    QString user_message = ui->Message_Input_Label->toPlainText();
    ui->Message_Input_Label->clear();
    std::string messi = user_message.toStdString();

    if (messi.empty() && image_size == 0) {
        // No message content or image to send
        ui->Send_Message_Button->setEnabled(true);
        isProcessing = false;
        return;
    }

    message.message_content = messi;
    message.conversation_id = convo_id_qstr.toInt();
    message.timestamp = std::time(nullptr);
    message.message_id = -1; // Assuming -1 indicates a new message
    message.sender_id = active_user->get_user_id();
    message.sender_username = active_user->get_active_user_username();

    // Build message metadata
    std::string header = "incoming_message\n";
    send(messageManager->get_message_manager_socket(), header.c_str(), header.size(), 0);

    std::string message_metadata = active_user->get_active_user_username() + "\\-" + convo_id_str + "\\+" + messi + "\\$" + std::to_string(image_size) + "\\|";
    send(messageManager->get_message_manager_socket(), message_metadata.c_str(), message_metadata.size(), 0);

    // Send image data if present
    if (image_size > 0) {
        send(messageManager->get_message_manager_socket(), image_data.constData(), image_size, 0);
    }

    // Append the message to the UI
    appendMessageToChat(message);
    messageManager->append_to_mem_struct(message.conversation_id, message);

    ui->Send_Message_Button->setEnabled(true);
    isProcessing = false;
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

    if ((!ui->user_login_name_2->text().isEmpty()) && (!ui->user_login_password_2->text().isEmpty()))
    {
        std::string username = (ui->user_login_name_2->text()).toStdString();
        std::string password = (ui->user_login_password_2->text()).toStdString();
        std::string user_pw = username + "|" + password;

        //eventually this needs to be actual client socket and not hard coded
        logicManager->verify_login(logicManager->get_logic_manager_socket(), user_pw);
        //get status of login
        std::cout << "Verifying..." << std::endl;

        std::string status_msg = logicManager->get_status(logicManager->get_logic_manager_socket());
        std::cout << "Verification status: " + status_msg << std::endl;

        std::cout << "Fetching user id" << std::endl;
        //int fetched_user_id = relationsManager->fetch_user_id_from_server(username);

        //breaks here often
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
            std::cout << "\nFetching Profile Picture" << std::endl;
            QPixmap profile_picture = logicManager->pull_profile_picture();

            int set_pfp_status = active_user->set_profile_pic_glob(profile_picture);

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
            std::vector<std::pair<std::string, int>> friends_list_persistent_memory = relationsManager->pull_friends_list(relationsManager->get_relation_manager_socket());
            relationsManager->set_friends_list_mem(friends_list_persistent_memory);
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
    //ui->active_user_label->setText(username_text);
    relationsManager->update_conversations_glob();
    set_conversations_main_page();
    QPixmap pixmap = active_user->get_profile_pic_pmap();
    ui->profile_pic_main_label->setPixmap(pixmap.scaled(ui->profile_pic_main_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    line->hide();
    line2->hide();
    line3->hide();
}

void MainWindow::switch_to_create_account_view()
{
    ui->login_mst_object_widget->hide();
    ui->create_mst_object_widget->show();
}

void MainWindow::switch_to_login_account_view()
{
    ui->create_mst_object_widget->hide();
    ui->login_mst_object_widget->show();
}

void MainWindow::switch_to_main_view_after_login()
{
    //re-render main friends and others here
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
    setup_friends_list();
}
QWidget* MainWindow::createTextOnlyWidget(const QString &labelText, const int &user_id)
{
    QFrame *frame = new QFrame;
    frame->setFrameStyle(QFrame::Box);

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

    frameLayout->addWidget(label);

    frame->setProperty("embed_user_id", QVariant(user_id));

    return frame;
}


void MainWindow::setup_friends_list()
{
    //pull all friends
    relationsManager->update_friends_list_mem();
    std::vector<std::pair<std::string, int>> friends_list = relationsManager->get_friends_list_mem();

    QWidget* scrollWidget = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

    if(friends_list.empty())
    {
        ui->scroll_area_friends_list->setWidget(scrollWidget);
        return;
    }

    for (const auto &data : friends_list)
    {
        std::cout << "Friend list Username: " << data.first << ", ID: " << data.second << std::endl;
        QString labelText = QString::fromStdString(data.first);
        scrollLayout->addWidget(createTextOnlyWidget(labelText, data.second));
    }
    //scrollAreaMainPageFriends

    scrollWidget->setLayout(scrollLayout);
    ui->scroll_area_friends_list->setWidget(scrollWidget);
}

void MainWindow::switch_to_friends_view()
{
    std::cout << "Switching to friends view" << std::endl;
    setup_friend_requests();
    std::cout << "Fetched Incoming Requests" << std::endl;
    setup_outbound_friend_requests();
    std::cout << "Fetched Outbound Requests" << std::endl;
    setup_friends_list();
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
/*
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
*/

//friends widget on main page
QWidget* MainWindow::createMainConversationWidget(std::vector<std::pair<std::string, int>> iso_convo)
{

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

    std::string convo_name = iso_convo[0].first;
    int convo_id = iso_convo[0].second;
    iso_convo.erase(iso_convo.begin());

    QLabel *conversationName = new QLabel();
    if(iso_convo.size() == 2)
    {
        std::cout << "Condition 1" << std::endl;
        std::string convo_text = iso_convo[0].first;
        conversationName->setText(QString::fromStdString(convo_text));
    }

    else if(convo_name.empty() && iso_convo.size() == 3)
    {
        std::cout << "Condition 2" << std::endl;
        std::string convo_text = iso_convo[0].first + " and " + iso_convo[1].first;
        conversationName->setText(QString::fromStdString(convo_text));
    }
    else if (convo_name.empty() && iso_convo.size() > 3)
    {
        std::cout << "Condition 3" << std::endl;
        std::string convo_text = iso_convo[1].first + ", " + iso_convo[2].first + ", others...";
        conversationName->setText(QString::fromStdString(convo_text));
    }
    else if (!convo_name.empty())
    {
        std::cout << "Condition 4" << std::endl;
        conversationName->setText(QString::fromStdString(convo_name));
    }
    conversationName->setStyleSheet("QLabel { color: #333333; font-size: 14px; }");

    QPushButton *switch_to_chat_button = new QPushButton("Switch to chat");
    QPushButton *change_conversation_name = new QPushButton("Rename Chat");

    switch_to_chat_button->setStyleSheet(
        "QPushButton { "
        "color: white; "
        "background-color: #4CAF50; "
        "border-radius: 6px; "
        "padding: 6px 12px; }"
        "QPushButton:hover { background-color: #45a049; }"
        );
    change_conversation_name->setStyleSheet(
        "QPushButton { "
        "color: white; "
        "background-color: #4CAF50; "
        "border-radius: 6px; "
        "padding: 6px 12px; }"
        "QPushButton:hover { background-color: #45a049; }"
        );

    frameLayout->addWidget(conversationName);
    frameLayout->addWidget(switch_to_chat_button);
    frameLayout->addWidget(change_conversation_name);

    std::vector<int> users_in_chat;
    for(int i = 1; i < iso_convo.size(); i++)
    {
        users_in_chat.push_back(iso_convo[i].second);
    }
    connect(switch_to_chat_button, &QPushButton::clicked, this, [this, convo_id]() {
        handle_switch_to_chat_button(convo_id);
    });

    /*
    QList<int> qList;
    for (int value : users_in_chat) {
        qList.append(value);
    }
    */

    //QVariant variant = QVariant::fromValue(qList);

    // Retrieve the QList<int> back from the QVariant
    //QList<int> retrievedList = variant.value<QList<int>>();

    frame->setProperty("embed_convo_id", convo_id);

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
    set_conversations_main_page();
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
void MainWindow::set_conversations_main_page()
{

    std::vector<std::vector<std::pair<std::string, int>>> conversations = relationsManager->get_conversations_mem();

    QWidget* scrollWidget = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

    if(conversations.empty())
    {
        ui->scrollAreaMainPageFriends->setWidget(scrollWidget);
        return;
    }

    for(auto &conversation : conversations)
    {

        scrollLayout->addWidget(createMainConversationWidget(conversation));
    }

    scrollWidget->setLayout(scrollLayout);
    ui->scrollAreaMainPageFriends->setWidget(scrollWidget);
    ui->scrollAreaMainPageFriends->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollAreaMainPageFriends->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollAreaMainPageFriends->setStyleSheet(
        "QScrollArea {"
        "    border-radius: 0px;"         // Ensures no rounded corners on the scroll area
        "    background: #ffffff;"       // Set consistent background color
        "    border: 1px solid #dcdcdc;" // Border for the scroll area
        "}"
        "QScrollArea > QWidget > QWidget {"
        "    border-radius: 0px;"         // Ensures no rounded corners for the viewport
        "    background: #ffffff;"       // Match background color
        "}"
        "QScrollBar:vertical {"
        "    background: #f0f0f0;"        // Background color of the vertical scrollbar
        "    width: 12px;"                // Width of the vertical scrollbar
        "    margin: 0px 0px 0px 0px;"    // Margins
        "    border: 1px solid #dcdcdc;"  // Border around the scrollbar
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #b0b0b0;"        // Color of the handle
        "    min-height: 20px;"           // Minimum height of the handle
        "    border-radius: 4px;"         // Rounded corners
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"                // Removes the up and down arrow buttons
        "    subcontrol-origin: margin;"
        "}"
        "QScrollBar:horizontal {"
        "    background: #f0f0f0;"        // Background color of the horizontal scrollbar
        "    height: 12px;"               // Height of the horizontal scrollbar
        "    margin: 0px 0px 0px 0px;"    // Margins
        "    border: 1px solid #dcdcdc;"  // Border around the scrollbar
        "}"
        "QScrollBar::handle:horizontal {"
        "    background: #b0b0b0;"        // Color of the handle
        "    min-width: 20px;"            // Minimum width of the handle
        "    border-radius: 4px;"         // Rounded corners
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "    width: 0px;"                 // Removes the left and right arrow buttons
        "    subcontrol-origin: margin;"
        "}"
        );

    ui->scrollAreaMainPageFriends->verticalScrollBar()->hide();
    ui->scrollAreaMainPageFriends->horizontalScrollBar()->hide();

    // Create a timer to auto-hide the scrollbars
    QTimer *hideScrollBarTimer = new QTimer(this);
    hideScrollBarTimer->setInterval(2000); // Hide after 2 seconds of inactivity
    hideScrollBarTimer->setSingleShot(true);

    // Connect the scroll bar signals to show them on interaction
    connect(ui->scrollAreaMainPageFriends->verticalScrollBar(), &QScrollBar::valueChanged, this, [=]() {
        ui->scrollAreaMainPageFriends->verticalScrollBar()->show();
        hideScrollBarTimer->start(); // Restart timer on every scroll
    });

    connect(ui->scrollAreaMainPageFriends->horizontalScrollBar(), &QScrollBar::valueChanged, this, [=]() {
        ui->scrollAreaMainPageFriends->horizontalScrollBar()->show();
        hideScrollBarTimer->start(); // Restart timer on every scroll
    });

    // Auto-hide scrollbars when the timer times out
    connect(hideScrollBarTimer, &QTimer::timeout, this, [=]() {
        ui->scrollAreaMainPageFriends->verticalScrollBar()->hide();
        ui->scrollAreaMainPageFriends->horizontalScrollBar()->hide();
    });
}


void MainWindow::handle_switch_to_chat_button(const int &convo_id)
{
    std::cout << "Switched to chat with conversation ID: " << convo_id << std::endl;
    std::vector<ChatMessage> chat_logs;

    // Update UI elements visibility
    ui->EmptyChatLabel->hide();
    ui->Send_Message_Button->show();
    ui->Message_Input_Label->show();
    ui->uploadButton->show();

    chatScrollArea->setStyleSheet(
        "QScrollBar:vertical {"
        "    background: #f0f0f0;"         // Background color of the scrollbar
        "    width: 12px;"                 // Width of the vertical scrollbar
        "    margin: 0px 0px 0px 0px;"     // Margins
        "    border: 1px solid #dcdcdc;"   // Border around the scrollbar
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #b0b0b0;"         // Color of the handle
        "    min-height: 20px;"            // Minimum height of the handle
        "    border-radius: 4px;"          // Rounded corners
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"                 // Removes the up and down arrow buttons
        "    subcontrol-origin: margin;"
        "}"
        "QScrollBar:horizontal {"
        "    background: #f0f0f0;"         // Background color of the horizontal scrollbar
        "    height: 12px;"                // Height of the horizontal scrollbar
        "    margin: 0px 0px 0px 0px;"     // Margins
        "    border: 1px solid #dcdcdc;"   // Border around the scrollbar
        "}"
        "QScrollBar::handle:horizontal {"
        "    background: #b0b0b0;"         // Color of the handle
        "    min-width: 20px;"             // Minimum width of the handle
        "    border-radius: 4px;"          // Rounded corners
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "    width: 0px;"                  // Removes the left and right arrow buttons
        "    subcontrol-origin: margin;"
        "}"
        );

    // Retrieve chat logs from memory
    chat_logs = messageManager->get_messages_from_memory(convo_id);
    std::cout << "Retrieved " << chat_logs.size() << " messages from memory." << std::endl;

    // Clear existing widgets in messages container
    if (containerLayout) {
        clearLayout(containerLayout);
    } else {
        qDebug() << "containerLayout is nullptr!";
        // Handle error if necessary
    }

    // Check if there are any messages in the conversation
    if (chat_logs.empty()) {
        ui->EmptyChatLabel->setText("Start a conversation! :)");
        ui->EmptyChatLabel->show();
    } else {
        ui->EmptyChatLabel->hide();

        // Iterate through each ChatMessage and add it to the UI
        for (const auto& message : chat_logs)
        {
            // Create a widget for the chat message
            QWidget* messageWidget = createChatMessageWidget(message);

            // Add the message widget to the container layout
            containerLayout->addWidget(messageWidget);
        }

        // Remove any stretch at the end to prevent messages from being pushed apart
        // Do not add containerLayout->addStretch();
    }

    // Embed the conversation ID into the Send_Message_Button for later use
    ui->Send_Message_Button->setProperty("embed_convo_id", convo_id);

    // Scroll to the bottom to display the latest message
    QTimer::singleShot(100, this, [this]() {
        if (chatScrollArea && chatScrollArea->verticalScrollBar()) {
            chatScrollArea->verticalScrollBar()->setValue(chatScrollArea->verticalScrollBar()->maximum());
        }
    });
}




int MainWindow::get_push_button_embed_id()
{
    QVariant embedded_id = ui->Send_Message_Button->property("userID");
    int embedded_id_int = embedded_id.toInt();
    return embedded_id_int;
}

void MainWindow::clearLayout(QLayout* layout)
{
    if (!layout)
        return;

    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            delete widget; // Delete the widget immediately
        } else if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout); // Recursively clear child layouts
            delete childLayout; // Delete the child layout
        }
        delete item; // Delete the layout item
    }
}


QWidget* MainWindow::createChatMessageWidget(const ChatMessage& message)
{
    // Create the main widget for the message bubble
    QWidget* messageWidget = new QWidget();
    messageWidget->setProperty("messageId", QVariant(message.message_id));
    messageWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum); // Prevent vertical stretching

    // Create the main layout (horizontal) to align messages left or right
    QHBoxLayout* mainLayout = new QHBoxLayout(messageWidget);
    mainLayout->setContentsMargins(10, 5, 10, 5);
    mainLayout->setSpacing(5);

    // Determine if the message is sent or received
    bool isSentByMe = (message.sender_username == active_user->get_active_user_username());

    // Create a container widget for the message content (bubble)
    QWidget* bubbleWidget = new QWidget();
    bubbleWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum); // Prevent vertical stretching
    bubbleWidget->setMaximumWidth(400); // Adjust as needed

    // Set background color depending on sender
    QString bubbleStyle = isSentByMe
                              ? "background-color: #064789; border-radius: 10px; padding: 8px;"
                              : "background-color: #EBF2FA; border-radius: 10px; padding: 8px;";
    bubbleWidget->setStyleSheet(bubbleStyle);

    // Create a vertical layout for the bubbleWidget
    QVBoxLayout* bubbleLayout = new QVBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(5, 2, 5, 2); // Reduce top and bottom margins
    bubbleLayout->setSpacing(2); // Reduce spacing between elements

    // Header layout (only username for received messages)
    if (!isSentByMe) {
        QHBoxLayout* headerLayout = new QHBoxLayout();
        headerLayout->setContentsMargins(0, 0, 0, 0); // Remove extra margins
        headerLayout->setSpacing(2); // Reduce spacing between elements

        // Sender label
        QString senderLabel = "<b>" + QString::fromStdString(message.sender_username) + "</b>";
        QLabel* usernameLabel = new QLabel(senderLabel);
        usernameLabel->setStyleSheet("font-size: 12px; color: #333333;"); // Adjust color as needed
        usernameLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        headerLayout->addWidget(usernameLabel);
        headerLayout->addStretch(); // Push the username to the left

        bubbleLayout->addLayout(headerLayout);
    }

    // Optional picture
    if (!message.image_arr.empty()) {
        QByteArray imageData(reinterpret_cast<const char*>(message.image_arr.data()), static_cast<int>(message.image_arr.size()));
        QPixmap pixmap;
        if (pixmap.loadFromData(imageData)) {
            QLabel* pictureLabel = new QLabel();
            pictureLabel->setAlignment(Qt::AlignCenter);
            pictureLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

            // Scale image to fit within the bubble
            int maxWidth = bubbleWidget->maximumWidth() - 20; // Adjust for padding
            QPixmap scaledPixmap = pixmap.scaledToWidth(maxWidth, Qt::SmoothTransformation);
            pictureLabel->setPixmap(scaledPixmap);

            bubbleLayout->addWidget(pictureLabel);
        } else {
            QLabel* errorLabel = new QLabel("[Failed to load image]");
            errorLabel->setStyleSheet("color: red; font-style: italic;");
            errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            bubbleLayout->addWidget(errorLabel);
        }
    }

    // Message text at the bottom
    QString messageText = QString::fromStdString(message.message_content).trimmed();
    if (!messageText.isEmpty()) {
        QLabel* textLabel = new QLabel(messageText);
        textLabel->setWordWrap(true);
        // Added padding-top and padding-bottom for increased vertical padding
        textLabel->setStyleSheet(isSentByMe
                                     ? "color: #FFFFFF; font-size: 15px; margin: 0px; padding-top: 4px; padding-bottom: 4px; padding-left: 0px; padding-right: 0px;"
                                     : "color: #333333; font-size: 15px; margin: 0px; padding-top: 4px; padding-bottom: 4px; padding-left: 0px; padding-right: 0px;");
        textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        textLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        textLabel->setContentsMargins(0, 0, 0, 0); // Ensure no extra margins are applied

        bubbleLayout->addWidget(textLabel);
    }

    // Adjust alignment based on sender
    if (isSentByMe) {
        mainLayout->addStretch();
        mainLayout->addWidget(bubbleWidget);
    } else {
        mainLayout->addWidget(bubbleWidget);
        mainLayout->addStretch();
    }

    return messageWidget;
}




void MainWindow::appendMessageToChat(const ChatMessage& message)
{
    if (!containerLayout) {
        qDebug() << "containerLayout is nullptr!";
        return;
    }

    // Create a widget for the chat message
    QWidget* messageWidget = createChatMessageWidget(message);

    // Add the message widget to the container layout
    containerLayout->addWidget(messageWidget);

    // Scroll to the bottom to display the latest message
    QTimer::singleShot(0, this, [this]() {
        if (chatScrollArea && chatScrollArea->verticalScrollBar()) {
            chatScrollArea->verticalScrollBar()->setValue(chatScrollArea->verticalScrollBar()->maximum());
        }
    });
}




void MainWindow::addMessageToTextBrowser(ChatMessage message)
{
    // Create the message widget
    QWidget* messageWidget = createChatMessageWidget(message);

    // Get the parent layout
    QVBoxLayout* parentLayout = qobject_cast<QVBoxLayout*>(ui->TextBrowserParentWidget->layout());
    if (!parentLayout) {
        // If no layout exists, create one
        parentLayout = new QVBoxLayout(ui->TextBrowserParentWidget);
        parentLayout->setAlignment(Qt::AlignTop);
        ui->TextBrowserParentWidget->setLayout(parentLayout);
    }

    // Add the message widget to the parent layout before the spacer
    // Find the spacer (if any) and insert before it
    int spacerIndex = -1;
    for (int i = 0; i < parentLayout->count(); ++i) {
        QSpacerItem* spacer = parentLayout->itemAt(i)->spacerItem();
        if (spacer) {
            spacerIndex = i;
            break;
        }
    }

    if (spacerIndex != -1) {
        parentLayout->insertWidget(spacerIndex, messageWidget);
    } else {
        parentLayout->addWidget(messageWidget);
    }

    // Scroll to the bottom to show the latest message
    // Assuming TextBrowserParentWidget is inside a QScrollArea
    QWidget* parentWidget = ui->TextBrowserParentWidget->parentWidget();
    QScrollArea* scrollArea = qobject_cast<QScrollArea*>(parentWidget);
    if (scrollArea) {
        QTimer::singleShot(100, scrollArea, [scrollArea]() {
            scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
        });
    }

}

void MainWindow::handle_refresh_conversations_button()
{
    std::cout << "Refreshing conversations..." << std::endl;
    relationsManager->update_conversations_glob();
    set_conversations_main_page();
}

QWidget* MainWindow::createWidgetWithCheckBox(const QString &labelText, const int &user_id)
{
    QFrame *frame = new QFrame;
    frame->setFrameStyle(QFrame::Box);

    frame->setStyleSheet(
        "QFrame { "
        "background-color: #f9f9f9; "
        "border: 1px solid #dcdcdc; "
        "border-radius: 8px; "
        "padding: 8px; }"
        );

    QVBoxLayout *frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(5, 5, 5, 5);

    QLabel *label = new QLabel(labelText);
    label->setStyleSheet("QLabel { color: #333333; font-size: 14px; }");

    QCheckBox *checkBox = new QCheckBox();
    checkBox->setProperty("UserID", user_id);

    frameLayout->addWidget(label);
    frameLayout->addWidget(checkBox);

    connect(checkBox, &QCheckBox::stateChanged, this, [this, user_id](int state) {
        handle_check_box_state_change(state, user_id);
    });

    return frame;


}
void MainWindow::handle_check_box_state_change(int state, int user_id)
{
    if(state == Qt::Checked) {
        std::cout << "Checkbox checked, ID: " << user_id << std::endl;
        //Add to list of users in chat array
        relationsManager->add_to_temp_convo_list(user_id);
        int count = 0;
        for (auto &i : relationsManager->get_temp_convo_list())
        {
            std::cout << "Conversation Member " << count << ": " << i << std::endl;
            count += 1;
        }
    } else {
        std::cout << "Checkbox is unchecked, ID: " << user_id << std::endl;
        //Remove from list of users in chat array
        relationsManager->remove_from_temp_convo_list(user_id);
        int count = 0;
        for (auto &i : relationsManager->get_temp_convo_list())
        {
            std::cout << "Conversation Member " << count << ": "  << i << std::endl;
            count += 1;
        }
    }

}


void MainWindow::handle_new_conversation_button()
{
    //pull all friends, show check buttons next to them as friends to add. Add them with embnedded user_id in frame or button
    ui->new_conversation_frame->show();
    ui->new_conversation_frame->raise();
    //show loading icon here

    std::vector<std::pair<std::string, int>> friends_list = relationsManager->get_friends_list_mem();

    QWidget* scrollWidget = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

    if(friends_list.empty())
    {
        ui->AddToConversationScrollArea->setWidget(scrollWidget);
        return;
    }

    for (const auto &row : friends_list)
    {
        QString labelText = QString::fromStdString(row.first);
        scrollLayout->addWidget(createWidgetWithCheckBox(labelText, row.second));
    }
    scrollWidget->setLayout(scrollLayout);
    ui->AddToConversationScrollArea->setWidget(scrollWidget);
}

void MainWindow::handle_confirm_new_convo_button()
{
    std::vector<int> conversation_members = relationsManager->get_temp_convo_list();
    for(auto &value : conversation_members)
    {
        std::cout << "Conversation Member ID: " << value << std::endl;
    }
    relationsManager->insert_new_conversation(conversation_members);

    relationsManager->clear_temp_convo_list();
    set_conversations_main_page();
    ui->new_conversation_frame->hide();
}

void MainWindow::handle_cancel_new_convo_button()
{
    relationsManager->clear_temp_convo_list();
    ui->new_conversation_frame->hide();
}

void MainWindow::handle_change_profile_pic_button()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Image"),
        "",
        tr("Image Files (*.png *.jpg *.bmp)")
    );
    if (filePath.isEmpty())
    {
        return;
    }

    QImage image;
    if (!image.load(filePath))
    {
        QMessageBox::warning(this, tr("Open Image"), tr("The image could not be loaded"));
        return;
    }
    QPixmap pixmap = QPixmap::fromImage(image);
    ui->profile_pic_label->setPixmap(pixmap.scaled(ui->profile_pic_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::handle_confirm_pfp_change_btn()
{
    QPixmap pixmap = ui->profile_pic_label->pixmap(Qt::ReturnByValue);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, tr("Confirm Profile Picture"), tr("No image selected."));
        return;
    }

    QByteArray img_byte_array;
    QBuffer buffer(&img_byte_array);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");

    int status = logicManager->send_profile_picture_change(img_byte_array);

    if (status == 0)
    {
        //display to screen eventually
        active_user->set_profile_pic_glob(pixmap);
        ui->profile_pic_main_label->setPixmap(pixmap.scaled(ui->profile_pic_main_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        std::cout << "good profile pic change, continue" << std::endl;
    }
    else
    {
        std::cout << "Bad profile pic change, Server Error" << std::endl;
    }
}

void MainWindow::to_settings_from_main_button()
{
    ui->main_window->hide();
    ui->settings_window->show();
}

void MainWindow::to_main_from_settings_button()
{
    ui->settings_window->hide();
    ui->main_window->show();
}


/*
 UI


*/

void MainWindow::setRoundedCorners() {
    QPainterPath path;
    path.addRoundedRect(centralWidget()->rect(), 10, 10);
    centralWidget()->setMask(QRegion(path.toFillPolygon().toPolygon()));
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && ui->header_buttons_widget->geometry().contains(event->pos())) {
        isDragging = true;
        dragStartPosition = event->globalPos() - this->frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (isDragging && (event->buttons() & Qt::LeftButton)) {
        QPoint newTopLeft = event->globalPos() - dragStartPosition;
        this->move(newTopLeft);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        event->accept();
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

    ui->Send_Message_Button->hide();
    ui->Message_Input_Label->hide();
    ui->uploadButton->hide();
    ui->new_conversation_frame->hide();
    ui->settings_window->hide();

    ui->friends_window->hide();
    ui->main_window->hide();
    ui->create_account->hide();
    ui->login_window->show();
    ui->display_image_widget->hide();

    ui->create_mst_object_widget->hide();

    /* ---- UI SETUP ------- */

    /* General Window Setup */
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    centralWidget()->setStyleSheet("border-radius: 10px; background-color: rgba(0,0,0,0.0);");  // Adjust color and radius as needed

    // Ensure the central widget fills the main window
    centralWidget()->setAutoFillBackground(true);
    centralWidget()->setStyleSheet("border-radius: 15px; background-color: #f0f0f0;");
    setCentralWidget(centralWidget());

    // Call the method to set rounded corners
    setRoundedCorners();


    /*
    setWindowFlags(Qt::FramelessWindowHint);
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::close);
    connect(ui->minimizeButton, &QPushButton::clicked, this, &MainWindow::showMinimized);
    connect(ui->maximizeButton, &QPushButton::clicked, this, &MainWindow::showMaximized);
    // Assuming 'headerWidget' is the name of your custom header widget
    ui->headerWidget->setStyleSheet("background-color: #3498db; color: white;");
    */


    /*-------Login Window Setup ------- */

    QString htmlText = R"(
    <div style="line-height: 1.6; font-size: 18px; color: #f0f0f0; font-family: 'Helvetica Neue', Arial, sans-serif; font-weight: 300; padding: 10px;">
        <p style="margin-bottom: 15px;">Don't give us your personal data, we don't want it!</p>
        <p style="margin-bottom: 15px;">Your data is encrypted, secure, and never sold.</p>
        <p style="margin-bottom: 15px;">Fast and reliable socket stream communication.</p>
    </div>
)";
    //ui->login_about_desc_label_2->setText(htmlText);

    htmlText = R"(
    <div style="line-height: 1.6; font-size: 18px; color: #f0f0f0; font-family: 'Helvetica Neue', Arial, sans-serif; font-weight: 300; padding: 10px;">
        <p style="margin-bottom: 15px;">Encrypted, Safe, Secure. Lotus.</p>
    </div>
)";
    ui->login_features_desc_label->setText(htmlText);


    QLinearGradient gradient(0, 0, 1, 1);
    gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
    gradient.setColorAt(0, QColor(235, 242, 250));  // #26373b(green ish)
    gradient.setColorAt(0.35, QColor(66, 122, 161));
    gradient.setColorAt(1, QColor(6, 71, 137));  // #33494c(green) rgb(38, 93, 223) blue

    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(gradient));
    ui->login_background->setAutoFillBackground(true);
    ui->login_background->setPalette(palette);


    line = new QFrame(this);
    line->setGeometry(629, 104, 2, 596);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Plain);
    line->show();

    line2 = new QFrame(this);
    line2->setGeometry(837, 338, 280, 2);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Plain);
    line2->show();

    line3 = new QFrame(this);
    line3->setGeometry(837, 428, 280, 2);
    line3->setFrameShape(QFrame::HLine);
    line3->setFrameShadow(QFrame::Plain);
    line3->show();

    QPixmap logo_pixmap("/Users/justin/the_harbor/Chat/ChatWidget/LotusAppClient/Resources/lotus-app-icon-no-bckgrnd.png");
    if (logo_pixmap.isNull()) {
        std::cout << "Failed to load image." << std::endl;
    } else {
        QPixmap cropped_pixmap = logo_pixmap.scaled(ui->login_logo_label->size(),
                                                    Qt::KeepAspectRatioByExpanding,
                                                    Qt::SmoothTransformation);

        // Set the cropped pixmap to the label
        ui->login_logo_label->setPixmap(cropped_pixmap);
    }



    /*-----Main Message Area Setup---------*/
    // In your MainWindow constructor or initialization function
    parentLayout = new QVBoxLayout(ui->TextBrowserParentWidget);
    parentLayout->setContentsMargins(0, 0, 0, 0);
    parentLayout->setSpacing(0);
    ui->TextBrowserParentWidget->setLayout(parentLayout);

    chatScrollArea = new QScrollArea(ui->TextBrowserParentWidget);
    chatScrollArea->setWidgetResizable(true);
    chatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chatScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    parentLayout->addWidget(chatScrollArea);

    messagesContainerWidget = new QWidget();
    chatScrollArea->setWidget(messagesContainerWidget);

    containerLayout = new QVBoxLayout(messagesContainerWidget);
    containerLayout->setAlignment(Qt::AlignTop);
    containerLayout->setContentsMargins(10, 10, 10, 10);
    containerLayout->setSpacing(10);
    messagesContainerWidget->setLayout(containerLayout);


    /*-----Main Window Gen Setup*/


    QIcon new_convo_icon("/Users/justin/the_harbor/Chat/ChatWidget/LotusAppClient/Resources/square-pencil-draw-edit-512.png");
    ui->NewConversationButton->setIcon(new_convo_icon);
    ui->NewConversationButton->setIconSize(QSize(ui->NewConversationButton->width(), ui->NewConversationButton->height()));


    /*---- END UI SETUP--------*/




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
    disconnect(ui->uploadButton, &QPushButton::clicked, this, &MainWindow::handle_upload_button_clicked);
    connect(ui->uploadButton, &QPushButton::clicked, this, &MainWindow::handle_upload_button_clicked);
    disconnect(ui->create_account_button, &QPushButton::clicked, this, &MainWindow::on_create_account_button_clicked);
    connect(ui->create_account_button, &QPushButton::clicked, this, &MainWindow::on_create_account_button_clicked);
    connect(ui->login_account_button_2, &QPushButton::clicked, this, &MainWindow::on_login_account_button_clicked);
    connect(ui->switch_to_create_account_btn_2, &QPushButton::clicked, this, &MainWindow::switch_to_create_account_view);
    connect(ui->switch_to_login_view, &QPushButton::clicked, this, &MainWindow::switch_to_login_account_view);
    connect(ui->add_friend_window_btn, &QPushButton::clicked, this, &MainWindow::switch_to_friends_view);
    connect(ui->user_search_button, &QPushButton::clicked, this, &MainWindow::send_friend_request);
    connect(ui->RefreshConversations, &QPushButton::clicked, this, &MainWindow::handle_refresh_conversations_button);
    connect(ui->NewConversationButton, &QPushButton::clicked, this, &MainWindow::handle_new_conversation_button);
    connect(ui->ConfirmNewConversationButton, &QPushButton::clicked, this, &MainWindow::handle_confirm_new_convo_button);
    connect(ui->CancelNewConversationButton, &QPushButton::clicked, this, &MainWindow::handle_cancel_new_convo_button);
    connect(ui->change_pfp_button, &QPushButton::clicked, this, &MainWindow::handle_change_profile_pic_button);
    connect(ui->to_main_from_settings_btn, &QPushButton::clicked, this, &MainWindow::to_main_from_settings_button);
    connect(ui->to_settings_from_main_btn, &QPushButton::clicked, this, &MainWindow::to_settings_from_main_button);
    connect(ui->confirm_profile_pic_btn, &QPushButton::clicked, this, &MainWindow::handle_confirm_pfp_change_btn);
    connect(ui->delete_image_send, &QPushButton::clicked, this, &MainWindow::handle_delete_image_btn);
    // Connect buttons to functions to handle close, minimize, and maximize

    connect(ui->exit_app_button, &QPushButton::clicked, this, []() {
        QApplication::quit();
    });
    connect(ui->minimize_app_button, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(ui->fullscreen_app_button, &QPushButton::clicked, this, [this]() {
        if (this->window()->isMaximized()) {
            this->window()->showNormal();
        } else {
            this->window()->showMaximized();
        }
    });

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
