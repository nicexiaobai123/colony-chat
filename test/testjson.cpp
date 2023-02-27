#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "json.h"

// vscode里面如果是hpp要完整路径才能被语法提示工具搜索到,写完整路便可以不用加-I去指定头文件编译了
// vscode里面.h后缀名在不用完整路径语法提示工具都能搜索到,当然编译的时候注意得加-I去指定头文件
// #include "../include/json.hpp" 

using namespace std;
using json = nlohmann::json;

string func1(){
    json js;
    // 添加数组
    js["id"] = {1,2,3,4,5};
    // 添加字符串
    js["from"] = "lisi";
    js["to"] = "wangwu";
    // 添加对象
    js["msg"]["A"] = "hello";
    js["msg"]["B"] = "world";
    // 上面等同下面这句一次性添加
    js["msg"] = {{"A","hello"},{"B","world"}};
    return string(js.dump());
}

string func2(){
    json js;

    vector<int> vec1;
    vec1.push_back(1);
    vec1.push_back(2);
    vec1.push_back(3);
    js["vec"] = vec1;

    map<int,string> m;
    m[1] = "华山";
    m.insert({2,"花果山"});
    m.insert(make_pair<int,string>(3,"水帘洞"));
    js["map"] = m;
    // js.dump() 
    // 序列化: 是将数据结构或是对象转换为二进制串（字节序列）的过程，
    // 也就是将具有一定结构的对象转换为可以存储或传输的形式
    return string(js.dump());
}

#if 0
int main()
{
    // 反序列化:假设我们收到字符串形式的json，我们把它转成json格式
    string revBuf= func2();
    json recvJs = json::parse(revBuf);  
    cout << recvJs <<endl;
    
    return 0;
}
#endif