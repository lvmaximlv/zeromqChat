#ifndef ZMQCHATCLIENT_H
#define ZMQCHATCLIENT_H
#include <thread>
#include <mutex>

#include <zmq.hpp>

#include "global.h"

static const int g_receiveTimeout = 10; //timeout for waiting message in ms

class CWorkerReceiver;
class CWorkerSender;

class CZmqChatClient
{
public:
    CZmqChatClient();
    ~CZmqChatClient();

    void start();
    void stop();

    friend class CWorkerReceiver;
    friend class CWorkerSender;
private:
    void init();

    void startReceiving() const;
    void startSending() const;

    void sendMessage(zmq::socket_t &_socket, const std::string &_name) const;
    void receiveMessage(zmq::socket_t &_socket)const;

    bool checkName(std::string &_name) const;
    bool checkIp(std::string &_ipaddr) const;
    bool checkPort(std::string &_port) const;

    static std::string connectParamString(const std::string& _ipaddr, const std::string &_port);

private:
    std::string m_name;
    std::string m_ipAddr;
    std::string m_sendPort;
    mutable std::string m_recvPort;

    std::atomic<bool> m_flIsRunning;

    std::unique_ptr<CWorkerSender> m_sender;
    std::unique_ptr<CWorkerReceiver> m_receiver;

    std::thread m_receiveThr;
    std::thread m_sendThr;

    mutable std::mutex m_mtx;
};

/* Receive worker. Used for receiving messages in other thread */
class CWorkerReceiver
{
public:
    CWorkerReceiver(CZmqChatClient *_client) : m_client(_client) {}

    void run();

private:
    CZmqChatClient *m_client;
};

/* Send worker. Used for sending messages in other thread */
class CWorkerSender
{
public:
    CWorkerSender(CZmqChatClient *_client) : m_client(_client) {}

    void run();

private:
    CZmqChatClient *m_client;
};

#endif // ZMQCHATCLIENT_H