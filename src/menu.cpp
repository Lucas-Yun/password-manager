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
#include <sstream>

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

    cout << "请输入密码（或输入 R 自动生成随机密码）: ";
    string password = getHiddenInput();

    if (password == "R" || password == "r") {
        password = generateRandomPassword(16);
        cout << "已生成随机密码: " << password << endl;
    }

    if (password.empty()) {
        cout << "密码不能为空！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    // 显示密码强度
    int strength = checkPasswordStrength(password);
    cout << "密码强度: " << getStrengthText(strength) << endl;

    // 输入备注（可选）
    cout << "请输入备注（可选，直接回车跳过）: ";
    string note;
    getline(cin, note);

    PlatformInfo info;
    info.accounts = accounts;
    info.password = password;
    info.note = note;
    info.createTime = getCurrentTime();
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
    if (!it->second.note.empty()) {
        cout << "备注: " << it->second.note << endl;
    }
    if (!it->second.createTime.empty()) {
        cout << "创建时间: " << it->second.createTime << endl;
    }
    cout << "--------------------------------" << endl;
    cout << "按 C 复制密码到剪贴板，其他键返回（30秒后自动返回）" << endl;

    time_t startTime = time(nullptr);
    while (true) {
        if (_kbhit()) {
            char key = _getch();
            if (key == 'C' || key == 'c') {
                if (copyToClipboard(it->second.password)) {
                    cout << "密码已复制到剪贴板！" << endl;
                } else {
                    cout << "复制失败！" << endl;
                }
                cout << "按任意键返回..." << endl;
                _getch();
            }
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
            cout << index << ". [" << pair.first << "]   账号数: " << accCount;
            if (!pair.second.createTime.empty()) {
                cout << "   创建: " << pair.second.createTime;
            }
            cout << endl;
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
        if (!it->second.note.empty()) {
            cout << "当前备注: " << it->second.note << endl;
        }
        if (!it->second.createTime.empty()) {
            cout << "创建时间: " << it->second.createTime << endl;
        }
        cout << "--------------------------------" << endl;
        cout << "1、添加账号" << endl;
        cout << "2、删除账号" << endl;
        cout << "3、修改密码" << endl;
        cout << "4、重命名平台" << endl;
        cout << "5、编辑备注" << endl;
        cout << "6、保存并返回" << endl;
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
            cout << "请输入新密码（或输入 R 自动生成随机密码）: ";
            string newPwd = getHiddenInput();
            if (newPwd == "R" || newPwd == "r") {
                newPwd = generateRandomPassword(16);
                cout << "已生成随机密码: " << newPwd << endl;
            }
            if (!newPwd.empty()) {
                int strength = checkPasswordStrength(newPwd);
                cout << "密码强度: " << getStrengthText(strength) << endl;
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
            cout << "当前备注: " << (it->second.note.empty() ? "（无）" : it->second.note) << endl;
            cout << "请输入新备注（直接回车清除备注）: ";
            string newNote;
            getline(cin, newNote);
            it->second.note = newNote;
            cout << "备注已更新！" << endl;
            cout << "按任意键继续..." << endl;
            _getch();
        } else if (choice == "6") {
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
                if (!item.second.note.empty()) {
                    cout << "备注: " << item.second.note << endl;
                }
                if (!item.second.createTime.empty()) {
                    cout << "创建时间: " << item.second.createTime << endl;
                }
                cout << "--------------------------------" << endl;
                cout << "按 C 复制密码到剪贴板，其他键返回（30秒后自动返回）" << endl;

                time_t startTime = time(nullptr);
                while (true) {
                    if (_kbhit()) {
                        char key = _getch();
                        if (key == 'C' || key == 'c') {
                            if (copyToClipboard(item.second.password)) {
                                cout << "密码已复制到剪贴板！" << endl;
                            } else {
                                cout << "复制失败！" << endl;
                            }
                            cout << "按任意键返回..." << endl;
                            _getch();
                        }
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

    int strength = checkPasswordStrength(newPwd);
    cout << "密码强度: " << getStrengthText(strength) << endl;

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

void exportData(const string& username) {
    cout << "\n=== 导出数据 ===" << endl;
    cin.ignore();

    map<string, PlatformInfo> passwords = loadPasswords(username);
    if (passwords.empty()) {
        cout << "当前没有任何平台信息可导出！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "请输入导出文件路径（直接回车使用默认 export_<用户名>.txt）: ";
    string filepath;
    getline(cin, filepath);
    if (filepath.empty()) {
        filepath = "export_" + username + ".txt";
    }

    ofstream outfile(filepath);
    if (!outfile.is_open()) {
        cout << "无法创建文件！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    outfile << "========== 密码导出 ==========" << endl;
    outfile << "用户: " << username << endl;
    outfile << "导出时间: " << getCurrentTime() << endl;
    outfile << "平台总数: " << passwords.size() << endl;
    outfile << "==============================" << endl << endl;

    for (const auto& pair : passwords) {
        outfile << "【平台】" << pair.first << endl;
        outfile << "  账号列表:" << endl;
        for (size_t i = 0; i < pair.second.accounts.size(); i++) {
            outfile << "    " << (i + 1) << ". " << pair.second.accounts[i] << endl;
        }
        outfile << "  密码: " << pair.second.password << endl;
        if (!pair.second.note.empty()) {
            outfile << "  备注: " << pair.second.note << endl;
        }
        if (!pair.second.createTime.empty()) {
            outfile << "  创建时间: " << pair.second.createTime << endl;
        }
        outfile << endl;
    }

    outfile.close();
    cout << "数据已导出到: " << filepath << endl;
    cout << "警告: 导出文件为明文，请注意安全！" << endl;
    cout << "按任意键返回..." << endl;
    _getch();
}

void passwordGenerator() {
    cout << "\n=== 随机密码生成器 ===" << endl;

    cout << "请输入密码长度（8-64，默认16）: ";
    string lenStr;
    cin.ignore();
    getline(cin, lenStr);

    int len = 16;
    if (!lenStr.empty()) {
        len = atoi(lenStr.c_str());
        if (len < 8) len = 8;
        if (len > 64) len = 64;
    }

    string pwd = generateRandomPassword(len);
    int strength = checkPasswordStrength(pwd);

    cout << "\n生成的密码: " << pwd << endl;
    cout << "密码长度: " << pwd.size() << endl;
    cout << "密码强度: " << getStrengthText(strength) << endl;
    cout << "--------------------------------" << endl;
    cout << "按 C 复制到剪贴板，R 重新生成，其他键返回: ";

    while (true) {
        char key = _getch();
        if (key == 'C' || key == 'c') {
            if (copyToClipboard(pwd)) {
                cout << "\n密码已复制到剪贴板！" << endl;
            } else {
                cout << "\n复制失败！" << endl;
            }
            break;
        } else if (key == 'R' || key == 'r') {
            pwd = generateRandomPassword(len);
            strength = checkPasswordStrength(pwd);
            cout << "\n重新生成: " << pwd << endl;
            cout << "密码强度: " << getStrengthText(strength) << endl;
            cout << "按 C 复制到剪贴板，R 重新生成，其他键返回: ";
        } else {
            break;
        }
    }
    cout << endl;
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
        cout << "8、导出数据" << endl;
        cout << "9、随机密码生成器" << endl;
        cout << "10、返回上一级" << endl;
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
            exportData(username);
        } else if (choice == "9") {
            passwordGenerator();
        } else if (choice == "10") {
            cout << "返回上一级..." << endl;
            break;
        } else {
            cout << "输入无效，请输入1-10之间的数字！" << endl;
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

    int strength = checkPasswordStrength(newPwd);
    cout << "密码强度: " << getStrengthText(strength) << endl;

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

void adminBackupData() {
    cout << "\n=== 数据备份 ===" << endl;

    // 创建备份目录名（带时间戳）
    string timestamp = getCurrentTime();
    for (size_t i = 0; i < timestamp.size(); i++) {
        if (timestamp[i] == ':' || timestamp[i] == ' ' || timestamp[i] == '-') {
            timestamp[i] = '_';
        }
    }
    string backupDir = "backup_" + timestamp;

    if (!createDirectory(backupDir)) {
        cout << "无法创建备份目录！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    // 复制 data 目录下所有文件
    string searchData = DATA_DIR + "*";
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchData.c_str(), &findData);
    int fileCount = 0;

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                string srcFile = DATA_DIR + findData.cFileName;
                string dstFile = backupDir + "\\" + findData.cFileName;
                if (CopyFile(srcFile.c_str(), dstFile.c_str(), FALSE)) {
                    fileCount++;
                }
            }
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }

    cout << "备份完成！" << endl;
    cout << "备份目录: " << backupDir << endl;
    cout << "备份文件数: " << fileCount << endl;
    cout << "按任意键返回..." << endl;
    _getch();
}

void adminRestoreData() {
    cout << "\n=== 数据恢复 ===" << endl;
    cin.ignore();

    cout << "请输入备份目录路径（例如 backup_2026_08_06_12_00_00）: ";
    string backupDir;
    getline(cin, backupDir);

    if (backupDir.empty()) {
        cout << "未输入路径！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    // 去除末尾的反斜杠
    if (backupDir[backupDir.size() - 1] == '\\') {
        backupDir = backupDir.substr(0, backupDir.size() - 1);
    }

    string searchPattern = backupDir + "\\*";
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPattern.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        cout << "备份目录不存在或为空！" << endl;
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    cout << "警告：此操作将覆盖当前所有数据！" << endl;
    cout << "确认恢复？(输入 Y 确认，其他键取消): ";
    char confirm = _getch();
    cout << confirm << endl;

    if (confirm != 'Y' && confirm != 'y') {
        cout << "已取消恢复。" << endl;
        FindClose(hFind);
        cout << "按任意键返回..." << endl;
        _getch();
        return;
    }

    int fileCount = 0;
    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            string srcFile = backupDir + "\\" + findData.cFileName;
            string dstFile = DATA_DIR + findData.cFileName;
            if (CopyFile(srcFile.c_str(), dstFile.c_str(), FALSE)) {
                fileCount++;
            }
        }
    } while (FindNextFile(hFind, &findData));
    FindClose(hFind);

    cout << "恢复完成！恢复文件数: " << fileCount << endl;
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
        cout << "7、数据备份" << endl;
        cout << "8、数据恢复" << endl;
        cout << "9、初始化系统" << endl;
        cout << "10、返回上一级" << endl;
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
            adminBackupData();
        } else if (choice == "8") {
            adminRestoreData();
        } else if (choice == "9") {
            adminInitSystem();
        } else if (choice == "10") {
            cout << "返回上一级..." << endl;
            break;
        } else {
            cout << "输入无效，请输入1-10之间的数字！" << endl;
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
