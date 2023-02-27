#include <iostream>
#include <functional>
#include <string>
#include "chatserver.hpp"
#include "chatservice.hpp"
#include "json.hpp"
#include "db/db.hpp"
using namespace std::placeholders;
using namespace std;
using namespace nlohmann;

// 构造函数，初始化TcpServer对象
ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 设置回调
    _server.setConnectionCallback(bind(&ChatServer::onConnect, this, _1));
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));
    // 设置线程数
    _server.setThreadNum(4);
}

// 开启服务
void ChatServer::start()
{
    _server.start();
}

// 连接回调
void ChatServer::onConnect(const TcpConnectionPtr &conn)
{
    // 客户端断开连接
    if (!conn->connected())
    {
        // 客户端退出
        ChatService::getInstance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// 事件回调
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    // 完全解耦网络模块与业务模块代码 -> 抽象类或者回调
    try
    {
        /* code */
        string str = buffer->retrieveAllAsString();
        json js = json::parse(str); // 反序列化 得到json
        
        // js["msgid"]也只是json中定义的类型，需要转成相应认识的类型
        // 会根据id得到对应的业务处理函数(多态思想)，但所有业务代码不可见
        auto msgHandler = ChatService::getInstance()->getHandler(js["msgid"].get<int>());
        msgHandler(conn, js, time);
    }
    catch(const std::exception& e)
    {
        // 异常处理,json会抛出对应异常
        // 客户端可能会发出一些错误的json格式,导致json解析出现异常
        // 同时也处理了业务函数可能会出现的异常
        std::cerr << e.what() << '\n';
    }
}