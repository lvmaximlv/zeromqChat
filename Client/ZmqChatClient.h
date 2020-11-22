#ifndef __ZMQCHATCLIENT_H_
#define __ZMQCHATCLIENT_H_

#include <zmq.hpp>
#include <string>
#include <iostream>

class CZmqChatClient
{
public:
    CZmqChatClient();
    ~CZmqChatClient();

    void run();
};

#endif