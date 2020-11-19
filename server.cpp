#include<zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>

int main()
{
    //prepare our context and sockets
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind("tcp://*:5555");

    while(true)
    {
        zmq::message_t request;

        //Wait for the next request from client
        socket.recv (&request);
        std::cout << "Recieved Hello\n";

        //Do some work
        sleep(1);

        //Send reply back to client
        zmq::message_t reply (5);
        memcpy(reply.data(), "World", 5);
        socket.send (reply);
    }

    return 0;
}