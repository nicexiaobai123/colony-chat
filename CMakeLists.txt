cmake_minimum_required(VERSION 3.0)
project(Chat)

# 配置编译选项
set(CMAKE_CXX_FLAGE ${CMAKE_CXX_FLAGE} -g)
SET(CMAKE_BUILD_TYPE "Debug")

# 设置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 头文件路径 
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)

# 指定搜索的子目录
add_subdirectory(${PROJECT_SOURCE_DIR}/src)