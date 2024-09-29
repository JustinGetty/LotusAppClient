#ifndef LOGICMANAGER_H
#define LOGICMANAGER_H
#include "Headers/networkmanager.h"
class logicmanager
{
public:
    logicmanager();
    void async_handle_auth_and_infrastructure(const int &logic_manager_socket);
    int get_logic_manager_socket();

private:
    int logic_manager_socket;
    networkmanager* logic_network_manager;
};

#endif // LOGICMANAGER_H
