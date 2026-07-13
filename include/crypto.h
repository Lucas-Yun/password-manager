#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include "types.h"

/**
 * @brief 加密/解密模块
 */

/**
 * @brief 加密数据（使用盐值+多字节密钥流）
 * @param data 原始数据
 * @return 加密后的数据（前8字节为盐值）
 */
std::string encryptData(const std::string& data);

/**
 * @brief 解密数据
 * @param data 加密后的数据（前8字节为盐值）
 * @return 原始数据
 */
std::string decryptData(const std::string& data);

/**
 * @brief 隐藏输入（密码显示为 *）
 * @return 用户输入的密码
 */
std::string getHiddenInput();

/**
 * @brief 检测密码强度
 * @param password 密码
 * @return 强度等级：弱/中/强
 */
std::string checkPasswordStrength(const std::string& password);

/**
 * @brief 生成随机强密码
 * @param length 密码长度
 * @return 生成的密码
 */
std::string generateRandomPassword(int length = 16);

#endif // CRYPTO_H
