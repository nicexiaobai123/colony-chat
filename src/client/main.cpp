#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <unordered_map>
#include <functional>
#include <thread>
#include <string>
#include <atomic>
#include <semaphore.h>
#include <vector>
#include "json.hpp"
#include "public.hpp"
#include "user.hpp"
#include "group.hpp"
using namespace std;
using namespace nlohmann;

// 全局
bool isMainMenuRunning = false;      // 控制主菜单页面(登录后页面)程序
sem_t rwsem;                         // 用于读写线程之间的通信
atomic_bool g_isLoginSuccess{false}; // 记录登录状态

// ---------------------------------- 主函数 ----------------------------------
// ------------------ main线程用作发送线程，子线程用作接收线程 -------------------
// 子线程用作接收线程
void readTaskHandler(int clientfd);
// 聊天主菜单页面(登录后的页面)
void mainMenu(int clientfd);

// 登录选项
void login(socklen_t clientfd)
{
    int id;
    char pwd[50]{0};
    cout << "user-id:";
    cin >> id;
    cin.get(); // 读掉换行
    cout << "user-password:";
    cin.getline(pwd, 50);

    json js;
    js["msgid"] = MsgType::LOGIN_MSG;
    js["id"] = id;
    js["password"] = pwd;
    string request = js.dump();
    if (-1 == send(clientfd, request.c_str(), request.length() + 1, 0))
    {
        cerr << "send login msg error:" << request << endl;
        return;
    }

    // 子线程处理服务器的响应,信号量阻塞等待,处理完后通知这里
    sem_wait(&rwsem);
    if (g_isLoginSuccess)
    {
        // 控制运行主菜单页面
        isMainMenuRunning = true;
        // 子线程响应登录成功,进入聊天主菜单页面
        mainMenu(clientfd);
    }
}

// 注册选项
void reg(socklen_t clientfd)
{
    char name[50]{0};
    char pwd[50];
    cout << "user-name:";
    cin.getline(name, 50);
    cout << "user-password:";
    cin.getline(pwd, 50);

    json js;
    js["msgid"] = MsgType::REG_MSG;
    js["name"] = name;
    js["password"] = pwd;
    string request = js.dump();
    if (-1 == send(clientfd, request.c_str(), request.length() + 1, 0))
    {
        cerr << "send reg msg error:" << request << endl;
    }
    // 等待信号量，子线程处理完注册消息会通知
    sem_wait(&rwsem);
}

// 退出选项
void quit(socklen_t clientfd)
{
    cout << "quit !!" << endl;
    sem_destroy(&rwsem);
    close(clientfd);
    exit(0);
}

//  入口,网络模块+一些选项
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 6666" << endl;
        exit(-1);
    }

    // 命令解析
    char *ip = argv[1];
    unsigned int port = atoi(argv[2]);

    // 创建socket
    socklen_t clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

    // 服务端地址
    sockaddr_in serverAddr{0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(port);

    // 连接
    if (-1 == connect(clientfd, (sockaddr *)&serverAddr, sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    // 初始化读写线程通信用的信号量
    // 线程互斥,信号量初始值0
    sem_init(&rwsem, 0, 0);

    // 子线程用作接收线程
    std::thread td(readTaskHandler, clientfd);
    td.detach();

    // main线程负责接受客户端输入,发送数据
    using handler = function<void(socklen_t)>;
    unordered_map<int, handler> handlerMap;
    handlerMap.insert({1, login});
    handlerMap.insert({2, reg});
    handlerMap.insert({3, quit});
    while (true)
    {
        // 首页面菜单
        cout << "====================================" << endl;
        cout << "----- 1.login -----" << endl;
        cout << "----- 2.register -----" << endl;
        cout << "----- 3.quit -----" << endl;
        cout << "====================================" << endl;
        int choice;
        cin >> choice;
        // 处理输入字符如'a',cin这时会发生错误,并且上一次的输入会留在输入流中,导致后面cin不会阻塞输入
        cin.clear();
        // 读掉缓冲区的回车,防止下一次cin读取到'\n'
        cin.get();

        // 调用对应的处理函数
        auto it = handlerMap.find(choice);
        if (it != handlerMap.end())
        {
            it->second(clientfd);
        }
        else
        {
            // 接受到错误的消息id
            cerr << "invalid choice !" << endl;
        }
    }
    return 0;
}

// ---------------------------------- 子线程函数 ----------------------------------
// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendVec;
// 记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupVec;

// 显示当前登录成功用户的基本信息
void showCurrentUserData()
{
    cout << "======================login user======================" << endl;
    cout << " current login user [id]:" << g_currentUser.getId()
         << "  [name]:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendVec.empty())
    {
        for (auto user : g_currentUserFriendVec)
        {
            cout << "id:" << user.getId()
                 << "  name:" << user.getName()
                 << "  state:" << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroupVec.empty())
    {
        for (Group &group : g_currentUserGroupVec)
        {
            cout << "群组:" << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << "    组员:" << user.getId() << " " << user.getName()
                     << " " << user.getState() << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}

// 处理登录
void doLoginResponse(json &responsejs)
{
    int err = responsejs["errno"].get<int>();
    // 响应登录失败
    if (err != Errno::SUCCESS)
    {
        cerr << responsejs["errmsg"] << endl;
    }
    // 响应登录成功
    else
    {
        // 记录当前用户的id和name(注销再登录,又重新设置)
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);

        // 记录当前用户的好友列表信息
        if (responsejs.contains("friends")) // 是否存在"friends"
        {
            vector<string> vec = responsejs["friends"];
            for (const string &str : vec)
            {
                json frdjs = json::parse(str);
                User user(frdjs["friendid"].get<int>(), frdjs["name"], "", frdjs["state"]);
                // // 放入当前登录用户的好友列表列表(全局变量)
                g_currentUserFriendVec.push_back(user);
            }
        }

        // 记录当前用户的群组列表信息
        if (responsejs.contains("groups")) // 是否存在"groups"
        {
            vector<string> vec = responsejs["groups"];
            for (const string &grpstr : vec)
            {
                json groupjs = json::parse(grpstr);
                // 组信息
                Group group(groupjs["id"].get<int>(), groupjs["groupname"], groupjs["groupdesc"]);
                // 组员信息
                vector<string> usersV = groupjs["users"];
                for (const string &userstr : usersV)
                {
                    json userjs = json::parse(userstr);
                    GroupUser user;
                    user.setId(userjs["userid"].get<int>());
                    user.setName(userjs["name"]);
                    user.setState(userjs["state"]);
                    user.setRole(userjs["role"]);
                    group.getUsers().push_back(user);
                }
                // 放入当前登录用户的群组列表(全局变量)
                g_currentUserGroupVec.push_back(group);
            }
        }

        // 显示当前离线信息
        if (responsejs.contains("offlinemsg")) // 是否存在"offlinemsg"
        {
            cout << "--------------------- offlinemsg --------------------- " << endl;
            vector<string> offMsgVec = responsejs["offlinemsg"];
            for (const string &offMsg : offMsgVec)
            {
                // {"msgid":ONE_CHAT_MSG,"fromid":1,"fromname":"li","toid":4,"msg":"hello"}
                //  time + [id] + name + " said: " + xxx
                json js = json::parse(offMsg);
                if (MsgType::ONE_CHAT_MSG == js["msgid"].get<int>())
                {
                    int id = js["fromid"].get<int>();
                    string name = js["fromname"];
                    string msg = js["msg"];
                    string time = js["time"];
                    cout << time << " [" << id << "]" << name << " said: " << msg << endl;
                }
                else if (MsgType::GROUP_CHAT_MSG == js["msgid"].get<int>())
                {
                    //{"msgid":GROUP_CHAT_MSG,"userid":1,"groupid":1,"groupmsg":"hello ggm"}
                    int groupid = js["groupid"].get<int>();
                    int userid = js["userid"].get<int>();
                    string username = js["username"];
                    string groupmsg = js["groupmsg"];
                    string time = js["time"];
                    cout << "群消息[" << groupid << "] " << time << " [" << userid << "]"
                         << username << " said: " << groupmsg << endl;
                }
            }
        }

        // 显示当前登录成功用户的基本信息(用户的id和name、好友、群组)
        showCurrentUserData();

        // 修改登录成功标签
        g_isLoginSuccess = true;
    }
}

// 处理注册的响应逻辑
void doRegResponse(json &responsejs)
{
    if (Errno::SUCCESS == responsejs["errno"].get<int>())
    {
        cout << "register is success,please do not forget id: " << responsejs["id"] << endl;
    }
    else
    {
        cerr << "name is already exist, register error!" << endl;
    }
}

// 子线程用作接收消息线程
void readTaskHandler(int clientfd)
{
    while (true)
    {
        // 这个接受recv要处理一下
        // 如果服务端端发送的数据过于大,4096存储不下;或需要非阻塞+轮询
        char buffer[4096]{0};
        if (recv(clientfd, buffer, 4096, 0) == 0)
        {
            cerr << "connection is closed" << endl;
            close(clientfd);
            exit(-1);
        }

        // 接受服务器发的数据,反序化成json
        json js = json::parse(buffer);
        int msgType = js["msgid"].get<int>();

        // 各消息处理
        switch (msgType)
        {
        case MsgType::REG_MSG_ACK:
        {
            doRegResponse(js); // 处理注册响应
            sem_post(&rwsem);  // 通知主线程(发送线程),注册结果处理完
            break;
        }
        case MsgType::LOGIN_MSG_ACK:
        {
            doLoginResponse(js); // 处理登录响应
            sem_post(&rwsem);    // 通知主线程(发送线程),登录结果处理完
            break;
        }
        case MsgType::CREATE_GROUP_MSG_ACK:
        {
            if (Errno::SUCCESS == js["errno"])
            {
                cout << "create group success, groupid is " << js["groupid"].get<int>() << endl;
            }
            else
            {
                cout << js["errmsg"].get<string>() << endl;
            }
            break;
        }
        case MsgType::ADD_GROUP_MSG_ACK:
        {
            if (Errno::SUCCESS == js["errno"])
            {
                cout << "Successfully joined the group!" << endl;
            }
            else
            {
                cout << js["errmsg"].get<string>() << endl;
            }
            break;
        }
        case MsgType::ONE_CHAT_MSG:
        {
            int id = js["fromid"].get<int>();
            string name = js["fromname"];
            string msg = js["msg"];
            string time = js["time"];
            cout << time << " [" << id << "]" << name << " said: " << msg << endl;
            break;
        }
        case MsgType::GROUP_CHAT_MSG:
        {
            int groupid = js["groupid"].get<int>();
            int userid = js["userid"].get<int>();
            string username = js["username"];
            string groupmsg = js["groupmsg"];
            string time = js["time"];
            cout << "群消息[" << groupid << "] " << time << " [" << userid << "]"
                 << username << " said: " << groupmsg << endl;
            break;
        }
        default:
        {
            cerr << "msgtype is " << js["msgid"].get<string>() << "error!" << endl;
            break;
        }
        }
    }
}

// --------------------------------- 登录过后的业务(主线程) ---------------------------------
// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();

// 客户端命令的函数声明
void help(int fd = 0, string str = "");
void chat(int, string);
void addfriend(int, string);
void creategroup(int, string);
void joingroup(int, string);
void groupchat(int, string);
void logout(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令: 格式 -- help"},
    {"chat", "一对一聊天: 格式 -- chat:friendid:message"},
    {"addfriend", "添加好友: 格式 -- addfriend:friendid"},
    {"creategroup", "创建群组: 格式 -- creategroup:groupname:groupdesc"},
    {"joingroup", "加入群组: 格式 -- joingroup:groupid"},
    {"groupchat", "群聊: 格式 -- groupchat:groupid:message"},
    {"logout", "退出: 格式 -- logout"}};

// 注册系统支持的客户端命令处理
// 两个参数,clientfd 和 命令字符串
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"joingroup", joingroup},
    {"groupchat", groupchat},
    {"logout", logout}};

// 聊天主菜单页面
void mainMenu(int clientfd)
{
    help();
    char buffer[256]{0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 256);
        string commandbuf(buffer);
        string command; // 命令存储,chat、help等
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            // help或者logout
            command = commandbuf;
        }
        else
        {
            // 其他
            command = commandbuf.substr(0, idx);
        }

        // 寻找对应handler
        auto it = commandHandlerMap.find(command);
        if (it != commandHandlerMap.end())
        {
            // 参数在命令后
            string args = commandbuf.substr(idx + 1, commandbuf.size() - idx);
            // 调用相应命令的事件处理回调，mainMenu可以对修改封闭，添加新功能不需要修改该函数
            it->second(clientfd, args);
        }
        else
        {
            cout << "invalid input command!" << endl;
        }
    }
}

// "help" command handler
void help(int fd, string str)
{
    for (auto &it : commandMap)
    {
        cout << it.first << "   " << it.second << endl;
    }
    cout << endl;
}

// "chat" command handler
void chat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }
    int toid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = MsgType::ONE_CHAT_MSG;
    js["fromid"] = g_currentUser.getId();
    js["fromname"] = g_currentUser.getName();
    js["toid"] = toid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buf = js.dump();

    int ret = send(clientfd, buf.c_str(), buf.length() + 1, 0);
    if (ret == -1)
    {
        cerr << "send chat msg error -> " << buf << endl;
    }
}

// "addfriend" command handler
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());

    json js;
    js["msgid"] = MsgType::ADD_FRIEND_MSG;
    js["userid"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buf = js.dump();

    int ret = send(clientfd, buf.c_str(), buf.length() + 1, 0);
    if (ret == -1)
    {
        cerr << "send addfriend msg error -> " << buf << endl;
    }
}

// "creategroup" command handler
void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }
    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = MsgType::CREATE_GROUP_MSG;
    js["creatorid"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buf = js.dump();

    int ret = send(clientfd, buf.c_str(), buf.length() + 1, 0);
    if (ret == -1)
    {
        cerr << "send creategroup msg error -> " << buf << endl;
    }
}

// "joingroup" command handler groupid
void joingroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());

    json js;
    js["msgid"] = MsgType::ADD_GROUP_MSG;
    js["userid"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buf = js.dump();

    int ret = send(clientfd, buf.c_str(), buf.length() + 1, 0);
    if (ret == -1)
    {
        cerr << "send joingroup msg error -> " << buf << endl;
    }
}

// "groupchat" command handler groupid:message
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }
    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = MsgType::GROUP_CHAT_MSG;
    js["userid"] = g_currentUser.getId();
    js["username"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["time"] = getCurrentTime();
    js["groupmsg"] = message;
    string buf = js.dump();

    int ret = send(clientfd, buf.c_str(), buf.length() + 1, 0);
    if (ret == -1)
    {
        cerr << "send groupchat msg error -> " << buf << endl;
    }
}

// "logout" command handler
void logout(int clientfd, string)
{
    json js;
    js["msgid"] = MsgType::LOGINOUT_MSG;
    js["userid"] = g_currentUser.getId();
    string buf = js.dump();

    int ret = send(clientfd, buf.c_str(), buf.length() + 1, 0);
    if (ret != -1)
    {
        // 注销成功
        isMainMenuRunning = false;      // 主窗口(用户登录后的窗口)不在显示
        g_currentUserFriendVec.clear(); // 清空好友列表信息
        g_currentUserGroupVec.clear();  // 清空群组列表信息
    }
    else
    {
        cerr << "send addfriend msg error -> " << buf << endl;
    }
}

// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime()
{
    time_t tt = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm *ptm = localtime(&tt);
    char date[60]{0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}