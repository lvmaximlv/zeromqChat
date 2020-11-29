#ifndef ZMQCHATSERVER_H
#define ZMQCHATSERVER_H

#include <iostream>
#include <string>
#include <unistd.h>
#include <queue>
#include <thread>
#include <mutex>

#include <zmq.hpp>

#include "global.h"

namespace ZmqChatServer
{
static const uint16_t g_standartRecvPort = 5555; // standart port for receiving messages.
static const int g_receiveTimeout = 1000; //timeout for waiting message in ms
static const std::string g_serverFilter = "_from_server_"; // filter for clients that uses sub socket

/**
 * @brief CZmqChatServer is simple chat server using zeromq
 * 
 * It uses zmq pull socket to receivin messages and zmq pub socket to send them to clients
 */
class CZmqChatServer
{
public: 
	CZmqChatServer();						///< Constructor
	CZmqChatServer(const uint16_t _port);	///< Constructor from port
	~CZmqChatServer();						///< Destructor

	void start();							///< start server
	void stop();							///< stop server

private:
	void startSending() const;				///< start sending messages
	void startReceiving() const;			/// start receiving messages

	void sendMsg(zmq::socket_t &_socket) const;	///< send message
	void recvMsg(zmq::socket_t &_socket) const;	///< receive message

	static std::string getConnectParamStr(const uint16_t &_port);	///< for a connection param string

private:
	uint16_t m_recvPort;					///< port for receiving messages
	uint16_t m_sendPort;					///< port for sending messages

	std::thread m_receiveThr;				///< thread for receiver worker
	std::thread m_sendThr;					///< thread for sender worker

	std::atomic<bool> m_flIsWorking; 		///< flag is used to run loops in recieve() and send() functions

	mutable std::mutex m_mtx;				///< mtx for shared data
	mutable std::queue<std::string> m_receivedMessages;	///< received messages queue.
};

} //namespace ZmqChatServer

#endif // ZMQCHATSERVER_H
