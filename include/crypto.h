#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include "types.h"

/**
 * @brief 加密/解密模块
 * 采用 XTEA 块加密算法 + CBC 模式 + PKCS7 填充
 */

/**
 * @brief 新版加密（XTEA-CBC）
 * 文件格式: [1字节版本标识] + [8字节IV] + [密文]
 * @param data 原始明文
 * @return 加密后的数据（含版本头和IV）
 */
std::string encryptData(const std::string& data);

/**
 * @brief 新版解密（XTEA-CBC）
 * 自动识别版本标识，支持新版格式
 * @param data 加密数据（含版本头和IV）
 * @return 解密后的明文
 */
std::string decryptData(const std::string& data);

/**
 * @brief 旧版 XOR 加密/解密（仅供转换程序使用）
 * @param data 原始数据
 * @return 加密/解密后的数据
 */
std::string legacyXorCrypt(const std::string& data);

/**
 * @brief 隐藏输入（密码显示为 *）
 * @return 用户输入的密码
 */
std::string getHiddenInput();

#endif // CRYPTO_H
