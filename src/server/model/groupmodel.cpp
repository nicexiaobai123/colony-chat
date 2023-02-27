#include "groupmodel.hpp"
#include "db.hpp"

// 静态变量,组角色
string GroupModel::Creator = "creator";
string GroupModel::Normal = "normal";

// 创建组
bool GroupModel::createGroup(Group &group)
{
    char sql[512];
    sprintf(sql, "insert into AllGroup(groupname,groupdesc) values('%s','%s')",
            group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 加入族群
bool GroupModel::addGruop(int userId, int groupId, const string &role)
{
    char sql[512];
    sprintf(sql, "insert into GroupUser values('%d','%d','%s')",groupId ,userId , role.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

// #1先根据userid在UserGroup表中的关系查找出该用户所属群信息
// #2再根据其groupid,查询属于该群组的所有用户的id,并且与User表作多表查询,查出用户详细信息
vector<Group> GroupModel::queryGroups(int userId)
{
    char sql[512];
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from \
            AllGroup a inner join GroupUser b on a.id = b.groupid and b.userid='%d'",
            userId);

    // 查询userid所在群组
    vector<Group> groupVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *result = mysql.query(sql);
        if (result != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != nullptr)
            {
                Group group(atoi(row[0]), row[1], row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(result);
        }
    }

    // 查询其群组组员信息
    for (Group &group : groupVec)
    {
        sprintf(sql, "select b.id,b.name,b.state,a.grouprole \
                from GroupUser a inner join User b on a.userid = b.id and a.groupid = '%d'",
                group.getId());
        MySQL mysql;
        if (mysql.connect())
        {
            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr)
            {
                MYSQL_ROW row = nullptr;
                while ((row = mysql_fetch_row(res)) != nullptr)
                {
                    GroupUser gUser;
                    gUser.setId(atoi(row[0]));
                    gUser.setName(row[1]);
                    gUser.setState(row[2]);
                    gUser.setRole(row[3]);
                    group.getUsers().push_back(gUser);
                }
                mysql_free_result(res);
            }
        }
    }
    return groupVec;
}

// 根据指定gid查询用户uid列表,返回除了本身uid其余所有,主要用于群聊
vector<int> GroupModel::queryGruopUsers(int userId, int groupId)
{
    char sql[512];
    sprintf(sql, "select userid from GroupUser where userid!='%d' and groupid='%d'", userId, groupId);
    // 查询群其他所有id
    MySQL mysql;
    vector<int> useridVec;
    if (mysql.connect())
    {
        MYSQL_RES *result = mysql.query(sql);
        if (result != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != nullptr)
            {
                useridVec.push_back(atoi(row[0]));
            }
            mysql_free_result(result);
        }
    }
    return useridVec;
}