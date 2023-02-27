# 最低版本
cmake_minimum_required(VERSION 3.0)

# 工程名称
project(main)

# 配置编译选项
set(CMAKE_CXX_FLAGE ${CMAKE_CXX_FLAGE} -g)

# 头文件路径 
include_directories(./include)

# 库文件路径
#link_directories()

# 指定搜索的子目录,会按指定子目录里面的CMakeLists继续执行
#add_subdirectory(./src)

# 当前目录下所有源文件
aux_source_directory(./ SRC_LISTS)
#set(SRC_LISTS ./muduo_server.cpp ./testjson.cpp)

# 设置可执行文件最终存储的位置
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 表示生成可执行文件server 由SRC_LISTS变量的源文件编译
add_executable(server ${SRC_LISTS})

# 表示server这个目标程序 还需要链接三个库文件
target_link_libraries(server muduo_net muduo_base pthread)