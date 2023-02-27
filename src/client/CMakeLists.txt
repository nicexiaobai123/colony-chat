# SRC_LIST包含当前目录所有源文件
aux_source_directory(./ SRC_LIST)

# 指定生成可执行文件
add_executable(ChatClient ${SRC_LIST})

# 链接需要的库
target_link_libraries(ChatClient pthread mysqlclient)