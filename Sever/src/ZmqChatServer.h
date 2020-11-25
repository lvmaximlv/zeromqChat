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

static const uint16_t g_standartRecvPort = 5555; // standart port for receiving messages.
static const int g_receiveTimeout = 1000; //timeout for waiting message in ms


class CWorkerReceiver;
class CWorkerSender;

class CZmqChatServer
{
public: 
	CZmqChatServer();
	CZmqChatServer(const uint16_t _port);
	~CZmqChatServer();

	void start();
	void stop();

	friend class CWorkerSender;
	friend class CWorkerReceiver;

private:
	void startSending() const;
	void startReceiving() const;

	void sendMsg(zmq::socket_t &_socket) const;
	void recvMsg(zmq::socket_t &_socket) const;

	static std::string getConnectParamStr(const uint16_t &_port);

private:
	uint16_t m_recvPort;	//port for receiving messages
	uint16_t m_sendPort;	//port for sending messages

	std::thread m_receiveThr;	//thread for receiver worker
	std::thread m_sendThr;		//thread for sender worker

	std::unique_ptr<CWorkerSender> m_sender;		//worker for send messages
	std::unique_ptr<CWorkerReceiver> m_receiver;	//worker for receiving messages

	std::atomic<bool> m_flIsWorking; //flag is used to run loops in recieve() and send() functions

	mutable std::mutex m_mtx;	//mtx for shared data
	mutable std::queue<std::string> m_receivedMessages;	//received messages queue. receive() push received messages, send() pop messages for sending
};

//Recieve worker. use CZmqServer::receive()
class CWorkerReceiver
{
public:
	CWorkerReceiver(CZmqChatServer *_server) : m_server(_server) {}
	void run();
private:
	CZmqChatServer *m_server;
};

//Send worker. use CZmqServer::send()
class CWorkerSender
{
public:
	CWorkerSender(CZmqChatServer *_server) : m_server(_server) {}
	void run();

private:
	CZmqChatServer *m_server;
};

#endif // ZMQCHATSERVER_H
