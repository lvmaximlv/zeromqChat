#ifndef __ZMQCHATSERVER_H_
#define __ZMQCHATSERVER_H_

#include <iostream>
#include <string>
#include <unistd.h>

#include <zmq.hpp>

class CZmqChatServer
{
public:
    CZmqChatServer();
    ~CZmqChatServer();

    void run();
};

#endif