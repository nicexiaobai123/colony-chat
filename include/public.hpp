#pragma once
// clinet 与 server 所需要的公共消息id
enum MsgType
{
    LOGIN_MSG = 1,  // 登录
    LOGIN_MSG_ACK,  // 登录响应
    REG_MSG,        // 注册
    REG_MSG_ACK,    // 注册响应
    ONE_CHAT_MSG,   // 点对点聊天
    ADD_FRIEND_MSG, // 添加好友

    CREATE_GROUP_MSG,     // 新建群
    CREATE_GROUP_MSG_ACK, // 新建群响应
    ADD_GROUP_MSG,        // 加入群
    ADD_GROUP_MSG_ACK,    // 加入群响应
    GROUP_CHAT_MSG,       // 群聊

    LOGINOUT_MSG          // 注销信息
};

// 错误号小说明
// 0成功  1失败不携带信息  2失败携带信息
enum Errno
{
    SUCCESS = 0,
    FAIL = 1,
    FAIL_MSG = 2
};