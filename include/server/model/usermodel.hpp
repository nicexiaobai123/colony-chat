#pragma once
#include "user.hpp"
#include "db.hpp"
//     -- 操作对象关系映射
//     -- 让业务代码与数据库操作分离
//     -- 无需关心底层是何种数据库,是何种sql语句
//     -- 业务层只需使用对象User和UserORM就可操作数据

// 本class实际操作数据库 
class UserModel
{
public:
    // User表增加数据方法
    bool insert(User& user);
    // User表查询方法,根据id得到User
    User query(int id);
    // User表根据id更新状态
    bool updateState(const User& user);
    // 重置用户state信息
    void resetState();
};