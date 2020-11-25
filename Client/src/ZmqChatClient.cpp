#include "ZmqChatClient.h"
#include <algorithm>

extern std::atomic<bool> g_quit;

CZmqChatClient::CZmqChatClient()
    :m_flIsRunning(false)
    ,m_sender(std::make_unique<CWorkerSender>(this))
    ,m_receiver(std::make_unique<CWorkerReceiver>(this))
{
    init();
    start();
}

CZmqChatClient::~CZmqChatClient()
{
    stop();
}

void CZmqChatClient::start()
{
    m_flIsRunning.store(true);

    /* start threads */
    m_sendThr    = std::thread(&CWorkerSender::run, m_sender.get());
    m_receiveThr = std::thread(&CWorkerReceiver::run, m_receiver.get());
}

void CZmqChatClient::stop()
{
    //if threads is running - stop loops and join threads
    if(m_flIsRunning.load())
    {
        m_flIsRunning.store(false);
        m_sendThr.join();
        m_receiveThr.join();
    }
}

void CZmqChatClient::init()
{
    std::cout << "**************************************************\n"
              << "                  ZmqClient\n                       "
              << "**************************************************\n";

    /*  get userName and check it */
    do{
        std::cout << "Enter your name: \n";
        std::getline(std::cin, m_name);
    } while(!checkName(m_name));

    /* get server ip and check it */
    do{
        std::cout << "\nEnter server ip address (no input for localhost): ";
        std::getline(std::cin, m_ipAddr);
    } while(!checkIp(m_ipAddr));

    /* get server port and check it */
    do{
        std::cout << "Enter server port (no input for 5555): ";
        std::getline(std::cin, m_sendPort);
    } while(!checkPort(m_sendPort));


    std::cout << "**************************************************\n";
}

void CZmqChatClient::startReceiving() const
{
    /* prepare context and socket */
    zmq::context_t context(4);
    zmq::socket_t socket(context, ZMQ_SUB);

    /* set socket options */
    socket.setsockopt(ZMQ_SUBSCRIBE, 0, 0);
    socket.setsockopt(ZMQ_RCVTIMEO, g_receiveTimeout); //set timeout

    /* try connect to server */
    print("Receiver: try connect to server [" + connectParamString(m_ipAddr, m_sendPort) + "]");
    try{
        //TODO: lock mtx
        socket.connect(connectParamString(m_ipAddr, m_sendPort));
    }
    catch( ... )
    {
        //fail to connect
        print("FAIL. Exit");
        g_quit.store(true); //exit
        return;
    }
    //successfully conected
    print("OK.");
    std::cout << "**************************************************\n";

    while(m_flIsRunning.load() && !g_quit.load())
    {
        receiveMessage(socket);
    }
}

void CZmqChatClient::startSending() const
{
    /* prepare context and socket */
    zmq::context_t context(4);
    zmq::socket_t socket(context, ZMQ_PUSH);

    /* try connect to server */
    print("Sender: try connect to server [" + connectParamString(m_ipAddr, m_recvPort) + "]");
    try{
        //TODO: lock mtx
        socket.connect(connectParamString(m_ipAddr, m_sendPort));
    }
    catch( ... )
    {
        //fail to connect
        print("FAIL. Exit");
        g_quit.store(true); //exit
        return;
    }
    //successfully conected
    print("OK.");
    std::cout << "**************************************************\n";

    std::string name{};
    {
        //mtx.lock
        name = m_name;
    }

    while(m_flIsRunning.load() && !g_quit.load())
    {
        sendMessage(socket, name);
    }
}

void CZmqChatClient::sendMessage(zmq::socket_t &_socket, const std::string &_name) const
{
    std::string inputStr;
    std::getline (std::cin, inputStr);

    //TODO: format inputStr
    inputStr = _name + ": " + inputStr;

    //TODO: log sending
    zmq::message_t message(inputStr.size());
    memcpy(message.data(), inputStr.data(), inputStr.size());

    try{
        _socket.send(message);
    }
    catch( ... )
    {
        print("sending error");
        return;
    }
}

void CZmqChatClient::receiveMessage(zmq::socket_t &_socket) const
{
    PLOGI << "Try to receive...";
    zmq::message_t rcvMessage;
    try {
        _socket.recv(&rcvMessage);
    }  catch ( ... ) {
        print("SOCKET RECV CATCHES EXPERSSION...");
        return;
    }
    print("rcv msg length", rcvMessage.size());

    std::string rcvMessage_str = std::string(static_cast<char*>(rcvMessage.data()),rcvMessage.size());
    if(!rcvMessage_str.empty())
    {
        //TODO: correct print
        std::cout << rcvMessage_str << '\n';
    }
}

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
        _ipaddr = "192.168.31.103";
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
    m_recvPort = std::to_string(std::stoi(_port) + 1);

    PLOGI << "sendPort <- " << _port;
    PLOGI << "recvPort <- " << m_recvPort;
    return true;
}

std::string CZmqChatClient::connectParamString(const std::string &_ipaddr, const std::string &_port)
{
    return ("tcp://" + _ipaddr + ":" + _port);
}

void CWorkerReceiver::run()
{
    m_client->startReceiving();
}

void CWorkerSender::run()
{
    m_client->startSending();
}
