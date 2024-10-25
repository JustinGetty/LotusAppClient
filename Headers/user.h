#ifndef USER_H
#define USER_H
#include <string>
#include <QImage>
#include <QBuffer>
#include <QPixmap>
class MainWindow;

class user
{
public:
    user();
    int set_user_id(int id);
    int set_active_user_username(std::string username);
    std::string get_active_user_username();
    int get_user_id();
    QByteArray get_profile_pic_arr();
    QPixmap get_profile_pic_pmap();
    int set_profile_pic_glob(QPixmap pixmap);

private:
    int user_id;
    std::string user_username;
    QPixmap profile_pic_glob_pixmap;

};

#endif // USER_H
