//
// Weather update client 
// Connects SUB socket to tcp://localhost:5556
// Collects weather updates and finds avg temp in zipcode

#include <zmq.hpp>
#include <iostream>
#include <sstream>

int main()
{
    zmq::context_t context(1);

    // Socket to talk to server
    std::cout << "Collecting updates from weather server...\n";
    zmq::socket_t subscriber(context, ZMQ_SUB);
    try{
        subscriber.connect("tcp://localhost:5556");
    }
    catch(...)
    {
        std::cout << "error\n";
    }
    // Subscribe to zipcode, default is NYC, 10001
    const char *filter = "10001 ";
    subscriber.setsockopt(ZMQ_SUBSCRIBE, filter, strlen(filter));

    // process 100 updates;
    int update_nbr;
    long total_temp = 0;

    for (update_nbr = 0; update_nbr < 100; ++update_nbr) {
        zmq::message_t update;
        int zipcode, temperature, relhumidity;
        subscriber.recv(&update);

        std::istringstream iss(static_cast<char*>(update.data()));
        iss >> zipcode >> temperature >> relhumidity;

        total_temp += temperature;

        std::cout << "zipcode: " << zipcode << ". temperature: " << temperature << ". relhumidity: " << relhumidity << ".\n";
    }

    std::cout 	<< "Average temperature for zipcode '"<< filter
    			<<"' was "<<(int) (total_temp / update_nbr) <<"F"
    			<< std::endl;

                return 0;
}