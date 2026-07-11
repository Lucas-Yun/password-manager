#ifndef FILE_IO_H
#define FILE_IO_H

#include <map>
#include <string>
#include "types.h"

/**
 * @brief 文件读写模块
 */

/**
 * @brief 创建目录
 * @param dir 目录路径
 * @return 是否成功
 */
bool createDirectory(const std::string& dir);

/**
 * @brief 加载所有用户
 * @return 用户名 -> 密码 的映射
 */
std::map<std::string, std::string> loadUsers();

/**
 * @brief 保存所有用户
 * @param users 用户名 -> 密码 的映射
 */
void saveUsers(const std::map<std::string, std::string>& users);

/**
 * @brief 加载某用户的所有密码信息
 * @param username 用户名
 * @return 平台名 -> PlatformInfo 的映射
 */
std::map<std::string, PlatformInfo> loadPasswords(const std::string& username);

/**
 * @brief 保存某用户的所有密码信息
 * @param username 用户名
 * @param passwords 平台名 -> PlatformInfo 的映射
 */
void savePasswords(const std::string& username, const std::map<std::string, PlatformInfo>& passwords);

#endif // FILE_IO_H
