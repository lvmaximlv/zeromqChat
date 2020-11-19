//
// Hello World client
// Connect REQ socket to tcp://localhost:5555
// Sends "Hello" to server, expects "World" back
// 
#include <zmq.hpp>
#include <string>
#include <iostream>


int main()
{
    //Prepare context and sockets
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    
    std::cout << "Connecting to hello world server...\n";
    socket.connect("tcp://localhost:5555");

    //Do 10 requests, waiting each time for a response

    for(int i = 0; i != 10; ++i)
    {
        zmq::message_t request(5);
        memcpy(request.data(), "Hello", 5);
        std::cout << "Sending Hello " << i << "...\n";
        socket.send(request);

        //Get the reply
        zmq::message_t reply;
        socket.recv (&reply);
        std::cout << "Recieved World " << i <<'\n';
    }

    return 0;
}