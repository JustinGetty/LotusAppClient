#include "Headers/logicmanager.h"

logicmanager::logicmanager() {

    logic_network_manager = new networkmanager;

    logic_manager_socket = logic_network_manager->setup_socket("LOGIC_MANAGEMENT", -1);
}

int logicmanager::get_logic_manager_socket()
{
    return logic_manager_socket;
}


void logicmanager::async_handle_auth_and_infrastructure(const int &logic_manager_socket)
{
    //thread this to handle login and shit
}
