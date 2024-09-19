#ifndef USER_H
#define USER_H
#include <string>

class MainWindow;

class user
{
public:
    user();
    int set_user_id(int id);
    int set_active_user_username(std::string username);
    std::string get_active_user_username();
    int get_user_id();

private:
    int user_id;
    std::string user_username;

};

#endif // USER_H
