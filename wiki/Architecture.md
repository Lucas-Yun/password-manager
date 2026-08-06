# 架构设计

## 模块划分

系统采用分层模块化设计，共 5 个核心模块 + 2 个工具程序：

```
┌─────────────────────────────────────────────────────┐
│                    main.cpp                         │
│              (程序入口、控制台初始化)                  │
├─────────────────────────────────────────────────────┤
│                  menu 模块                           │
│         (界面展示、业务逻辑、用户交互)                  │
├──────────┬──────────┬───────────┬───────────────────┤
│ user模块  │ file_io  │ crypto模块 │   types.h        │
│(注册/登录)│(文件读写) │(加密/解密)  │ (常量/数据结构)    │
└──────────┴──────────┴───────────┴───────────────────┘
```

### 模块职责

| 模块 | 头文件 | 源文件 | 职责 |
|------|--------|--------|------|
| 类型定义 | `types.h` | - | 全局常量、加密密钥、`PlatformInfo` 结构体 |
| 加密模块 | `crypto.h` | `crypto.cpp` | XTEA-CBC 加解密、PKCS7 填充、密码隐藏输入 |
| 文件模块 | `file_io.h` | `file_io.cpp` | 用户数据与密码数据的加载/保存、自动格式升级 |
| 用户模块 | `user.h` | `user.cpp` | 注册验证、登录验证、管理员密码动态读取 |
| 菜单模块 | `menu.h` | `menu.cpp` | 所有界面展示、业务逻辑、输入处理 |
| 入口 | - | `main.cpp` | 控制台 UTF-8 设置、数据目录创建、首次管理员初始化 |

### 工具程序

| 程序 | 源文件 | 说明 |
|------|--------|------|
| 数据转换工具 | `convert.cpp` | 旧版 XOR 数据批量转换为新版 XTEA-CBC |
| 加密诊断测试 | `crypto_test.cpp` | 40 项加解密一致性验证 |

## 依赖关系

```
main.cpp
  ├── menu.h     → showLoginMenu()
  ├── file_io.h  → createDirectory(), loadUsers(), saveUsers()
  └── types.h    → DATA_DIR, ADMIN_USERNAME, ADMIN_PASSWORD

menu.cpp
  ├── crypto.h   → getHiddenInput()
  ├── file_io.h  → loadPasswords(), savePasswords(), loadUsers(), saveUsers()
  ├── user.h     → (间接依赖)
  └── types.h    → PlatformInfo, ADMIN_USERNAME, ADMIN_PASSWORD

user.cpp
  ├── crypto.h   → getHiddenInput()
  ├── file_io.h  → loadUsers(), saveUsers()
  └── types.h    → ADMIN_USERNAME, ADMIN_PASSWORD, DATA_DIR

file_io.cpp
  ├── crypto.h   → encryptData(), decryptData()
  └── types.h    → DATA_DIR, USER_FILE, CRYPTO_VERSION, PlatformInfo

crypto.cpp
  └── types.h    → XTEA_KEY, XTEA_ROUNDS, XTEA_BLOCK_SIZE, CRYPTO_VERSION, LEGACY_XOR_KEY
```

## 数据流

### 用户注册流程

```
用户输入管理员密码
       │
       ▼
  user.cpp: registerUser()
       │
       ├── getAdminPassword()  ──→ file_io.cpp: loadUsers() ──→ crypto.cpp: decryptData()
       │                                                    ──→ 解析用户列表
       │
       ├── 验证管理员密码
       │
       ├── 用户名查重
       │
       ├── 用户输入密码 (getHiddenInput)
       │
       ├── saveUsers()  ──→ file_io.cpp ──→ crypto.cpp: encryptData() ──→ 写入 users.dat
       │
       └── 创建空的用户数据文件 (用户名.dat)
```

### 用户登录流程

```
用户输入用户名 + 密码
       │
       ▼
  user.cpp: loginUser()
       │
       ├── loadUsers()  ──→ 读取 users.dat ──→ decryptData() ──→ 解析用户列表
       │
       ├── 查找用户名
       │
       ├── 比对密码
       │
       ├── 是 Admin? ──→ showAdminMenu()
       │
       └── 普通用户? ──→ showUserMenu()
```

### 密码查询流程

```
用户输入平台名称
       │
       ▼
  menu.cpp: queryInfo()
       │
       ├── loadPasswords(username) ──→ 读取 用户名.dat
       │                              ├── decryptData()
       │                              └── 解析平台:账号列表:密码
       │
       ├── 查找平台
       │
       ├── 找到? ──→ 显示账号和密码 ──→ 30秒倒计时 / 按任意键返回
       │
       └── 未找到? ──→ "未查询到信息"
```

## 自动格式升级机制

`file_io.cpp` 中的 `loadUsers()` 和 `loadPasswords()` 在读取文件时会检测首字节：

```
读取文件内容
     │
     ├── 首字节 == 0x02 (CRYPTO_VERSION)?
     │     ├── 是 → 新版 XTEA-CBC 解密
     │     └── 否 → 旧版 XOR 解密 (向后兼容)
     │
     ├── 解析明文数据
     │
     └── 如果是旧格式且数据非空?
           ├── 是 → 自动用新格式重新加密并写回文件
           └── 否 → 不操作
```

这确保即使用户没有手动运行转换工具，旧版数据也会在首次读取时自动升级。
