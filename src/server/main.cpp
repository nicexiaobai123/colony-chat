#include "chatserver.hpp"
#include "chatservice.hpp"
#include "iostream"
#include <signal.h>
using namespace std;

// 处理服务器ctrl+c结束后,重置业务信息
void resetHandler(int)
{
    ChatService::getInstance()->reset();
    exit(0);
}
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6666" << endl;
        exit(-1);
    }
    // 命令解析
    string ip = argv[1];
    unsigned int port = atoi(argv[2]);

    // 信号捕捉函数 SIGINT ctrl+c
    signal(SIGINT, resetHandler);

    EventLoop loop;
    ChatServer svr(&loop, InetAddress(ip, port), "Chat");
    svr.start();
    loop.loop();
    return 0;
}