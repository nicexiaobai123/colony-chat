#include "chatservice.hpp"
#include "public.hpp"
#include "muduo/base/Logging.h"
using namespace std::placeholders;
using namespace muduo;

// 从redis消息队列中获取订阅的消息
// ---------------------------------------------------------------------------------
void ChatService::redisNotifyHandler(int id, string msg)
{
    {
        lock_guard<mutex> lock(_connMapMtx);
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            it->second->send(msg);
            return;
        }
    }
    // 不在线,存储离线消息,在redis通知客户端前意外下线
    _offlineModel.insert(id, msg);
}
// ---------------------------------------------------------------------------------

// 构造函数
ChatService::ChatService()
{
    // 注册业务处理方法->注册回调
    _msgHandlerMap.insert({MsgType::LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({MsgType::REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({MsgType::LOGINOUT_MSG, std::bind(&ChatService::loginOut, this, _1, _2, _3)});
    _msgHandlerMap.insert({MsgType::ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({MsgType::CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({MsgType::ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});

    // 聊天业务
    _msgHandlerMap.insert({MsgType::ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({MsgType::GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if (redis.connect())
    {
        // 设置redis业务层回调
        redis.init_notify_handler(std::bind(&ChatService::redisNotifyHandler, this, _1, _2));
    }
}

// 单例接口
ChatService *ChatService::getInstance()
{
    static ChatService _service;
    return &_service;
}

// 提供外部接口,获取消息id对应的处理器
handler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 接受到错误的消息id,提供一个空处理handler
        return [msgid](const TcpConnectionPtr &, json &, Timestamp)
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler";
        };
    }
    return it->second;
}

// 用户登录
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string password = js["password"];
    User user = _usermodel.query(id);
    if (user.getPwd() == password && user.getId() == id)
    {
        json response;
        if (user.getState() == "offline")
        {
            // 登录成功
            {
                // 存储在线连接信息-线程安全
                lock_guard<mutex> lock(this->_connMapMtx);
                _userConnMap.insert({id, conn});
            }

            // 更新用户状态
            user.setState("online");
            _usermodel.updateState(user);

            // redis-订阅登录的id到消息队列
            redis.subscribe(id);

            // 处理用户离线信息
            vector<string> msgVec = _offlineModel.query(id);
            if (!msgVec.empty())
            {
                response["offlinemsg"] = msgVec;
                // 离线消息取了就删除
                _offlineModel.remove(id);
            }

            // 处理好友列表信息
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                vector<string> usersV;
                for (User &user : userVec)
                {
                    json userJs;
                    userJs["friendid"] = user.getId();
                    userJs["name"] = user.getName();
                    userJs["state"] = user.getState();
                    usersV.push_back(userJs.dump());
                }
                response["friends"] = usersV;
            }

            // 处理群组信息
            vector<Group> groupVec = _groupModel.queryGroups(id);
            if (!groupVec.empty())
            {
                vector<string> groupV;
                for (Group &group : groupVec)
                {
                    // 组信息
                    json gruopJs;
                    gruopJs["id"] = group.getId();
                    gruopJs["groupname"] = group.getName();
                    gruopJs["groupdesc"] = group.getDesc();
                    vector<string> usersV;
                    for (const GroupUser &user : group.getUsers())
                    {
                        // 组员信息
                        json usersJs;
                        usersJs["userid"] = user.getId();
                        usersJs["name"] = user.getName();
                        usersJs["state"] = user.getState();
                        usersJs["role"] = user.getRole();
                        usersV.push_back(usersJs.dump());
                    }
                    gruopJs["users"] = usersV;
                    groupV.push_back(gruopJs.dump());
                }
                response["groups"] = groupV;
            }

            // 登录成功信息回复
            response["msgid"] = MsgType::LOGIN_MSG_ACK;
            response["errno"] = Errno::SUCCESS;
            response["id"] = user.getId();
            response["name"] = user.getName();
            conn->send(response.dump());
        }
        else
        {
            // 登录失败,重复登录
            response["msgid"] = MsgType::LOGIN_MSG_ACK;
            response["errno"] = Errno::FAIL_MSG;
            response["errmsg"] = "The account has been logged in, please do not log in again";
            conn->send(response.dump());
        }
    }
    else
    {
        // 登录失败,账号或者密码错误
        json response;
        response["msgid"] = MsgType::LOGIN_MSG_ACK;
        response["errno"] = Errno::FAIL_MSG;
        response["errmsg"] = "Login failed,account or password error";
        conn->send(response.dump());
    }
}

// 用户注册
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string password = js["password"];

    User user;
    user.setName(name);
    user.setPwd(password);
    if (_usermodel.insert(user)) // insert会改变User的id
    {
        // 用户注册成功 -- 返回成功码,客户端根据“码”判断成功失败与否
        json response;
        response["msgid"] = MsgType::REG_MSG_ACK;
        response["errno"] = Errno::SUCCESS;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // 用户注册失败 -- 返回失败码
        json response;
        response["msgid"] = MsgType::REG_MSG_ACK;
        response["errno"] = Errno::FAIL;
        conn->send(response.dump());
    }
}

// 注销登录
void ChatService::loginOut(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int id = js["userid"].get<int>();
    {
        // 移除连接信息
        lock_guard<mutex> lock(this->_connMapMtx);
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 更新用户状态
    User user(id, "", "", "offline");
    _usermodel.updateState(user);

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    redis.unsubscribe(user.getId());
}

// 客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    // 移除_userConnMap连接信息,线程安全
    int id;
    {
        lock_guard<mutex> lock(this->_connMapMtx);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if (it->second == conn)
            {
                id = it->first;
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // 更新用户状态
    User user;
    user.setId(id);
    user.setState("offline");
    _usermodel.updateState(user);

    // redis 取消订阅
    redis.unsubscribe(user.getId());
}

// 点对点聊天
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(this->_connMapMtx);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // 在线,直接转发
            it->second->send(js.dump());
            return;
        }
    }

    // 看是否状态是online,是则需要跨服务通信,在redis的消息队列中publish
    if (_usermodel.query(toid).getState() == "online")
    {
        redis.publish(toid, js.dump());
        return;
    }

    // 不在线,存储离线消息
    _offlineModel.insert(toid, js.dump());
}

// 业务重置方法
void ChatService::reset()
{
    // 服务器退出,目前重置的只有这个状态
    _usermodel.resetState();
}

// 添加好友
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["userid"].get<int>();
    int friendid = js["friendid"].get<int>();
    _friendModel.insert(userid, friendid);
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["creatorid"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        // 修改创建者
        _groupModel.addGruop(userid, group.getId(), GroupModel::Creator);
        // 创建成功
        json response;
        response["groupid"] = group.getId();
        response["msgid"] = MsgType::CREATE_GROUP_MSG_ACK;
        response["errno"] = Errno::SUCCESS;
        conn->send(response.dump());
    }
    else
    {
        // 创建群组失败 -- 返回失败码
        json response;
        response["msgid"] = MsgType::CREATE_GROUP_MSG_ACK;
        response["errno"] = Errno::FAIL_MSG;
        response["errmsg"] = "create group fail,already exists!";
        conn->send(response.dump());
    }
}

// 添加群组
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();

    if (_groupModel.addGruop(userid, groupid, GroupModel::Normal))
    {
        // 加入群成功
        json response;
        response["msgid"] = MsgType::ADD_GROUP_MSG_ACK;
        response["errno"] = Errno::SUCCESS;
        conn->send(response.dump());
    }
    else
    {
        // 加入群失败
        json response;
        response["msgid"] = MsgType::ADD_GROUP_MSG_ACK;
        response["errno"] = Errno::FAIL_MSG;
        response["errmsg"] = "Failed to join the group. The group number is incorrect or has been joined!";
        conn->send(response.dump());
    }
}

// 群发消息
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> users = _groupModel.queryGruopUsers(userid, groupid);

    // 加锁,要操作_userConnMap
    lock_guard<mutex> lock(_connMapMtx);
    for (int id : users)
    {
        if (_userConnMap.find(id) != _userConnMap.end())
        {
            // 在线直接发消息
            _userConnMap[id]->send(js.dump());
        }
        else if (_usermodel.query(id).getState() == "online")
        {
            // 看是否状态是online,是则需要跨服务通信,在redis的消息队列中publish
            redis.publish(id, js.dump());
        }
        else
        {
            // 存储离线消息
            _offlineModel.insert(id, js.dump());
        }
    }
}