#pragma once
#include <string>
#include "user.hpp"
using namespace std;
// 群组的用户,需要一个role信息,其他与User表是一样的
class GroupUser : public User
{
public:
    // 设置用户角色
    void setRole(const string &role) { this->role = role; }
    // 获取用户角色
    string getRole() const { return role; };
    
private:
    string role;
};
