# 指定最低CMake版本，按需调整
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(password_manager)

# 1. 设置C++标准 C++11，等价 -std=c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 添加头文件搜索路径，等价 -Iinclude
include_directories(include)

# 3. 收集所有源代码文件
set(SOURCES
    srcmain.cpp
    srccrypto.cpp
    srcfile_io.cpp
    srcuser.cpp
    srcmenu.cpp
)

# 4. 创建可执行程序，输出名称 password_manager.exe
add_executable(password_manager ${SOURCES})

# ========== 传入gcc编译选项 ==========
# 等价：-Wall -finput-charset=UTF-8
if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_compile_options(password_manager PRIVATE
        -Wall
        -finput-charset=UTF-8
    )
endif()
