#include <zmq.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

#define within(num) (int) ((float) num *random() / (RAND_MAX + 1.0))


int main()
{
    //prepare context and publisher
    zmq::context_t context(1);
    zmq::socket_t publisher(context, ZMQ_PUB);
    publisher.bind("tcp://*:5556");
    publisher.bind("ipc://weather.ipc"); // Not usable on wondows/

    //Initialize random number generator
    srandom ((unsigned) time (NULL));
    while (1) {
        int zipcode, temperature, relhumidity;

        //get values that will fool the boss

        zipcode     = within(100000);
        temperature = within(215) - 80;
        relhumidity = within(50) + 10;

        // send messages to all subscribesr
        zmq::message_t message(20);
        snprintf((char*) message.data(), 20, "%05d %d %d", zipcode, temperature, relhumidity);
        publisher.send(message);
    }

    return 0;
}