#include "ZmqChatServer.h"

extern std::atomic<bool> g_quit;

CZmqChatServer::CZmqChatServer()
    :CZmqChatServer(g_standartRecvPort)
{}

CZmqChatServer::CZmqChatServer(const uint16_t _port)
    : m_recvPort(_port)
    , m_sendPort(_port + 1)
    , m_flIsWorking(false)
{
    // initialize workers
    m_sender = std::make_unique<CWorkerSender>(this);
    m_receiver = std::make_unique<CWorkerReceiver>(this);
}


CZmqChatServer::~CZmqChatServer()
{
    this->stop(); //stop threads
}


void CZmqChatServer::start()
{
    m_flIsWorking.store(true); //set working flag

    /* start threads */
    m_receiveThr = std::thread(&CWorkerReceiver::run, m_receiver.get());
    m_sendThr = std::thread(&CWorkerSender::run, m_sender.get());
}

void CZmqChatServer::stop()
{
    //if threads is running - stop loops and join threads
    if(m_flIsWorking.load())
    {
        m_flIsWorking.store(false);
        m_sendThr.join();
        m_receiveThr.join();
    }
}

void CZmqChatServer::startSending() const
{
    /* prepare context and socket */
    print("prepare sending context and socket");
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PUB);

    /* bind send socket to port */
    print("bind send socket to port :", m_sendPort);
    socket.bind(getConnectParamStr(m_sendPort));

    //work loop
    print("start sending messages...");
    while(!g_quit.load() && m_flIsWorking.load())
    {
        sendMsg(socket);
    }
}

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


    //work loop
    print("start receiving messages...");
    while(!g_quit.load() && m_flIsWorking.load())
    {
        recvMsg(socket);
    }
}

void CZmqChatServer::sendMsg(zmq::socket_t &_socket) const
{
    if(std::lock_guard<std::mutex> lock(m_mtx); !m_receivedMessages.empty())
    {
        const std::string &messageString = m_receivedMessages.front();  //get message to send

        print("[Send message]", messageString); //info about message

        /* prepare message */
        zmq::message_t broadcast_msg(messageString.size());
        memcpy(broadcast_msg.data(), messageString.data(), messageString.size());
        m_receivedMessages.pop();

        /* send message */
        _socket.send(broadcast_msg);
    }
}

void CZmqChatServer::recvMsg(zmq::socket_t &_socket) const
{
    zmq::message_t recv_msg;

    try{
    _socket.recv(&recv_msg);
    }
    catch(...)
    {
        print("Socket RECV catches expression...");
        return;
    }
    
    std::string recv_str = std::string(static_cast<char*>(recv_msg.data()), recv_msg.size());
    if(!recv_str.empty())
    {
        print("[Receive message]", recv_str);
        std::lock_guard<std::mutex> lock(m_mtx);
        m_receivedMessages.push(recv_str);
    }
} 


std::string CZmqChatServer::getConnectParamStr(const uint16_t &_port)
{
    return ("tcp://*:" + std::to_string(_port));
}




void CWorkerReceiver::run()
{
    m_server->startReceiving();
}

void CWorkerSender::run()
{
    m_server->startSending();
}
