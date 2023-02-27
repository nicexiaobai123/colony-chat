#pragma once
#include <string>
#include <vector>
using namespace std;
// 没用ORM了(比较简单)，直接只封装了一层数据库操作
class OfflineMsgModel
{
public:
    // 存储离线消息
    void insert(int userid,string jsmsg);
    // 删除用户离线消息
    void remove(int userid);
    // 获取全部离线消息
    vector<string> query(int userid);
};