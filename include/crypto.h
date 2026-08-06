#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include "types.h"

/**
 * @brief 加密/解密模块
 * 采用 XTEA 块加密算法 + CBC 模式 + PKCS7 填充
 */

/**
 * @brief 加密（XTEA-CBC）
 * 文件格式: [1字节版本标识] + [8字节IV] + [密文]
 * @param data 原始明文
 * @return 加密后的数据（含版本头和IV）
 */
std::string encryptData(const std::string& data);

/**
 * @brief 解密（XTEA-CBC）
 * @param data 加密数据（含版本头和IV）
 * @return 解密后的明文
 */
std::string decryptData(const std::string& data);

/**
 * @brief 隐藏输入（密码显示为 *）
 * @return 用户输入的密码
 */
std::string getHiddenInput();

/**
 * @brief 生成随机密码
 * @param length 密码长度（默认16）
 * @return 随机密码字符串
 */
std::string generateRandomPassword(int length = 16);

/**
 * @brief 检测密码强度
 * @param password 待检测的密码
 * @return 强度等级: 0=弱, 1=中, 2=强, 3=非常强
 */
int checkPasswordStrength(const std::string& password);

/**
 * @brief 获取密码强度描述文本
 * @param level 强度等级
 * @return 描述文本
 */
std::string getStrengthText(int level);

/**
 * @brief 复制文本到剪贴板
 * @param text 要复制的文本
 * @return 是否成功
 */
bool copyToClipboard(const std::string& text);

/**
 * @brief 获取当前时间字符串
 * @return 格式: YYYY-MM-DD HH:MM:SS
 */
std::string getCurrentTime();

#endif // CRYPTO_H
