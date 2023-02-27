#pragma once
#include "group.hpp"
// 组群的操作接口
class GroupModel
{
public:
    // 创建组
    bool createGroup(Group &group);
    // 加入族群
    bool addGruop(int userId, int groupId, const string &role);
    // 查询用户所在群组信息 ---- 不仅返回一个独立的群信息,群中包含了组员(vector<GroupUser>)一起返回
    vector<Group> queryGroups(int userId);
    // 根据指定gid查询用户uid列表,返回除了本身uid其余所有,主要用于群聊
    vector<int> queryGruopUsers(int userId, int groupId);

public:
    static string Creator;
    static string Normal;
};