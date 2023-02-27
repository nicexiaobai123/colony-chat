#include <iostream>
#include <muduo/net/TcpServer.h>
// #include <muduo/net/TcpClient.h> 给客户端使用
#include <muduo/net/EventLoop.h>
#include <functional>
#include <string>
using namespace std;
using namespace muduo::net;
using namespace muduo;
using namespace placeholders;

/*基于mudou网络库开发服务器程序
1.组合TcpServer对象
2.组合EventLoop事件循环对象的指针
3.明确TcpServer的构造函数需要什么参数，写出ChatServer的构造函数
4.在当前服务类的构造函数中，注册处理连接的回调函数和处理读写事件的回调函数
5.设置合适的服务端线程数量，muduo库会自己分配I/O线程和worker线程
*/
class ChatServer
{
public:
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg)
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 注册处理连接的回调
        _server.setConnectionCallback(bind(&ChatServer::onConnectio, this, _1));
        // 注册处理读写事件的回调
        _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));
        // 设置合适的服务端线程数量
        _server.setThreadNum(4);
    }
    void start() { _server.start(); }
private:
    void onConnectio(const TcpConnectionPtr &conn)
    {
        if (conn.get()->connected())
        {
            cout << "form:" << conn->peerAddress().toIpPort() << 
                "   to:" << conn->localAddress().toIpPort() << " On" << endl;
        }
        else
        {
            cout << "form:" << conn->peerAddress().toIpPort() << 
                "   to:" << conn->localAddress().toIpPort() << " Off" << endl;
            conn->shutdown();   // close(fd)
            //_loop->quit();
        }
    }
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
    {
        string str = buffer->retrieveAllAsString();
        cout << "recv data:" << str << "time:" << time.toFormattedString() << endl;
        conn->send(str);
    }
private:
    TcpServer _server;  //#1
    EventLoop *_loop;   //#2 epoll
};

#if 0
int main()
{
    EventLoop loop;
    ChatServer server(&loop,InetAddress("127.0.0.1",3333),"ChatServer");
    server.start();     // listenfd epoll_ctl=>epoll
    loop.loop();        // epoll_wait

    return 0;
}
#endif