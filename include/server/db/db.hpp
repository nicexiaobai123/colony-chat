#pragma once
#include <string>
#include <mysql/mysql.h>
#include <muduo/base/Logging.h>
using namespace std;
using namespace muduo;

// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

// 数据库操作类
class MySQL
{
public:
    // 初始化数据库连接
    MySQL()
    {
        _conn = mysql_init(nullptr);
    }
    // 释放数据库连接资源
    ~MySQL()
    {
        if (_conn != nullptr)
            mysql_close(_conn);
    }
    // 连接数据库
    bool connect()
    {
        MYSQL *p = mysql_real_connect(
            _conn, server.c_str(),
            user.c_str(), password.c_str(),
            dbname.c_str(), 3306, nullptr, 0);
        if (p != nullptr)
        {
            // 设置编码
            // #1指 客户端 发送过来的sql语句的编码
            // #2指 mysqld 收到客户端的语句后,要转换到的编码
            // #2指 server 执行语句后，返回给客户端的数据的编码
            mysql_query(_conn, "set names utf8");
            LOG_INFO << "mysql connection is successful!";
        }
        else
        {
            LOG_INFO << "mysql connection is fail!";
        }
        return p;
    }
    // 更新操作
    bool update(string sql)
    {
        if (mysql_query(_conn, sql.c_str()))
        {
            LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败!";
            return false;
        }
        return true;
    }
    // 查询操作
    MYSQL_RES *query(string sql)
    {
        if (mysql_query(_conn, sql.c_str()))
        {
            LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败!";
            return nullptr;
        }
        return mysql_use_result(_conn);
    }
    // 得到底层Mysql连接
    MYSQL *getConnection() const { return _conn; }

private:
    MYSQL *_conn;
};