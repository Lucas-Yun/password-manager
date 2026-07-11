#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

// ========== 全局常量 ==========
const std::string ADMIN_USERNAME = "Admin";
const std::string ADMIN_PASSWORD = "admin_root";
const std::string DATA_DIR = "data\\";
const std::string USER_FILE = "users.dat";
const char ENCRYPT_KEY = 0x5A;

// ========== 数据结构 ==========

/**
 * @brief 平台信息结构体
 * 存储某个平台下的多个账号和一个密码
 */
struct PlatformInfo {
    std::vector<std::string> accounts;  // 账号列表（可多个）
    std::string password;               // 密码（仅一个）
};

#endif // TYPES_H
