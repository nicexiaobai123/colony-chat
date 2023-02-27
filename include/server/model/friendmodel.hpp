#pragma once
#include "db.hpp"
#include "user.hpp"
#include <vector>
using namespace std;
// 好友表数据库操作,简单未有ORM
class FriendModel
{
public:
    // 添加好友
    bool insert(int userid, int friendid);
    // 返回好友列表
    vector<User> query(int userid);
     // 删除好友
    bool remove();
};