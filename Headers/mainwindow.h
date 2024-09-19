
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "networkmanager.h"
#include "user.h"
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

private:
    Ui::MainWindow *ui;
    QString user_message;
    networkmanager *networkManager;
    user *active_user;
    bool isProcessing = false;
    bool isLoginProcessing = false;

protected:
    //void keyPressEvent(QKeyEvent *event) override;  // Declaration in the header file
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

signals:
    void appendMessageToTextBrowser(const QString& message);

};

#endif // MAINWINDOW_H
