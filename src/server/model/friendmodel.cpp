#include "friendmodel.hpp"
// 添加好友
bool FriendModel::insert(int userid, int friendid)
{
    // 组装sql
    char sql[512];
    sprintf(sql, "insert into Friend values('%d','%d')", userid, friendid);
    // 存储消息
    MySQL mysql;
    if (mysql.connect())
    {
        return mysql.update(sql);
    }
    return false;
}

// 返回好友列表
vector<User> FriendModel::query(int userid)
{
    // 组装sql
    char sql[512]{0};
    sprintf(sql, "SELECT f.friendid,u.name,u.state \
            FROM Friend f INNER JOIN User u ON f.userid='%d' AND f.friendid=u.id ",
            userid);

    // 数据库查询
    MySQL mysql;
    vector<User> userVec;
    if (mysql.connect())
    {
        MYSQL_RES *result = mysql.query(sql);
        if (result != nullptr)
        {
            MYSQL_ROW row;
            User user;
            while ((row = mysql_fetch_row(result)) != nullptr)
            {
                // f.friendid,u.name,u.state 
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                userVec.push_back(user);
            }
            mysql_free_result(result);
        }
    }
    return userVec;
}