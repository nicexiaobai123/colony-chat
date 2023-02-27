#include "offlinemsgmodel.hpp"
#include "db.hpp"

// 存储离线消息
void OfflineMsgModel::insert(int userid, string jsmsg)
{
    // 组装sql
    char sql[512];
    sprintf(sql, "insert into OfflineMessage values('%d','%s')", userid, jsmsg.c_str());
    // 存储消息
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 删除用户离线消息
void OfflineMsgModel::remove(int userid)
{
    // 组装sql
    char sql[512];
    sprintf(sql, "delete from OfflineMessage where userid='%d'", userid);
    // 删除消息
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 获取全部离线消息
vector<string> OfflineMsgModel::query(int userid)
{
    // 组装sql
    char sql[512];
    sprintf(sql, "select message from OfflineMessage where userid='%d'", userid);
    // 获取消息
    MySQL mysql;
    vector<string> msgVec;
    if (mysql.connect())
    {
        MYSQL_RES *result = mysql.query(sql);
        if (result != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != nullptr)
            {
                msgVec.push_back(row[0]);
            }
            mysql_free_result(result);
        }
    }
    return msgVec;
}