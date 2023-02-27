#include "usermodel.hpp"
#include <iostream>
using namespace std;
// User表增加数据方法
bool UserModel::insert(User &user)
{
    // 组装sql
    char sql[512]{0};
    sprintf(sql, "insert into User(name,password,state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    // 数据库更新
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功后自增长主键的id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// User表查询方法,根据id得到User
User UserModel::query(int id)
{
    // 组装sql
    char sql[512]{0};
    sprintf(sql, "select id,name,password,state from User where id=%d", id);

    // 数据库查询
    MySQL mysql;
    User user;
    if (mysql.connect())
    {
        MYSQL_RES *result = mysql.query(sql);
        if (result != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row != nullptr)
            {
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
            }
            mysql_free_result(result);
        }
    }
    return user;
}

// User表根据id更新状态
bool UserModel::updateState(const User &user)
{
    // 组装sql
    char sql[512]{0};
    sprintf(sql, "update User set state='%s' where id = '%d'", user.getState().c_str(), user.getId());
    // 数据库更新
    MySQL mysql;
    if (mysql.connect())
    {
        return mysql.update(sql);
    }
    return false;
}

// 重置用户state信息
void UserModel::resetState()
{
    char sql[512] = "update User set state='offline' where state='online'";
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}