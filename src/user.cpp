#include "user.h"
#include "crypto.h"
#include "file_io.h"
#include <iostream>
#include <fstream>

using namespace std;

bool registerUser() {
    cout << "\n=== 用户注册 ===" << endl;
    cout << "请输入管理员密码: ";
    string adminPwd = getHiddenInput();

    if (adminPwd != ADMIN_PASSWORD) {
        cout << "管理员密码错误！" << endl;
        return false;
    }

    cout << "请输入用户名: ";
    string username;
    cin >> username;

    if (username.empty()) {
        cout << "用户名不能为空！" << endl;
        return false;
    }

    if (username == ADMIN_USERNAME) {
        cout << "该用户名是系统保留用户名，不可使用！" << endl;
        return false;
    }

    map<string, string> users = loadUsers();
    if (users.find(username) != users.end()) {
        cout << "用户名已存在，请使用其他用户名！" << endl;
        return false;
    }

    cout << "请输入密码: ";
    string password = getHiddenInput();

    if (password.empty()) {
        cout << "密码不能为空！" << endl;
        return false;
    }

    cout << "请确认密码: ";
    string confirmPwd = getHiddenInput();

    if (password != confirmPwd) {
        cout << "两次输入的密码不一致！" << endl;
        return false;
    }

    users[username] = password;
    saveUsers(users);

    string userFile = DATA_DIR + username + ".dat";
    ofstream file(userFile, ios::binary);
    file.close();

    cout << "注册成功！请重新登录。" << endl;
    return true;
}

bool loginUser(string& loggedInUser, bool& isAdmin) {
    cout << "\n=== 用户登录 ===" << endl;
    cout << "请输入用户名: ";
    string username;
    cin >> username;

    cout << "请输入密码: ";
    string password = getHiddenInput();

    if (username == ADMIN_USERNAME && password == ADMIN_PASSWORD) {
        loggedInUser = ADMIN_USERNAME;
        isAdmin = true;
        cout << "管理员登录成功！" << endl;
        return true;
    }

    map<string, string> users = loadUsers();
    auto it = users.find(username);
    if (it != users.end() && it->second == password) {
        loggedInUser = username;
        isAdmin = false;
        cout << "登录成功！欢迎 " << username << endl;
        return true;
    } else {
        cout << "用户名或密码错误！" << endl;
        return false;
    }
}
