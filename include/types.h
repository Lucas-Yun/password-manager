#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>
#include <cstdint>

// ========== 全局常量 ==========
const std::string ADMIN_USERNAME = "Admin";
const std::string ADMIN_PASSWORD = "admin_root";
const std::string DATA_DIR = "data\\";
const std::string USER_FILE = "users.dat";

// ========== 加密相关常量 ==========

// 旧版 XOR 加密密钥（仅供转换程序使用）
const char LEGACY_XOR_KEY = 0x5A;

// 新版 XTEA 加密
// XTEA 使用 128 位密钥（4 个 32 位整数）
const uint32_t XTEA_KEY[4] = {0x4D616E31, 0x61676572, 0x53656372, 0x33744B21};
// 上述密钥对应 ASCII: "Man1agerSecr3tK!"
const unsigned int XTEA_ROUNDS = 32;
// XTEA 块大小: 8 字节（2 x uint32_t）
const size_t XTEA_BLOCK_SIZE = 8;
// 文件头标识，用于区分新旧格式
const unsigned char CRYPTO_VERSION = 0x02;  // 新版标识

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
