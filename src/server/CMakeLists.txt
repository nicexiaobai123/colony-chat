# SRC_LIST包含当前目录所有源文件
aux_source_directory(./ SRC_LIST)
aux_source_directory(./model SRC_MODEL)
aux_source_directory(./redis SRC_REDIS)

# 指定生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${SRC_MODEL} ${SRC_REDIS})

# 链接需要的库
target_link_libraries(ChatServer muduo_net muduo_base pthread mysqlclient hiredis)