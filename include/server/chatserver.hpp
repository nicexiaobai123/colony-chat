#pragma once
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;
// 聊天服务器类
class ChatServer
{
public:
    // 构造函数,初始化TcpServer对象
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg);
    // 开启服务
    void start();

private:
    // 连接回调
    void onConnect(const TcpConnectionPtr &);
    // 事件回调
    void onMessage(const TcpConnectionPtr &, Buffer *, Timestamp);

private:
    TcpServer _server;
    EventLoop *_loop;
};