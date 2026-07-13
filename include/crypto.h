#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include "types.h"

/**
 * @brief 加密/解密模块
 */

/**
 * @brief XOR 加密/解密
 * @param data 原始数据
 * @return 加密/解密后的数据
 */
std::string xorEncryptDecrypt(const std::string& data);

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
