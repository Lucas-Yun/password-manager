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

#endif // CRYPTO_H
