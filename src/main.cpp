#include "menu.h"
#include "file_io.h"
#include "types.h"
#include <windows.h>
#include <clocale>
#include <iostream>
#include <map>
#include <string>

using namespace std;

int main() {
    // 设置控制台代码页为 UTF-8
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    setlocale(LC_ALL, "Chinese_People's Republic of China.utf8");

    // 创建数据目录
    createDirectory(DATA_DIR);

    // 首次运行时自动创建管理员账户
    map<string, string> users = loadUsers();
    if (users.find(ADMIN_USERNAME) == users.end()) {
        users[ADMIN_USERNAME] = ADMIN_PASSWORD;
        saveUsers(users);
    }

    // 启动系统
    showLoginMenu();
    return 0;
}
