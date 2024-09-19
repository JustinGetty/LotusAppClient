#include "Headers/user.h"

//construcot, called when initializing user
user::user() {


}

int user::set_user_id(int id){

    user_id = id;

    return 0;

}

int user::set_active_user_username(std::string username){
    user_username = username;

    return 0;
}


std::string user::get_active_user_username(){

    return user_username;
}

int user::get_user_id(){

    return user_id;
}
