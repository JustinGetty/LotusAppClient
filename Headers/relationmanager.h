#include <Headers/networkmanager.h>
#ifndef RELATIONMANAGER_H
#define RELATIONMANAGER_H

class relationmanager
{
public:
    relationmanager();
    int get_relation_manager_socket();
    void async_manage_requests(const int &relation_manager_socket);

private:
    int relation_manager_socket;
    networkmanager* relation_network_manager;
};

#endif // RELATIONMANAGER_H
