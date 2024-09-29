#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Headers/networkmanager.h"
#include "Headers/user.h"
#include "Headers/messagemanager.h"
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
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_Send_Message_Button_clicked();
    void onAppendMessageToTextBrowser(const QString& message);
    void on_uploadButton_clicked();
    void on_create_account_button_clicked();
    void on_login_account_button_clicked();
    void switch_to_create_account_view();
    void switch_to_login_account_view();
    void switch_to_main_view_after_login();
    void set_mainview_objects_tot();
    void switch_to_friends_view();
    void send_friend_request();
    void handle_accept_friend_request_button();
    void handle_decline_friend_request_button();

private:
    Ui::MainWindow *ui;
    QString user_message;
    networkmanager *networkManager;
    messagemanager *messageManager;
    user *active_user;
    bool isProcessing = false;
    bool isLoginProcessing = false;
    QWidget* createWidgetWithFrame(const QString &labelText, const int &user_id);

protected:
    //void keyPressEvent(QKeyEvent *event) override;  // Declaration in the header file
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

signals:
    void appendMessageToTextBrowser(const QString& message);

};

#endif // MAINWINDOW_H
