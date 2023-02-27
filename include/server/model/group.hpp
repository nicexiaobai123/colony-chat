#pragma once
#include <vector>
#include "groupuser.hpp"

// 群组的ORM
class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
        : _id(id), _name(name), _desc(desc)
    {
        _users.clear();
    }

    void setId(int id) { this->_id = id; }
    void setName(const string &name) { this->_name = name; }
    void setDesc(const string &desc) { this->_desc = desc; }

    int getId() const { return this->_id; }
    string getName() const { return this->_name; }
    string getDesc() const { return this->_desc; }
    // 返回引用,外部可修改
    vector<GroupUser> &getUsers() { return this->_users; }

private:
    int _id;
    string _name;
    string _desc;
    // 既然是一个组,必然要包含组员
    vector<GroupUser> _users;
};