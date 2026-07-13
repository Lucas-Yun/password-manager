#include "menu.h"
#include "crypto.h"
#include "file_io.h"
#include "user.h"
#include "types.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <conio.h>
#include <windows.h>
#include <ctime>

using namespace std;

// ========== 普通用户功能 ==========

void addInfo(const string& username) {
    cout << "\n=== 添加信息 ===" << endl;
    cin.ignore();

    cout << "请输入平台名称: ";
    string platform;
    getline(cin, platform);

    if (platform.empty()) {
        cout << "平台名称不能为空！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "请输入账号（输入完成后直接回车，结束输入也直接回车）:" << endl;
    vector<string> accounts;
    while (true) {
        cout << "账号" << (accounts.size() + 1) << ": ";
        string account;
        getline(cin, account);
        if (account.empty()) {
            break;
        }
        accounts.push_back(account);
        cout << "已添加账号: " << account << endl;
    }

    if (accounts.empty()) {
        cout << "至少需要输入一个账号！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "请选择密码输入方式:" << endl;
    cout << "1、手动输入密码" << endl;
    cout << "2、生成随机强密码" << endl;
    cout << "请选择（输入序号）: ";
    string pwdChoice;
    getline(cin, pwdChoice);

    string password;
    if (pwdChoice == "2") {
        cout << "请输入密码长度（默认16）: ";
        string lenStr;
        getline(cin, lenStr);
        int length = 16;
        if (!lenStr.empty()) {
            length = atoi(lenStr.c_str());
            if (length < 8) length = 8;
            if (length > 64) length = 64;
        }
        password = generateRandomPassword(length);
        cout << "生成的密码: " << password << endl;
        cout << "密码强度: " << checkPasswordStrength(password) << endl;
    } else {
        cout << "请输入密码: ";
        password = getHiddenInput();
    }

    if (password.empty()) {
        cout << "密码不能为空！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "密码强度: " << checkPasswordStrength(password) << endl;

    map<string, PlatformInfo> passwords = loadPasswords(username);
    PlatformInfo info;
    info.accounts = accounts;
    info.password = password;
    passwords[platform] = info;
    savePasswords(username, passwords);

    cout << "信息添加成功！" << endl;
    cout << "按任意键返回..." << endl;
    _getch();
}

void queryInfo(const string& username) {
    cout << "\n=== 查询信息 ===" << endl;
    cin.ignore();

    cout << "请输入平台名称: ";
    string platform;
    getline(cin, platform);

    map<string, PlatformInfo> passwords = loadPasswords(username);
    auto it = passwords.find(platform);

    if (it == passwords.end()) {
        cout << "未查询到信息" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "平台 [" << platform << "] 的信息如下:" << endl;
    cout << "账号列表:" << endl;
    for (size_t i = 0; i < it->second.accounts.size(); i++) {
        cout << "  " << (i + 1) << ". " << it->second.accounts[i] << endl;
    }
    cout << "密码: " << it->second.password << endl;
    cout << "（30秒后自动返回，或按任意键立即返回）" << endl;

    time_t startTime = time(nullptr);
    while (true) {
        if (_kbhit()) {
            _getch();
            break;
        }
        time_t currentTime = time(nullptr);
        if (difftime(currentTime, startTime) >= 30) {
            break;
        }
        Sleep(100);
    }
}

void deleteInfo(const string& username) {
    cout << "\n=== 删除信息 ===" << endl;
    cin.ignore();

    cout << "请输入要删除的平台名称: ";
    string platform;
    getline(cin, platform);

    map<string, PlatformInfo> passwords = loadPasswords(username);
    auto it = passwords.find(platform);

    if (it == passwords.end()) {
        cout << "未找到该平台的信息" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "确认删除平台 [" << platform << "] 的所有信息？(输入 Y 确认，其他键取消): ";
    char confirm = _getch();
    cout << confirm << endl;

    if (confirm == 'Y' || confirm == 'y') {
        passwords.erase(it);
        savePasswords(username, passwords);
        cout << "删除成功！" << endl;
    } else {
        cout << "已取消删除。" << endl;
    }

    cout << "按任意键返回..." << endl;
    _getch();
}

void editInfo(const string& username) {
    cout << "\n=== 修改信息 ===" << endl;
    cin.ignore();

    cout << "请输入要修改的平台名称: ";
    string platform;
    getline(cin, platform);

    if (platform.empty()) {
        cout << "平台名称不能为空！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    map<string, PlatformInfo> passwords = loadPasswords(username);
    auto it = passwords.find(platform);

    if (it == passwords.end()) {
        cout << "未找到该平台的信息" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "平台 [" << platform << "] 当前信息:" << endl;
    cout << "当前账号列表:" << endl;
    for (size_t i = 0; i < it->second.accounts.size(); i++) {
        cout << "  " << (i + 1) << ". " << it->second.accounts[i] << endl;
    }
    cout << "当前密码: " << it->second.password << endl;

    cout << "\n请选择要修改的内容:" << endl;
    cout << "1、修改账号" << endl;
    cout << "2、修改密码" << endl;
    cout << "3、修改账号和密码" << endl;
    cout << "0、取消修改" << endl;
    cout << "请选择（输入序号）: ";
    string choice;
    getline(cin, choice);

    if (choice == "0") {
        cout << "已取消修改。" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    vector<string> newAccounts = it->second.accounts;
    string newPassword = it->second.password;

    if (choice == "1" || choice == "3") {
        cout << "请输入新的账号列表（输入完成后直接回车，结束输入也直接回车）:" << endl;
        newAccounts.clear();
        while (true) {
            cout << "账号" << (newAccounts.size() + 1) << ": ";
            string account;
            getline(cin, account);
            if (account.empty()) {
                break;
            }
            newAccounts.push_back(account);
            cout << "已添加账号: " << account << endl;
        }

        if (newAccounts.empty()) {
            cout << "至少需要输入一个账号！修改失败。" << endl;
            cout << "按任意键返回..." << endl;
            _getch();
            return;
        }
    }

    if (choice == "2" || choice == "3") {
        cout << "请选择密码输入方式:" << endl;
        cout << "1、手动输入密码" << endl;
        cout << "2、生成随机强密码" << endl;
        cout << "请选择（输入序号）: ";
        string pwdChoice;
        getline(cin, pwdChoice);

        if (pwdChoice == "2") {
            cout << "请输入密码长度（默认16）: ";
            string lenStr;
            getline(cin, lenStr);
            int length = 16;
            if (!lenStr.empty()) {
                length = atoi(lenStr.c_str());
                if (length < 8) length = 8;
                if (length > 64) length = 64;
            }
            newPassword = generateRandomPassword(length);
            cout << "生成的密码: " << newPassword << endl;
            cout << "密码强度: " << checkPasswordStrength(newPassword) << endl;
        } else {
            cout << "请输入新密码: ";
            newPassword = getHiddenInput();
        }

        if (newPassword.empty()) {
            cout << "密码不能为空！修改失败。" << endl;
            cout << "按任意键返回..." << endl;
            _getch();
            return;
        }

        cout << "密码强度: " << checkPasswordStrength(newPassword) << endl;
    }

    if (choice == "1" || choice == "2" || choice == "3") {
        it->second.accounts = newAccounts;
        it->second.password = newPassword;
        savePasswords(username, passwords);
        cout << "修改成功！" << endl;
    } else {
        cout << "输入无效，已取消修改。" << endl;
    }

    cout << "按任意键返回..." << endl;
    _getch();
}

void listPlatforms(const string& username) {
    cout << "\n=== 平台列表 ===" << endl;

    map<string, PlatformInfo> passwords = loadPasswords(username);

    if (passwords.empty()) {
        cout << "当前没有保存任何平台信息。" << endl;
    } else {
        cout << "已保存的平台列表（共 " << passwords.size() << " 个）:" << endl;
        int index = 1;
        for (const auto& pair : passwords) {
            cout << index << ". " << pair.first;
            cout << " （账号数: " << pair.second.accounts.size() << "）" << endl;
            index++;
        }
    }

    cout << "按任意键返回..." << endl;
    _getch();
}

void exportData(const string& username) {
    cout << "\n=== 导出数据 ===" << endl;

    map<string, PlatformInfo> passwords = loadPasswords(username);

    if (passwords.empty()) {
        cout << "当前没有数据可导出。" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    string filename = DATA_DIR + username + "_backup.txt";
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cout << "创建导出文件失败！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    outFile << "=== 密码备份文件 ===" << endl;
    outFile << "用户名: " << username << endl;
    outFile << "备份时间: " << __DATE__ << " " << __TIME__ << endl;
    outFile << "========================" << endl << endl;

    int index = 1;
    for (const auto& pair : passwords) {
        outFile << index << ". 平台: " << pair.first << endl;
        outFile << "   账号列表:" << endl;
        for (size_t i = 0; i < pair.second.accounts.size(); i++) {
            outFile << "   - " << pair.second.accounts[i] << endl;
        }
        outFile << "   密码: " << pair.second.password << endl;
        outFile << endl;
        index++;
    }

    outFile.close();
    cout << "数据导出成功！" << endl;
    cout << "导出文件路径: " << filename << endl;
    cout << "警告：导出文件为明文存储，请妥善保管并及时删除！" << endl;
    cout << "按任意键返回..." << endl;
    _getch();
}

void showUserMenu(const string& username) {
    while (true) {
        system("cls");
        cout << "=== 密码管理系统 ===" << endl;
        cout << "当前用户: " << username << endl;
        cout << "1、添加信息" << endl;
        cout << "2、查询信息" << endl;
        cout << "3、删除信息" << endl;
        cout << "4、修改信息" << endl;
        cout << "5、平台列表" << endl;
        cout << "6、导出数据" << endl;
        cout << "7、返回上一级" << endl;
        cout << "请选择（输入序号）: ";

        string choice;
        cin >> choice;

        if (choice == "1") {
            addInfo(username);
        } else if (choice == "2") {
            queryInfo(username);
        } else if (choice == "3") {
            deleteInfo(username);
        } else if (choice == "4") {
            editInfo(username);
        } else if (choice == "5") {
            listPlatforms(username);
        } else if (choice == "6") {
            exportData(username);
        } else if (choice == "7") {
            cout << "返回上一级..." << endl;
            break;
        } else {
            cout << "输入无效，请输入1、2、3、4、5、6或7！" << endl;
            cout << "按任意键继续..." << endl;
            cin.ignore();
            _getch();
        }
    }
}

// ========== 管理员功能 ==========

void adminAddUsers() {
    cout << "\n=== 批量添加用户 ===" << endl;
    cout << "请输入要添加的用户数量: ";
    int count;
    cin >> count;

    if (count <= 0) {
        cout << "数量无效！" << endl;
        return;
    }

    map<string, string> users = loadUsers();
    int successCount = 0;

    for (int i = 0; i < count; i++) {
        cout << "\n--- 用户 " << (i + 1) << " ---" << endl;
        cout << "请输入用户名: ";
        string username;
        cin >> username;

        if (username.empty()) {
            cout << "用户名不能为空，跳过！" << endl;
            continue;
        }

        if (username == ADMIN_USERNAME) {
            cout << "该用户名是系统保留用户名，跳过！" << endl;
            continue;
        }

        if (users.find(username) != users.end()) {
            cout << "用户名已存在，跳过！" << endl;
            continue;
        }

        cout << "请输入密码: ";
        string password = getHiddenInput();

        if (password.empty()) {
            cout << "密码不能为空，跳过！" << endl;
            continue;
        }

        users[username] = password;
        string userFile = DATA_DIR + username + ".dat";
        ofstream file(userFile, ios::binary);
        file.close();

        cout << "用户 " << username << " 添加成功！" << endl;
        successCount++;
    }

    saveUsers(users);
    cout << "\n批量添加完成！成功添加 " << successCount << " 个用户。" << endl;
    cout << "按任意键返回..." << endl;
    _getch();
}

void adminDeleteUsers() {
    cout << "\n=== 删除用户 ===" << endl;
    map<string, string> users = loadUsers();

    if (users.empty()) {
        cout << "当前没有普通用户！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "当前用户列表:" << endl;
    int index = 1;
    for (const auto& pair : users) {
        cout << index << ". " << pair.first << endl;
        index++;
    }

    cout << "\n请输入要删除的用户名（或输入 0 返回）: ";
    string username;
    cin >> username;

    if (username == "0") {
        return;
    }

    if (users.find(username) == users.end()) {
        cout << "用户不存在！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "确认删除用户 " << username << "？(输入 Y 确认，其他键取消): ";
    char confirm = _getch();
    cout << confirm << endl;

    if (confirm == 'Y' || confirm == 'y') {
        users.erase(username);
        saveUsers(users);

        string userFile = DATA_DIR + username + ".dat";
        ifstream filecheck(userFile);
        if (filecheck.good()) {
            filecheck.close();
            remove(userFile.c_str());
        }

        cout << "用户删除成功！" << endl;
    } else {
        cout << "已取消删除。" << endl;
    }

    cout << "按任意键返回..." << endl;
    _getch();
}

void adminViewUsers() {
    cout << "\n=== 查看用户列表 ===" << endl;
    map<string, string> users = loadUsers();

    if (users.empty()) {
        cout << "当前没有普通用户。" << endl;
    } else {
        cout << "当前用户列表（共 " << users.size() << " 个用户）:" << endl;
        int index = 1;
        for (const auto& pair : users) {
            cout << index << ". " << pair.first << endl;
            index++;
        }
    }

    cout << "按任意键返回..." << endl;
    _getch();
}

void adminModifyUserPassword() {
    cout << "\n=== 修改用户密码 ===" << endl;
    map<string, string> users = loadUsers();

    if (users.empty()) {
        cout << "当前没有普通用户！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "当前用户列表:" << endl;
    int index = 1;
    for (const auto& pair : users) {
        cout << index << ". " << pair.first << endl;
        index++;
    }

    cout << "\n请输入要修改密码的用户名（或输入 0 返回）: ";
    string username;
    cin >> username;

    if (username == "0") {
        return;
    }

    if (users.find(username) == users.end()) {
        cout << "用户不存在！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "请输入新密码: ";
    string newPassword = getHiddenInput();

    if (newPassword.empty()) {
        cout << "密码不能为空！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "请确认新密码: ";
    string confirmPassword = getHiddenInput();

    if (newPassword != confirmPassword) {
        cout << "两次输入的密码不一致！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    users[username] = newPassword;
    saveUsers(users);

    cout << "用户 " << username << " 的密码修改成功！" << endl;
    cout << "按任意键返回..." << endl;
    _getch();
}

void adminInitSystem() {
    cout << "\n=== 初始化系统 ===" << endl;
    cout << "警告：此操作将删除所有用户数据和密码信息！" << endl;
    cout << "确认初始化系统？(输入 Y 确认，其他键取消): ";
    char confirm = _getch();
    cout << confirm << endl;

    if (confirm == 'Y' || confirm == 'y') {
        map<string, string> users = loadUsers();
        for (const auto& pair : users) {
            string userFile = DATA_DIR + pair.first + ".dat";
            ifstream filecheck(userFile);
            if (filecheck.good()) {
                filecheck.close();
                remove(userFile.c_str());
            }
        }

        users.clear();
        saveUsers(users);

        cout << "系统初始化完成！" << endl;
    } else {
        cout << "已取消初始化。" << endl;
    }

    cout << "按任意键返回..." << endl;
    _getch();
}

void showAdminMenu(const string& username) {
    while (true) {
        system("cls");
        cout << "=== 管理员系统 ===" << endl;
        cout << "当前用户: " << username << "（管理员）" << endl;
        cout << "1、批量添加用户" << endl;
        cout << "2、删除用户" << endl;
        cout << "3、查看用户列表" << endl;
        cout << "4、修改用户密码" << endl;
        cout << "5、初始化系统" << endl;
        cout << "6、返回上一级" << endl;
        cout << "请选择（输入序号）: ";

        string choice;
        cin >> choice;

        if (choice == "1") {
            adminAddUsers();
        } else if (choice == "2") {
            adminDeleteUsers();
        } else if (choice == "3") {
            adminViewUsers();
        } else if (choice == "4") {
            adminModifyUserPassword();
        } else if (choice == "5") {
            adminInitSystem();
        } else if (choice == "6") {
            cout << "返回上一级..." << endl;
            break;
        } else {
            cout << "输入无效，请输入1、2、3、4、5或6！" << endl;
            cout << "按任意键继续..." << endl;
            cin.ignore();
            _getch();
        }
    }
}

// ========== 登录菜单 ==========

void showLoginMenu() {
    while (true) {
        system("cls");
        cout << "=== 密码查询系统 ===" << endl;
        cout << "1、登录" << endl;
        cout << "2、注册" << endl;
        cout << "3、退出" << endl;
        cout << "请选择（输入序号）: ";

        string choice;
        cin >> choice;

        if (choice == "1") {
            string loggedInUser;
            bool isAdmin = false;
            if (loginUser(loggedInUser, isAdmin)) {
                if (isAdmin) {
                    showAdminMenu(loggedInUser);
                } else {
                    showUserMenu(loggedInUser);
                }
            } else {
                cout << "按任意键继续..." << endl;
                cin.ignore();
                _getch();
            }
        } else if (choice == "2") {
            registerUser();
            cout << "按任意键继续..." << endl;
            cin.ignore();
            _getch();
        } else if (choice == "3") {
            cout << "感谢使用，再见！" << endl;
            break;
        } else {
            cout << "输入无效，请输入1、2或3！" << endl;
            cout << "按任意键继续..." << endl;
            cin.ignore();
            _getch();
        }
    }
}
