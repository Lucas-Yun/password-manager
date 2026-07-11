#ifndef MENU_H
#define MENU_H

#include <string>

/**
 * @brief 菜单模块
 */

/**
 * @brief 显示登录/注册菜单
 */
void showLoginMenu();

/**
 * @brief 显示普通用户菜单
 * @param username 当前登录用户名
 */
void showUserMenu(const std::string& username);

/**
 * @brief 显示管理员菜单
 * @param username 当前登录管理员用户名
 */
void showAdminMenu(const std::string& username);

#endif // MENU_H
