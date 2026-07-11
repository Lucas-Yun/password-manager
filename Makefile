# PasswordManager Makefile
# 用法:
#   make          编译生成可执行文件
#   make clean    清理编译产物
#   make run      编译并运行

# 编译器
CXX = g++
CXXFLAGS = -std=c++11 -Wall -finput-charset=UTF-8
INCLUDES = -Iinclude

# 源文件和目标文件
SRCDIR = src
INCDIR = include
TARGET = password_manager

SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 编译规则
$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# 清理
clean:
	del /Q $(SRCDIR)\*.o $(TARGET).exe 2>NUL

# 运行
run: $(TARGET)
	.\$(TARGET).exe

.PHONY: all clean run
