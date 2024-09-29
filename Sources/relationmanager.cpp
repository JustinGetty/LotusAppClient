#include "Headers/relationmanager.h"
#include "Headers/networkmanager.h"
relationmanager::relationmanager() {

    relation_network_manager = new networkmanager();
    relation_manager_socket = relation_network_manager->setup_socket("RELATION_MANAGEMENT", -1);

}

void relationmanager::async_manage_requests(const int &relation_manager_socket)
{
    //listen and make updates accordingly
}


int relationmanager::get_relation_manager_socket()
{
    return relation_manager_socket;
}
