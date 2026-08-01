# PasswordManager Makefile
# 用法:
#   make          编译主程序和诊断测试
#   make main     仅编译主程序
#   make test     仅编译诊断测试
#   make clean    清理编译产物
#   make run      编译主程序并运行

# 编译器
CXX = g++
CXXFLAGS = -std=c++11 -Wall -finput-charset=UTF-8
INCLUDES = -Iinclude

# 目录
SRCDIR = src
INCDIR = include

# 主程序
MAIN_TARGET = password_manager
MAIN_SRCS = $(SRCDIR)/main.cpp $(SRCDIR)/crypto.cpp $(SRCDIR)/file_io.cpp $(SRCDIR)/user.cpp $(SRCDIR)/menu.cpp

# 加密诊断测试程序
TEST_TARGET = crypto_test
TEST_SRCS = $(SRCDIR)/crypto_test.cpp $(SRCDIR)/crypto.cpp

# 默认目标：编译全部
all: $(MAIN_TARGET) $(TEST_TARGET)

# 主程序
$(MAIN_TARGET): $(MAIN_SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(MAIN_SRCS) -o $@

# 诊断测试
$(TEST_TARGET): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_SRCS) -o $@

# 仅编译主程序
main: $(MAIN_TARGET)

# 仅编译诊断测试
test: $(TEST_TARGET)

# 运行诊断测试
runtest: $(TEST_TARGET)
	.\$(TEST_TARGET).exe

# 清理
clean:
	del /Q $(MAIN_TARGET).exe $(TEST_TARGET).exe 2>NUL

# 运行主程序
run: $(MAIN_TARGET)
	.\$(MAIN_TARGET).exe

.PHONY: all main test runtest clean run
