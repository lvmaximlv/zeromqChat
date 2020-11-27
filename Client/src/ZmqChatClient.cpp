#include "ZmqChatClient.h"
#include <cassert>
#include <algorithm>

using namespace ZmqChatClient;


/**
 * @brief CZmqChatClient constructor
 */
CZmqChatClient::CZmqChatClient()
    : m_flIsRunning(false)
	, m_localQuitFlag(false)
	, m_externalQuitFlag(m_localQuitFlag)
{
}

/**
 * @brief CZmqChatClient constructor
 * @param[in] _extQuitFlag external quit flag. Setting it to 'true' will end send and recv loops
 */
CZmqChatClient::CZmqChatClient(std::atomic<bool> &_extQuitFlag)
    : m_flIsRunning(false)
	, m_localQuitFlag(false)
	, m_externalQuitFlag(_extQuitFlag)
{

}

/**
 * @brief Destructor
 *
 * If client is running - stops client
 */
CZmqChatClient::~CZmqChatClient()
{
	stop();
}

inline std::string clear() { return "\r\x1B [OK"; }
inline void invite() {std::cout << clear() << std::flush << "You: " << std::flush; }


/**
 * @brief Starts client
 *
 * Init client parameters and starts send and recv loops
 */
void CZmqChatClient::start()
{
	// init client params
	init();

	//set isRunning flag
	m_flIsRunning = true;

	// start receive thread
	m_receiveThr = std::thread(&CZmqChatClient::startReceiving, this);


	startSending();

// 	zmq::context_t context(1);
// 	zmq::socket_t socket_push(context, ZMQ_PUSH);

// 	socket_push.setsockopt(ZMQ_SNDTIMEO, g_socketTimeout);

// 	{
// 		std::lock_guard<std::mutex> lock(m_mtx);
// 			socket_push.connect(formatConnectParam(m_ipAddr, m_serverRecvPort));
// 	}
// 	std::cout << "\n\n********************** Start Client *************************\n\n";

// 	while(!m_localQuitFlag.load() && !m_externalQuitFlag.load())
// 	{
// //		std::cout << "You: ";
// 		std::string inputStr;
// 		invite();
// 		std::getline(std::cin, inputStr);

// 		if(!inputStr.empty())
// 		{
// 			inputStr = m_name + inputStr;

// 			zmq::message_t message(inputStr.size());
// 			memcpy(message.data(), inputStr.data(), inputStr.size());

// 			try {
// 				socket_push.send(message);
// 				std::cout << "sended...\n";
// 			} catch ( ... ) {
// 				std::cout << "send error! exit";
// 				return;
// 			}
// 		}
// 	}

	std::cout << "\n\n********************** END CLIENT *************************\n\n";
}

/**
 * @brief Stops client
 * 
 * Stops send and recv loops, join threads
 */
void CZmqChatClient::stop()
{
    std::cout << "Stopping client...";
    if(m_flIsRunning)
    {
        print ("Stopping threads...");
        if(m_receiveThr.joinable())
            m_receiveThr.join();
        print("receiver joined");

        if(m_sendThr.joinable())
            m_sendThr.join();
	    print("sender joined");
	    print("Ok.\n");
    }
    std::cout << "true";
}

/**
 * @brief init client
 * 
 * Asks user for unput parameters: username, server ip address
 *  and server receive port.
 */
void CZmqChatClient::init()
{
	std::cout << "**************************************************\n"
			  << "                  ZmqClient\n"
			  << "**************************************************\n";

	/*  get userName and check it */
	do{
		std::cout << "\nEnter your name:";
		std::getline(std::cin, m_name);
	} while(!checkName(m_name));

	/* get server ip and check it */
	do{
		std::cout << "Enter server ip address (no input for localhost): ";
		std::getline(std::cin, m_ipAddr);
	} while(!checkIp(m_ipAddr));

	/* get server port and check it */
	do{
		std::cout << "Enter server port (no input for 5555): ";
		std::getline(std::cin, m_serverRecvPort);
	} while(!checkPort(m_serverRecvPort));


	std::cout << "**************************************************\n";
}

/**
 * @brief Init receiving
 * 
 * Create context and receiving socket,
 * sets socket options and connects to server.
 * Starts receiving loop
 */
void CZmqChatClient::startReceiving() const
{
	std::cout << "+++++++++++ START RECEIVING ++++++++++\n";
	/* prepare context and socket */
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_SUB);

	/* set socket options */
	const std::string filter = "_from_server_ ";
	socket.setsockopt(ZMQ_RCVTIMEO, g_socketTimeout);
	socket.setsockopt(ZMQ_SUBSCRIBE, filter.data(), filter.size());

	/* try connect to server */
	print("Receiver: try connect to server [" + formatConnectParam(m_ipAddr, m_serverSendPort) + "]");
	try{
		std::lock_guard<std::mutex> lock(m_mtx);
		socket.connect(formatConnectParam(m_ipAddr, m_serverSendPort));
	}
	catch( ... )
	{
		//fail to connect
		print("FAIL. Exit");
		m_localQuitFlag.store(true);//exit
		return;
	}
	//successfully conected
	print("OK.");

	while(!m_localQuitFlag.load() && !m_externalQuitFlag.load())
	{
		receiveMessage(socket);
	}

	std::cout << "++++++++++ END RECEIVING ++++++++++++\n";
}


/**
 * @brief Init sending
 * 
 * Create context and sending socket,
 * sets socket options and connects to server.
 * Starts sending loop
 */
void CZmqChatClient::startSending() const
{
	/* prepare context and socket */
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_PUSH);
	socket.setsockopt(ZMQ_SNDTIMEO, g_socketTimeout); //set timeout


	/* try connect to server */
	print("Sender: try connect to server [" + formatConnectParam(m_ipAddr, m_serverSendPort) + "]");
	std::string name{};
	try{
		name = m_name;
		std::lock_guard<std::mutex> lock(m_mtx);
		socket.connect(formatConnectParam(m_ipAddr, m_serverRecvPort));
	}
	catch( ... )
	{
		//fail to connect
		print("FAIL. Exit");
		m_localQuitFlag.store(true);
		return;
	}
	//successfully conected
	print("OK.");
	std::cout << "**************************************************\n";

	while(!m_localQuitFlag.load() && !m_externalQuitFlag.load())
	{
		sendMessage(socket, name);
	}

	PLOGI << "exit start sending";
}

/**
 * @brief Wait for user message and send in to the server
 * 
 * Gets user message from console, formats it to the package and sends to server
 * 
 * @param[in] _socket  socket to send the message 
 * @param[in] _name username
 */
void CZmqChatClient::sendMessage(zmq::socket_t &_socket, const std::string &_name) const
{
	std::string inputStr;	
	invite();
	std::getline(std::cin, inputStr);

	if(!inputStr.empty())
	{
		inputStr = _name + ": " + inputStr;

		zmq::message_t message(inputStr.size());
		memcpy(message.data(), inputStr.data(), inputStr.size());

		try {
			_socket.send(message);
			std::cout << "sended...\n";
		} catch ( ... ) {
			std::cout << "send error! exit";
			return;
		}
	}
}

/**
 * @brief output received messages
 * 
 * Checks ZMQ input queue for messages, and output them to the console
 * 
 * @param[in] _socket socket for receiving messages 
 */
void CZmqChatClient::receiveMessage(zmq::socket_t &_socket) const
{
	bool rc;
	do {
		zmq::message_t message;
		if((rc = _socket.recv(&message, ZMQ_DONTWAIT)) == true)
		{
			std::string rcvMessage_str = std::string(static_cast<char*>(message.data()), message.size());
			PLOGI << "RECEIVED MESSAGE " << rcvMessage_str;
			std::cout << clear() << std::flush
					  << "received: " << rcvMessage_str
					  << std::endl << std::flush;
		}
	} while(rc == true);
	usleep(1);
}

/**
 * @brief Check user name for validity
 * 
 * @param[in, out] _name [in]  - received from user username
 *                       [out] - formatted username  
 * @return true if @see _name is formatted to valid state
 * @return false if @see _name is
 */
bool CZmqChatClient::checkName(std::string &_name) const
{
	if(_name.empty())
		_name = "User";

	PLOGI << "user name <- " << _name;
	return true;
}

bool CZmqChatClient::checkIp(std::string &_ipaddr) const
{
	if(_ipaddr.empty())
		_ipaddr = "localhost";
	//_ipaddr = "*";

	PLOGI << "server ip <- " << _ipaddr;
	return true;
}

bool CZmqChatClient::checkPort(std::string &_port) const
{
	if(_port.empty())
		_port = "5555";

	/* set receive port
	 * recv port = send port + 1 */
	m_serverSendPort = std::to_string(std::stoi(_port) + 1);

	PLOGI << "sendPort <- " << _port;
	PLOGI << "recvPort <- " << m_serverSendPort;
	return true;
}

std::string CZmqChatClient::formatConnectParam(const std::string &_ipaddr, const std::string &_port)
{
	return ("tcp://" + _ipaddr + ":" + _port);
}
