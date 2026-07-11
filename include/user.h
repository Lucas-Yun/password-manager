#ifndef USER_H
#define USER_H

#include <string>

/**
 * @brief 用户管理模块
 */

/**
 * @brief 用户注册（需管理员密码验证）
 * @return 是否注册成功
 */
bool registerUser();

/**
 * @brief 用户登录
 * @param loggedInUser 输出：登录的用户名
 * @param isAdmin 输出：是否为管理员
 * @return 是否登录成功
 */
bool loginUser(std::string& loggedInUser, bool& isAdmin);

#endif // USER_H
