#include "ZmqChatClient.h"
#include <cassert>
#include <algorithm>

using namespace ZmqChatClient;


/**
 * @brief CZmqChatClient constructor
 */
CZmqChatClient::CZmqChatClient()
    : m_serverRecvPort("5555")
	, m_serverSendPort("5556")
	, m_flIsRunning(false)
	, m_localQuitFlag(false)
	, m_externalQuitFlag(m_localQuitFlag)
{
	PLOGI << "create CZmqChatClient object";
}

/**
 * @brief CZmqChatClient constructor
 * @param[in] _extQuitFlag external quit flag. Setting it to 'true' will end send and recv loops
 */
CZmqChatClient::CZmqChatClient(std::atomic<bool> &_extQuitFlag)
    : m_serverRecvPort("5555")
	, m_serverSendPort("5556")
	, m_flIsRunning(false)
	, m_localQuitFlag(false)
	, m_externalQuitFlag(_extQuitFlag)
{
	PLOGI << "create CZmqChatClient object";
}

/**
 * @brief Destructor
 *
 * If client is running - stops client
 */
CZmqChatClient::~CZmqChatClient()
{
	PLOGI << "destroy CZmqChatClient object";
	stop();
}

/**
 * @brief Starts client
 *
 * Init client parameters and starts send and recv loops
 */
void CZmqChatClient::start()
{
	PLOGI << "start client..."; 

	// init client params
	PLOGI << "init client params";
	init();

	//set isRunning flag
	m_flIsRunning = true;

	// start receive thread
	PLOGI << "start receiving thread";
	m_receiveThr = std::thread(&CZmqChatClient::startReceiving, this);

	//start sending
	PLOGI << "start sending";
	startSending();
}

/**
 * @brief Stops client
 * 
 * Stops send and recv loops, join threads
 */
void CZmqChatClient::stop()
{
    PLOGI << "Stopping client...";
    if(m_flIsRunning)
    {
        PLOGI << "Stopping threads...";
        if(m_receiveThr.joinable()) 
		{ 
			m_receiveThr.join(); 
        	PLOGI << "receiver joined";
		}

        if(m_sendThr.joinable()) 
		{ 
			m_sendThr.join(); 
	    	PLOGI << "sender joined";
		}
	    PLOGI << "Ok.";
    }
    std::cout << "Bye!\n";
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
		std::cout << "\nEnter your name (leave empty for 'User'): ";
		std::getline(std::cin, m_name);
	} while(!checkName(m_name) && !m_externalQuitFlag.load());

	/* get server ip and check it */
	do{
		std::cout << "Enter server ip address (leave empty for 'localhost'): ";
		std::getline(std::cin, m_ipAddr);
	} while(!checkIp(m_ipAddr) && !m_externalQuitFlag.load());

	// port chosing on server is not implemented yet...

	// /* get server port and check it */
	// do{
	// 	std::cout << "Enter server port (leave empty for 5555): ";
	// 	std::getline(std::cin, m_serverRecvPort);
	// } while(!checkPort(m_serverRecvPort) && !m_externalQuitFlag.load());


	std::cout << "**************************************************\n\n";
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
	/* prepare context and socket */
	PLOGI << "[receiving] prepare context and socket";
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_SUB);

	/* set socket options */
	PLOGI << "[receiving] set socket options";
	socket.setsockopt(ZMQ_RCVTIMEO, g_socketTimeout);
	socket.setsockopt(ZMQ_SUBSCRIBE, g_serverFilter.data(), g_serverFilter.size());

	/* try connect to server */
	PLOGI << "[receiving] try to connect to server...";
	try{
		std::lock_guard<std::mutex> lock(m_mtx);
		socket.connect(formatConnectParam(m_ipAddr, m_serverSendPort));
	}
	catch( ... )
	{
		//fail to connect
		PLOGI << "...Fail.";
		m_localQuitFlag.store(true);//exit
		return;
	}
	PLOGI << "...Ok.";

	//start receiving loop
	while(!m_localQuitFlag.load() && !m_externalQuitFlag.load())
	{
		receiveMessage(socket);
	}
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
	PLOGI << "[sending] prepare context and socket";
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_PUSH);

	PLOGI << "[sending] set socket options";
	socket.setsockopt(ZMQ_SNDTIMEO, g_socketTimeout); //set timeout


	/* try connect to server */
	PLOGI << "[sending] trye to connect to server...";
	try{
		std::lock_guard<std::mutex> lock(m_mtx);
		socket.connect(formatConnectParam(m_ipAddr, m_serverRecvPort));
	}
	catch( ... )
	{
		PLOGI << "...Fail.";
		m_localQuitFlag.store(true);
		return;
	}
	PLOGI << "...Ok";

	std::string name{};	//username for sending
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		name = m_name;
	}

	// start send loop
	while(!m_localQuitFlag.load() && !m_externalQuitFlag.load())
	{
		sendMessage(socket, name);
	}
}

/**
 * @brief get input from std::getline and clear line
 * 
 * @return std::string user input
 */
inline std::string getUserInput()
{
	std::string inputStr;	
	std::getline(std::cin, inputStr);

	// delete cin line.				// '\033[A' moves cursor up one line
	std::cout << "\033[A\33[2K";	// '33[2K' erases the entire line cursor is currently on	

	return inputStr;
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
	//get message from user
	std::string inputStr = getUserInput();	

	if(!inputStr.empty())
	{
		PLOGI << "sending: " << inputStr;
		//create message
		CChatMessage message(_name, inputStr);
		// send message to server
		try {
			_socket.send(message);
		} catch ( ... ) {
			PLOGI << "send: fail";
			return;
		}
		PLOGI << "send: ok";
	}
}

/**
 * @brief print CChat message to console
 * 
 * @param[in] _message message to print
 */
inline void displayMessage (CChatMessage &_message)
{
	//get data from message
	auto [filter, username, message] = _message.getData();

	auto br= [](const std::string &inString) { return "[" + inString + "]"; }; // add brackets to string

	PLOGI << "received: " << br(filter) << br(username) << br(message);

	std::cout << '\r' <<  br(username) << " > " << message << '\n';
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
		CChatMessage message;
		if((rc = _socket.recv(&message, ZMQ_DONTWAIT)) == true)
		{
			displayMessage(message);
		}
	} while(rc == true);
	usleep(1);
}



/**
 * @brief Check user name for validity
 * 
 * @param[in, out] _name [in]  - received from user username
 *                       [out] - formatted username  
 * @return true if input name is valid
 * @return false elsewise
 */
bool CZmqChatClient::checkName(std::string &_name) const
{
	if(_name.empty())
		_name = "User";
	if(_name.size() > 25)
	{
		std::cout << "name must be at least 1 and at max 25 characters long\n";
		return false;
	}

	PLOGI << "user name <- " << _name;
	return true;
}

/**
 * @brief check input ip string for validity
 * 
 * @param[in, out] _ipaddr input ip address string 
 * @return true if input is valid
 * @return false elsewise
 */
bool CZmqChatClient::checkIp(std::string &_ipaddr) const
{
	if(_ipaddr.empty())
	{
		_ipaddr = "localhost";
	}
	else
	{
		std::string  ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
		std::regex ipRegex ("^" + ipRange
						+ "\\." + ipRange
						+ "\\." + ipRange
						+ "\\." + ipRange + "$");
		if(!std::regex_match(_ipaddr,ipRegex))
		{
			std::cout << "invalid ip address.\n";
			return false;
		}
	}

	PLOGI << "server ip <- " << _ipaddr;
	return true;
}

/**
 * @brief check input port for validity ans set
 * 
 * @param[in, out] _port input port 
 * @return true if port is valid
 * @return false elsewise
 */
bool CZmqChatClient::checkPort(std::string &_port) const
{
	if(_port.empty())
		_port = "5555";

	auto portNum = std::stoi(_port);
	if(portNum < 1 || portNum > 65536)
	{
		std::cout << "port must be at least 1 and at max 65535\n";
	}

	/* set receive port
	 * recv port = send port + 1 */
	m_serverSendPort = std::to_string(std::stoi(_port) + 1);

	PLOGI << "server receive port <- " << _port;
	PLOGI << "server send port <- " << m_serverSendPort;
	return true;
}

/**
 * @brief form a connection parametr string
 * 
 * @param[in] _ipaddr server ip address 
 * @param[in] _port server port
 * @return std::string formatted param string
 */
std::string CZmqChatClient::formatConnectParam(const std::string &_ipaddr, const std::string &_port)
{
	return ("tcp://" + _ipaddr + ":" + _port);
}

/**
 * @brief Construct a new CChatMessage::CChatMessage object from username and message
 * 
 * @param[in] _name username 
 * @param[in] _message message
 */
CChatMessage::CChatMessage(const std::string &_name, const std::string &_message)
{
	std::string message = _name + " " + _message;
	this->rebuild(message.size());
	memcpy(this->data(), message.data(), message.size());
}

/**
 * @brief get data from zmq::message_t
 * 
 * @return CChatMessage::messageDataT tuple od data: filter, username and message
 */
CChatMessage::messageDataT CChatMessage::getData()
{
	std::stringstream dataStream( static_cast<char*>(this->data()) );	// convert zmq::message_t to stringstream
	dataStream >> m_filter >> m_username;	 	// get filter and username from sstream
	dataStream.get();							// remove space
	std::getline(dataStream, m_message);		//read all left data from sstream
	return {m_filter, m_username, m_message};	
}