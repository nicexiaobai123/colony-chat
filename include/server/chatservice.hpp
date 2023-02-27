#pragma once
#include <functional>
#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include <mutex>
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemsgmodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
using namespace std;
using namespace nlohmann;
using namespace muduo;
using namespace muduo::net;

// 回调函数类型
using handler = function<void(const TcpConnectionPtr &, json&, Timestamp)>;

// 服务器业务类 单例
class ChatService
{
public:
    // 单例接口
    static ChatService *getInstance();
    // 提供外部接口,获取消息id对应的处理器
    handler getHandler(int);
    // 客户端异常退出
    void clientCloseException(const TcpConnectionPtr &);
    // 业务重置方法
    void reset();
private:
    ChatService();
    ChatService(const ChatService &) = delete;
    ChatService &operator=(const ChatService &) = delete;
    // 用户登录
    void login(const TcpConnectionPtr &, json&, Timestamp);
    // 用户注册
    void reg(const TcpConnectionPtr &, json&, Timestamp);
    // 点对点聊天
    void oneChat(const TcpConnectionPtr &, json&, Timestamp);
    // 添加好友
    void addFriend(const TcpConnectionPtr &, json&, Timestamp);
    // 创建群组
    void createGroup(const TcpConnectionPtr &, json&, Timestamp);
    // 添加群组
    void addGroup(const TcpConnectionPtr &, json&, Timestamp);
    // 群发消息
    void groupChat(const TcpConnectionPtr &, json&, Timestamp);
    // 注销登录
    void loginOut(const TcpConnectionPtr &, json&, Timestamp);
    // redis处理通道消息
    void redisNotifyHandler(int, string);
private:
    // 存储消息id:业务处理方法
    unordered_map<int, handler> _msgHandlerMap;
    // 存储用户连接信息,以便服务器向客户端发送信息和一些判断
    unordered_map<int,TcpConnectionPtr> _userConnMap;
    // 定义互斥,保证_userConnMap线程安全
    mutex _connMapMtx;

    // 数据操作类对象
    UserModel _usermodel;
    OfflineMsgModel _offlineModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
    
    // redis操作对象
    Redis redis;
};