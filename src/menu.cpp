#include "menu.h"
#include "crypto.h"
#include "file_io.h"
#include "user.h"
#include "types.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <conio.h>
#include <windows.h>
#include <ctime>

using namespace std;

// ========== 辅助函数 ==========

bool stringContains(const string& text, const string& pattern) {
    if (pattern.empty()) return true;
    return search(text.begin(), text.end(), pattern.begin(), pattern.end()) != text.end();
}

string toLower(string s) {
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            s[i] = s[i] - 'A' + 'a';
        }
    }
    return s;
}

bool fuzzyMatch(const string& text, const string& pattern) {
    if (pattern.empty()) return true;
    return stringContains(toLower(text), toLower(pattern));
}

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

    map<string, PlatformInfo> passwords = loadPasswords(username);
    if (passwords.find(platform) != passwords.end()) {
        cout << "该平台已存在！是否覆盖？(输入 Y 确认覆盖，其他键取消): ";
        char confirm = _getch();
        cout << confirm << endl;
        if (confirm != 'Y' && confirm != 'y') {
            cout << "已取消添加。" << endl;
            cout << "按任意键返回..." << endl;
            _getch();
            return;
        }
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

    cout << "请输入密码: ";
    string password = getHiddenInput();

    if (password.empty()) {
        cout << "密码不能为空！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

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

    map<string, PlatformInfo> passwords = loadPasswords(username);
    if (passwords.empty()) {
        cout << "当前没有任何平台信息！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "当前已保存的平台列表:" << endl;
    int idx = 1;
    for (const auto& pair : passwords) {
        cout << idx << ". " << pair.first << endl;
        idx++;
    }

    cout << "\n请输入要删除的平台名称（或输入 0 返回）: ";
    string platform;
    getline(cin, platform);

    if (platform == "0") {
        return;
    }

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

void listAllPlatforms(const string& username) {
    cout << "\n=== 平台列表 ===" << endl;
    map<string, PlatformInfo> passwords = loadPasswords(username);

    if (passwords.empty()) {
        cout << "当前没有任何平台信息。" << endl;
    } else {
        cout << "共保存了 " << passwords.size() << " 个平台:" << endl;
        cout << "----------------------------------------" << endl;
        int index = 1;
        int totalAccounts = 0;
        for (const auto& pair : passwords) {
            size_t accCount = pair.second.accounts.size();
            totalAccounts += (int)accCount;
            cout << index << ". [" << pair.first << "]   账号数: " << accCount << endl;
            index++;
        }
        cout << "----------------------------------------" << endl;
        cout << "账号总数: " << totalAccounts << " 个" << endl;
    }

    cout << "按任意键返回..." << endl;
    _getch();
}

void editInfo(const string& username) {
    cout << "\n=== 编辑信息 ===" << endl;
    cin.ignore();

    map<string, PlatformInfo> passwords = loadPasswords(username);
    if (passwords.empty()) {
        cout << "当前没有任何平台信息！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "当前已保存的平台列表:" << endl;
    int idx = 1;
    for (const auto& pair : passwords) {
        cout << idx << ". " << pair.first << endl;
        idx++;
    }

    cout << "\n请输入要编辑的平台名称（或输入 0 返回）: ";
    string platform;
    getline(cin, platform);

    if (platform == "0") {
        return;
    }

    auto it = passwords.find(platform);
    if (it == passwords.end()) {
        cout << "未找到该平台的信息！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    while (true) {
        system("cls");
        cout << "=== 编辑平台 [" << platform << "] ===" << endl;
        cout << "当前账号:" << endl;
        for (size_t i = 0; i < it->second.accounts.size(); i++) {
            cout << "  " << (i + 1) << ". " << it->second.accounts[i] << endl;
        }
        cout << "当前密码: ********" << endl;
        cout << "--------------------------------" << endl;
        cout << "1、添加账号" << endl;
        cout << "2、删除账号" << endl;
        cout << "3、修改密码" << endl;
        cout << "4、重命名平台" << endl;
        cout << "5、保存并返回" << endl;
        cout << "请选择: ";

        string choice;
        cin >> choice;
        cin.ignore();

        if (choice == "1") {
            cout << "请输入要添加的账号（直接回车取消）: ";
            string newAcc;
            getline(cin, newAcc);
            if (!newAcc.empty()) {
                it->second.accounts.push_back(newAcc);
                cout << "账号 " << newAcc << " 已添加！" << endl;
                cout << "按任意键继续..." << endl;
                _getch();
            }
        } else if (choice == "2") {
            if (it->second.accounts.empty()) {
                cout << "当前没有可删除的账号！" << endl;
            } else {
                cout << "请输入要删除的账号序号（1-" << it->second.accounts.size() << "，或输入 0 取消）: ";
                string idxStr;
                getline(cin, idxStr);
                if (idxStr != "0") {
                    int delIdx = atoi(idxStr.c_str());
                    if (delIdx >= 1 && delIdx <= (int)it->second.accounts.size()) {
                        string removed = it->second.accounts[delIdx - 1];
                        it->second.accounts.erase(it->second.accounts.begin() + delIdx - 1);
                        cout << "账号 " << removed << " 已删除！" << endl;
                    } else {
                        cout << "无效序号！" << endl;
                    }
                }
            }
            cout << "按任意键继续..." << endl;
            _getch();
        } else if (choice == "3") {
            cout << "请输入新密码: ";
            string newPwd = getHiddenInput();
            if (!newPwd.empty()) {
                cout << "请确认新密码: ";
                string confirmPwd = getHiddenInput();
                if (newPwd == confirmPwd) {
                    it->second.password = newPwd;
                    cout << "密码修改成功！" << endl;
                } else {
                    cout << "两次密码不一致，密码未修改。" << endl;
                }
            } else {
                cout << "密码不能为空！" << endl;
            }
            cout << "按任意键继续..." << endl;
            _getch();
        } else if (choice == "4") {
            cout << "请输入新的平台名称（直接回车取消）: ";
            string newName;
            getline(cin, newName);
            if (!newName.empty()) {
                if (newName != platform && passwords.find(newName) != passwords.end()) {
                    cout << "该名称已存在，不能重命名！" << endl;
                } else if (newName != platform) {
                    PlatformInfo info = it->second;
                    passwords.erase(it);
                    passwords[newName] = info;
                    platform = newName;
                    it = passwords.find(platform);
                    cout << "平台已重命名为 [" << newName << "]！" << endl;
                }
            }
            cout << "按任意键继续..." << endl;
            _getch();
        } else if (choice == "5") {
            if (it->second.accounts.empty()) {
                cout << "至少需要保留一个账号！请先添加账号。" << endl;
                cout << "按任意键继续..." << endl;
                _getch();
            } else {
                savePasswords(username, passwords);
                cout << "修改已保存！" << endl;
                break;
            }
        } else {
            cout << "输入无效！" << endl;
            cout << "按任意键继续..." << endl;
            _getch();
        }
    }

    cout << "按任意键返回..." << endl;
    _getch();
}

void searchPlatforms(const string& username) {
    cout << "\n=== 搜索平台 ===" << endl;
    cin.ignore();

    cout << "请输入搜索关键词（支持模糊搜索）: ";
    string keyword;
    getline(cin, keyword);

    map<string, PlatformInfo> passwords = loadPasswords(username);
    vector<pair<string, PlatformInfo>> results;

    for (const auto& pair : passwords) {
        bool match = fuzzyMatch(pair.first, keyword);
        if (!match) {
            for (const auto& acc : pair.second.accounts) {
                if (fuzzyMatch(acc, keyword)) {
                    match = true;
                    break;
                }
            }
        }
        if (match) {
            results.push_back(pair);
        }
    }

    if (results.empty()) {
        cout << "未找到匹配的平台信息。" << endl;
    } else {
        cout << "找到 " << results.size() << " 个匹配结果:" << endl;
        cout << "----------------------------------------" << endl;
        for (size_t i = 0; i < results.size(); i++) {
            cout << (i + 1) << ". [" << results[i].first << "]" << endl;
            cout << "   账号: ";
            for (size_t j = 0; j < results[i].second.accounts.size(); j++) {
                if (j > 0) cout << ", ";
                cout << results[i].second.accounts[j];
            }
            cout << endl;
        }
        cout << "----------------------------------------" << endl;

        if (!results.empty()) {
            cout << "输入序号查看详细信息（0 返回）: ";
            string idxStr;
            getline(cin, idxStr);
            int viewIdx = atoi(idxStr.c_str());
            if (viewIdx >= 1 && viewIdx <= (int)results.size()) {
                const auto& item = results[viewIdx - 1];
                cout << "\n=== 详细信息 ===" << endl;
                cout << "平台: " << item.first << endl;
                cout << "账号列表:" << endl;
                for (size_t i = 0; i < item.second.accounts.size(); i++) {
                    cout << "  " << (i + 1) << ". " << item.second.accounts[i] << endl;
                }
                cout << "密码: " << item.second.password << endl;
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
                return;
            }
        }
    }

    cout << "按任意键返回..." << endl;
    _getch();
}

void changeUserPassword(const string& username) {
    cout << "\n=== 修改登录密码 ===" << endl;

    cout << "请输入当前密码: ";
    string currentPwd = getHiddenInput();

    map<string, string> users = loadUsers();
    if (users[username] != currentPwd) {
        cout << "当前密码错误！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "请输入新密码: ";
    string newPwd = getHiddenInput();

    if (newPwd.empty()) {
        cout << "密码不能为空！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "请确认新密码: ";
    string confirmPwd = getHiddenInput();

    if (newPwd != confirmPwd) {
        cout << "两次输入的密码不一致！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    if (newPwd == currentPwd) {
        cout << "新密码与原密码相同，无需修改。" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    users[username] = newPwd;
    saveUsers(users);

    cout << "密码修改成功！" << endl;
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
        cout << "4、平台列表" << endl;
        cout << "5、编辑信息" << endl;
        cout << "6、搜索平台" << endl;
        cout << "7、修改密码" << endl;
        cout << "8、返回上一级" << endl;
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
            listAllPlatforms(username);
        } else if (choice == "5") {
            editInfo(username);
        } else if (choice == "6") {
            searchPlatforms(username);
        } else if (choice == "7") {
            changeUserPassword(username);
        } else if (choice == "8") {
            cout << "返回上一级..." << endl;
            break;
        } else {
            cout << "输入无效，请输入1-8之间的数字！" << endl;
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
        cout << "按任意键返回..." << endl;
        _getch();
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
            map<string, PlatformInfo> pwd = loadPasswords(pair.first);
            cout << index << ". " << pair.first << "   平台数: " << pwd.size() << endl;
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

void adminChangeOwnPassword() {
    cout << "\n=== 修改管理员密码 ===" << endl;

    cout << "请输入当前密码: ";
    string currentPwd = getHiddenInput();

    if (currentPwd != ADMIN_PASSWORD) {
        cout << "当前密码错误！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "请输入新密码: ";
    string newPwd = getHiddenInput();

    if (newPwd.empty()) {
        cout << "密码不能为空！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "请确认新密码: ";
    string confirmPwd = getHiddenInput();

    if (newPwd != confirmPwd) {
        cout << "两次输入的密码不一致！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    if (newPwd == currentPwd) {
        cout << "新密码与原密码相同，无需修改。" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    map<string, string> users = loadUsers();
    users[ADMIN_USERNAME] = newPwd;
    saveUsers(users);

    cout << "管理员密码修改成功！下次登录请使用新密码。" << endl;
    cout << "按任意键返回..." << endl;
    _getch();
}

void adminSystemStats() {
    cout << "\n=== 系统统计信息 ===" << endl;
    cout << "----------------------------------------" << endl;

    map<string, string> users = loadUsers();
    int userCount = users.size();
    int totalPlatforms = 0;
    int totalAccounts = 0;

    for (const auto& pair : users) {
        map<string, PlatformInfo> pwd = loadPasswords(pair.first);
        totalPlatforms += (int)pwd.size();
        for (const auto& p : pwd) {
            totalAccounts += (int)p.second.accounts.size();
        }
    }

    cout << "普通用户总数: " << userCount << endl;
    cout << "平台总数:     " << totalPlatforms << endl;
    cout << "账号总数:     " << totalAccounts << endl;

    if (userCount > 0) {
        cout << "----------------------------------------" << endl;
        cout << "各用户详情:" << endl;
        int idx = 1;
        for (const auto& pair : users) {
            map<string, PlatformInfo> pwd = loadPasswords(pair.first);
            int platCount = (int)pwd.size();
            int accCount = 0;
            for (const auto& p : pwd) {
                accCount += (int)p.second.accounts.size();
            }
            cout << idx << ". " << pair.first << "  -  平台: " << platCount << "  账号: " << accCount << endl;
            idx++;
        }
    }

    cout << "----------------------------------------" << endl;
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
        cout << "5、修改管理员密码" << endl;
        cout << "6、系统统计信息" << endl;
        cout << "7、初始化系统" << endl;
        cout << "8、返回上一级" << endl;
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
            adminChangeOwnPassword();
        } else if (choice == "6") {
            adminSystemStats();
        } else if (choice == "7") {
            adminInitSystem();
        } else if (choice == "8") {
            cout << "返回上一级..." << endl;
            break;
        } else {
            cout << "输入无效，请输入1-8之间的数字！" << endl;
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
