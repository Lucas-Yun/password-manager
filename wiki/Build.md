# 编译与构建

## 环境要求

| 项目 | 要求 |
|------|------|
| 操作系统 | Windows |
| 编译器 | MinGW g++（支持 C++11） |
| 依赖库 | Windows API（`windows.h`、`conio.h`） |
| 构建工具 | GNU Make（可选，支持手动编译） |

## 项目结构

```
PasswordManager/
├── include/                # 头文件
│   ├── types.h             # 数据结构与加密常量
│   ├── crypto.h            # 加密模块接口
│   ├── file_io.h           # 文件读写接口
│   ├── user.h              # 用户管理接口
│   └── menu.h              # 菜单接口
├── src/                    # 源文件
│   ├── crypto.cpp          # XTEA-CBC 加密实现
│   ├── file_io.cpp         # 文件读写实现
│   ├── user.cpp            # 注册/登录实现
│   ├── menu.cpp            # 菜单与业务逻辑
│   ├── main.cpp            # 程序入口
│   ├── convert.cpp         # 数据转换工具
│   └── crypto_test.cpp     # 加密诊断测试
├── Makefile                # 构建脚本
├── .gitignore
├── LICENSE
└── README.md
```

## 使用 Makefile

### 编译全部

```bash
make
```

编译生成三个可执行文件：
- `password_manager.exe` — 主程序
- `convert.exe` — 数据转换工具
- `crypto_test.exe` — 加密诊断测试

### 其他 Make 命令

| 命令 | 说明 |
|------|------|
| `make main` | 仅编译主程序 |
| `make convert` | 仅编译转换工具 |
| `make test` | 仅编译诊断测试 |
| `make runtest` | 编译并运行诊断测试 |
| `make run` | 编译主程序并运行 |
| `make clean` | 清理所有编译产物 |

### Makefile 内容

```makefile
CXX = g++
CXXFLAGS = -std=c++11 -Wall -finput-charset=UTF-8
INCLUDES = -Iinclude

# 主程序
MAIN_SRCS = src/main.cpp src/crypto.cpp src/file_io.cpp src/user.cpp src/menu.cpp

# 转换工具（链接 crypto.cpp 共享加密实现）
CONVERT_SRCS = src/convert.cpp src/crypto.cpp

# 诊断测试
TEST_SRCS = src/crypto_test.cpp src/crypto.cpp
```

> **关键**：转换工具和诊断测试都链接 `crypto.cpp`，确保与主程序使用完全相同的加密算法实现。

## 手动编译

如果没有 Make 工具，可以手动执行 g++ 命令：

### 主程序

```bash
g++ -std=c++11 -Wall -finput-charset=UTF-8 -Iinclude ^
    src/main.cpp src/crypto.cpp src/file_io.cpp src/user.cpp src/menu.cpp ^
    -o password_manager.exe
```

### 转换工具

```bash
g++ -std=c++11 -Wall -finput-charset=UTF-8 -Iinclude ^
    src/convert.cpp src/crypto.cpp ^
    -o convert.exe
```

### 诊断测试

```bash
g++ -std=c++11 -Wall -finput-charset=UTF-8 -Iinclude ^
    src/crypto_test.cpp src/crypto.cpp ^
    -o crypto_test.exe
```

## 编译选项说明

| 选项 | 说明 |
|------|------|
| `-std=c++11` | 使用 C++11 标准（代码中使用了 `auto`、范围 for 等） |
| `-Wall` | 开启所有常用警告 |
| `-finput-charset=UTF-8` | 指定源文件编码为 UTF-8（解决中文乱码） |
| `-Iinclude` | 添加头文件搜索路径 |

## 运行

```bash
# 主程序
password_manager.exe

# 数据转换工具
convert.exe
# 或指定目录
convert.exe D:\my\data\

# 诊断测试
crypto_test.exe
```

## 常见编译问题

### 中文乱码

确保编译时添加 `-finput-charset=UTF-8` 参数。程序运行时已通过 `SetConsoleOutputCP(65001)` 设置控制台为 UTF-8 编码。

### 找不到 `conio.h`

`conio.h` 是 Windows 特有头文件。如果使用 MSYS2/MinGW 环境，通常已包含。如果使用其他编译器（如 MSVC），需要替换 `_getch()` 和 `_kbhit()` 的实现。

### 链接错误：multiple definition of `main`

确保不要把 `convert.cpp` 或 `crypto_test.cpp` 和 `main.cpp` 一起编译。它们是独立的程序，各有自己的 `main()` 函数。

### 编译器路径

如果 `g++` 不在 PATH 中，需指定完整路径：

```bash
"D:\Program Files (x86)\Dev-Cpp\MinGW64\bin\g++.exe" -std=c++11 ...
```
