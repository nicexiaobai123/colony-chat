#pragma once
#include <string>
using namespace std;

// ORM -- 对象关系映射
// 将类与数据库中的表一一对应起来
// 一个类表示一个表,一个类属性表示一个字段,一个对象表示一条记录

class User
{
public:
    User(int i = -1, string n = "", string pwd = "", string st = "offline")
        : id(i), name(n), password(pwd), state(st)
    {   }

    void setId(int id) { this->id = id; }
    void setName(const string &name) { this->name = name; }
    void setPwd(const string &pwd) { this->password = pwd; }
    void setState(const string &state) { this->state = state; }

    int getId() const { return this->id; }
    string getName() const { return this->name; }
    string getPwd() const { return this->password; }
    string getState() const { return this->state; }

protected:
    int id;
    string name;
    string password;
    string state;
};
