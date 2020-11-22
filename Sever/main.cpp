#include <iostream>

#include "ZmqChatServer.h"

int main()
{
    CZmqChatServer server;
    server.run();
    
    return 0;
}