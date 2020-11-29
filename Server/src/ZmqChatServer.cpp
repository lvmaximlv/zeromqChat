#include "ZmqChatServer.h"

extern std::atomic<bool> g_quit;

using namespace ZmqChatServer;

/**
 * @brief Construct a new CZmqChatServer::CZmqChatServer object
 * 
 */
CZmqChatServer::CZmqChatServer()
	:CZmqChatServer(g_standartRecvPort)
{

}

/**
 * @brief Construct a new CZmqChatServer::CZmqChatServer object from port
 * 
 * @param _port port for receiving 
 */
CZmqChatServer::CZmqChatServer(const uint16_t _port)
	: m_recvPort(_port)
	, m_sendPort(_port + 1)
	, m_flIsWorking(false)
{

}

/**
 * @brief Destroy the CZmqChatServer::CZmqChatServer object
 * 
 * stops server
 */
CZmqChatServer::~CZmqChatServer()
{
	this->stop(); //stop threads
}

/**
 * @brief starts server
 * 
 */
void CZmqChatServer::start()
{
	m_flIsWorking.store(true); //set working flag

	// start threads 
	m_receiveThr = std::thread(&CZmqChatServer::startReceiving, this);

	// start sending loop
	startSending();
}

/**
 * @brief stops server
 * 
 * join threads
 */
void CZmqChatServer::stop()
{
	std::cout << "Stopping threads...";
	//if threads is running - stop loops and join threads
	if(m_flIsWorking.load())
	{
		m_flIsWorking.store(false);
		if(m_sendThr.joinable())
		{
			m_sendThr.join();
		}
		if(m_receiveThr.joinable())
		{
			m_receiveThr.join();
		}
	}
	std::cout << "Ok.\n";
}

/**
 * @brief starts sending
 * 
 * prepares context and socket,
 * sets socket options, bind socket and starts sending loop
 */
void CZmqChatServer::startSending() const
{
	/* prepare context and socket */
	print("prepare sending context and socket");
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_PUB);

	/* bind send socket to port */
	print("bind send socket to port :", m_sendPort);
	socket.bind(getConnectParamStr(m_sendPort));

	//start sending loop
	print("start sending messages...");
	while(!g_quit.load() && m_flIsWorking.load())
	{
		sendMsg(socket);
	}
}

/**
 * @brief starts receiving
 * 
 * prepare context and socket,
 * sets socket options and starts receiving loop
 */
void CZmqChatServer::startReceiving() const
{
	/* prepare context and socket */
	print("prepare recv context and socket");
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_PULL);

	/* bind receiving socket to port */
	print("bind receiving socket to port :", m_recvPort);
	socket.bind(getConnectParamStr(m_recvPort));
	/* set socket options */
	socket.setsockopt(ZMQ_RCVTIMEO, g_receiveTimeout); //set timeout


	//start receiving loop
	print("start receiving messages...");
	while(!g_quit.load() && m_flIsWorking.load())
	{
		recvMsg(socket);
	}
}

/**
 * @brief send message via given socket
 * 
 * @param[in] _socket socket to send message 
 */
void CZmqChatServer::sendMsg(zmq::socket_t &_socket) const
{
	if(std::lock_guard<std::mutex> lock{m_mtx}; !m_receivedMessages.empty())	//check messages queue
	{
		std::string &messageString = m_receivedMessages.front();  //get message to send

		print("[Send message]", messageString); //info about message

		messageString = g_serverFilter + " " + messageString; //add filter param

		/* prepare message */
		zmq::message_t broadcast_msg(messageString.size());
		memcpy(broadcast_msg.data(), messageString.data(), messageString.size());
		m_receivedMessages.pop();

		/* send message */
		_socket.send(broadcast_msg);
	}
}

/**
 * @brief receive message
 * 
 * @param[in] _socket socket to receive message
 */
void CZmqChatServer::recvMsg(zmq::socket_t &_socket) const
{
	bool rc;
	do {
		zmq::message_t recv_msg;
		if((rc = _socket.recv(&recv_msg, ZMQ_DONTWAIT)) == true) // check zmq recv queue for ready messages
		{
			std::string recv_str = std::string(static_cast<char*>(recv_msg.data()), recv_msg.size()); //convert message_t to string
			if(!recv_str.empty())
			{
				print("[Receive message]", recv_str);
				std::lock_guard<std::mutex> lock(m_mtx);
				m_receivedMessages.push(recv_str);	// push message to queue
			}
		}
	} while(rc == true);
	usleep(1);
} 

/**
 * @brief form a connection param string
 * 
 * @param[in] _port port  
 * @return std::string connection param string
 */
std::string CZmqChatServer::getConnectParamStr(const uint16_t &_port)
{
	return ("tcp://*:" + std::to_string(_port));
}

