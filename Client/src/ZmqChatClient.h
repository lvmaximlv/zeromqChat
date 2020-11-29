#ifndef ZMQCHATCLIENT_H
#define ZMQCHATCLIENT_H

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <regex>

#include <zmq.hpp>

#include <plog/Log.h>

namespace ZmqChatClient
{
// timeout for waiting/sending message in ms
static const int g_socketTimeout = 10;


/**
 * @brief The CZmqChatClient class
 *
 * CZmqChat client is realisation of the simple zmq chat client.
 *
 * It uses ZMQ_PUB socket to send data to the server
 * and ZMQ_SUB socket to receive data.
 *
 * Sending and receiving ports are two adjacent ports.
 *
 * to start client use start()
 */
class CZmqChatClient
{
public:
	//Constructors:
	CZmqChatClient();
	CZmqChatClient(std::atomic<bool> &_extQuitFlag);

	//Destructor
	~CZmqChatClient();

	void start();								///< start receiving thread and send loop
	void stop();								///< stop receiving and sending

private:
	void init();								///< init client, get user name, ip address and port

	void startReceiving() const;				///< setup zmq environment for receiving and start recv loop
	void startSending() const;					///< setup zmq environment for sending and start send loop

	void sendMessage(zmq::socket_t &_socket, const std::string &_name) const;	///< send message with given socket
	void receiveMessage(zmq::socket_t &_socket)const;	///< receive and print message from given socket

	bool checkName(std::string &_name) const;	///< check: given name is valid for user name
	bool checkIp(std::string &_ipaddr) const;	///< check: given ip is valid
	bool checkPort(std::string &_port) const;	///< check: given port is valid

	///< format connect param string from address and port
	static std::string formatConnectParam(const std::string& _ipaddr, const std::string &_port);

private:
	std::string m_name;							///< user name
	std::string m_ipAddr;						///< server ip address
	std::string m_serverRecvPort;				///< server receiving port
	mutable std::string m_serverSendPort;		///< server sending port

    bool m_flIsRunning;                         ///< flag: is running
	mutable std::atomic<bool> m_localQuitFlag;  ///< local quit flag
	const std::atomic<bool> &m_externalQuitFlag;///< external quit flag

	std::thread m_receiveThr;					///< receive thread
	std::thread m_sendThr;						///< send thread

	mutable std::mutex m_mtx;					///< mutex for shared data

}; //class CZmqChatClient


/**
 * @brief CChatMessage struct
 * 
 * 	simple wrap zmq::message_t
 */
struct CChatMessage : public zmq::message_t
{
	std::string m_filter;		///< server filter
	std::string m_username;		///< username
	std::string m_message;		///< message

	CChatMessage() = default;
	CChatMessage(const std::string &_name, const std::string &_message);	///< create message from data

	typedef std::tuple<std::string, std::string, std::string> messageDataT;
	messageDataT getData();		///<rceive data from message
};

} // namespace ZmqChatClient

#endif // ZMQCHATCLIENT_H
