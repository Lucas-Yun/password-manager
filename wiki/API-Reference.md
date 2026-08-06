# API 参考

## types.h — 类型与常量

### 常量

```cpp
// 用户相关
const std::string ADMIN_USERNAME = "Admin";
const std::string ADMIN_PASSWORD = "admin_root";
const std::string DATA_DIR = "data\\";
const std::string USER_FILE = "users.dat";

// 旧版加密
const char LEGACY_XOR_KEY = 0x5A;

// 新版 XTEA 加密
const uint32_t XTEA_KEY[4] = {0x4D616E31, 0x61676572, 0x53656372, 0x33744B21};
const unsigned int XTEA_ROUNDS = 32;
const size_t XTEA_BLOCK_SIZE = 8;
const unsigned char CRYPTO_VERSION = 0x02;
```

### 数据结构

```cpp
struct PlatformInfo {
    std::vector<std::string> accounts;  // 账号列表（可多个）
    std::string password;               // 密码（仅一个）
    std::string note;                   // 备注（可选）
    std::string createTime;             // 创建时间（YYYY-MM-DD HH:MM:SS）
};
```

---

## crypto.h — 加密模块

### encryptData

```cpp
std::string encryptData(const std::string& data);
```

加密数据（XTEA-CBC）。

- **输入**：原始明文
- **输出**：`[0x02版本标识] + [8字节IV] + [XTEA-CBC密文]`
- **特性**：每次加密使用随机 IV，相同明文加密结果不同
- **空数据处理**：空字符串返回仅含版本标识的单字节

### decryptData

```cpp
std::string decryptData(const std::string& data);
```

解密数据，自动识别新旧格式。

- **输入**：加密数据（含版本头）
- **输出**：解密后的明文
- **向后兼容**：首字节非 `0x02` 时自动用旧版 XOR 解密

### legacyXorCrypt

```cpp
std::string legacyXorCrypt(const std::string& data);
```

旧版 XOR 加密/解密（对称操作）。

- **输入**：原始数据或加密数据
- **输出**：加密或解密后的数据
- **用途**：仅供转换工具和向后兼容使用

### getHiddenInput

```cpp
std::string getHiddenInput();
```

隐藏输入，密码显示为 `*`。

- **返回**：用户输入的密码字符串
- **特性**：支持退格删除，只接受可打印 ASCII（32-126）

### generateRandomPassword

```cpp
std::string generateRandomPassword(int length = 16);
```

生成随机密码。

- **输入**：密码长度（默认 16，有效范围 4-64，超出范围会被截断）
- **字符集**：大写字母 + 小写字母 + 数字 + 特殊字符 `!@#$%^&*_-+=`
- **特性**：保证至少包含每种字符各一个，生成后随机打乱顺序
- **返回**：随机密码字符串

### checkPasswordStrength

```cpp
int checkPasswordStrength(const std::string& password);
```

检测密码强度。

- **输入**：待检测的密码
- **判定规则**：
  - 长度 < 6 → `0`（弱）
  - 长度 ≥ 6 且字符类型 ≥ 2 → `1`（中）
  - 长度 ≥ 10 且字符类型 ≥ 3 → `2`（强）
  - 长度 ≥ 12 且字符类型 ≥ 3 → `3`（非常强）
- **字符类型**：大写字母、小写字母、数字、其他（特殊字符）
- **返回**：强度等级 `0`=弱, `1`=中, `2`=强, `3`=非常强

### getStrengthText

```cpp
std::string getStrengthText(int level);
```

获取密码强度描述文本。

- **输入**：强度等级（`checkPasswordStrength` 的返回值）
- **返回**：`0`→弱, `1`→中, `2`→强, `3`→非常强, 其他→未知

### copyToClipboard

```cpp
bool copyToClipboard(const std::string& text);
```

复制文本到系统剪贴板（Windows）。

- **输入**：要复制的文本
- **返回**：是否复制成功
- **实现**：通过 `OpenClipboard` / `SetClipboardData(CF_TEXT)` 实现

### getCurrentTime

```cpp
std::string getCurrentTime();
```

获取当前时间字符串。

- **返回**：格式为 `YYYY-MM-DD HH:MM:SS` 的本地时间字符串
- **实现**：基于 `time()` / `localtime()`，使用 `ostringstream` 格式化

---

## file_io.h — 文件读写模块

### createDirectory

```cpp
bool createDirectory(const std::string& dir);
```

创建目录（如已存在则返回 true）。

### loadUsers

```cpp
std::map<std::string, std::string> loadUsers();
```

加载所有用户。

- **返回**：`用户名 -> 密码` 的映射
- **自动升级**：如果文件是旧版格式，加载后自动用新格式覆写

### saveUsers

```cpp
void saveUsers(const std::map<std::string, std::string>& users);
```

保存所有用户。

- **输入**：`用户名 -> 密码` 的映射
- **加密**：数据经 `encryptData()` 加密后写入 `data/users.dat`

### loadPasswords

```cpp
std::map<std::string, PlatformInfo> loadPasswords(const std::string& username);
```

加载指定用户的所有密码信息。

- **输入**：用户名
- **返回**：`平台名 -> PlatformInfo` 的映射
- **自动升级**：如果文件是旧版格式，加载后自动用新格式覆写

### savePasswords

```cpp
void savePasswords(const std::string& username, const std::map<std::string, PlatformInfo>& passwords);
```

保存指定用户的所有密码信息。

- **输入**：用户名，`平台名 -> PlatformInfo` 的映射
- **加密**：数据经 `encryptData()` 加密后写入 `data/用户名.dat`

---

## user.h — 用户管理模块

### registerUser

```cpp
bool registerUser();
```

用户注册（需管理员密码验证）。

- **流程**：验证管理员密码 → 用户名查重 → 输入密码 → 确认密码 → 保存
- **返回**：注册是否成功
- **限制**：用户名不能为空、不能为 `Admin`、不能与已有用户重名

### loginUser

```cpp
bool loginUser(std::string& loggedInUser, bool& isAdmin);
```

用户登录。

- **输出参数**：
  - `loggedInUser`：登录的用户名
  - `isAdmin`：是否为管理员
- **返回**：登录是否成功
- **特性**：支持管理员密码动态读取（修改后立即生效）

---

## menu.h — 菜单模块

### showLoginMenu

```cpp
void showLoginMenu();
```

显示登录/注册主菜单。循环显示直到用户选择退出。

- 选项：1、登录 2、注册 3、退出
- 登录成功后根据用户类型跳转到 `showUserMenu()` 或 `showAdminMenu()`

### showUserMenu

```cpp
void showUserMenu(const std::string& username);
```

显示普通用户菜单。

- **输入**：当前登录用户名
- 选项：1、添加信息 2、查询信息 3、删除信息 4、平台列表 5、编辑信息 6、搜索平台 7、修改密码 8、导出数据 9、随机密码生成器 10、返回上一级

### showAdminMenu

```cpp
void showAdminMenu(const std::string& username);
```

显示管理员菜单。

- **输入**：当前登录管理员用户名
- 选项：1、批量添加用户 2、删除用户 3、查看用户列表 4、修改用户密码 5、修改管理员密码 6、系统统计信息 7、数据备份 8、数据恢复 9、初始化系统 10、返回上一级

---

## 内部辅助函数（menu.cpp）

以下函数在 `menu.cpp` 内部使用，不对外暴露：

| 函数 | 说明 |
|------|------|
| `addInfo(username)` | 添加平台信息（支持随机密码、强度检测、备注、自动创建时间） |
| `queryInfo(username)` | 查询平台密码（显示备注/创建时间，按 C 复制到剪贴板，30 秒自动返回） |
| `deleteInfo(username)` | 删除平台信息 |
| `listAllPlatforms(username)` | 列出所有平台（显示创建时间） |
| `editInfo(username)` | 编辑平台信息（子菜单 6 项：增删账号 / 改密码 / 重命名 / 编辑备注 / 保存） |
| `searchPlatforms(username)` | 模糊搜索平台（详情显示备注/创建时间，按 C 复制到剪贴板） |
| `changeUserPassword(username)` | 修改自己的登录密码（显示密码强度） |
| `exportData(username)` | 导出所有平台信息为明文文本文件 |
| `passwordGenerator()` | 随机密码生成器（8-64 位，可复制 / 重新生成） |
| `adminAddUsers()` | 管理员批量添加用户 |
| `adminDeleteUsers()` | 管理员删除用户 |
| `adminViewUsers()` | 管理员查看用户列表 |
| `adminModifyUserPassword()` | 管理员修改用户密码 |
| `adminChangeOwnPassword()` | 管理员修改自己的密码（显示密码强度） |
| `adminSystemStats()` | 系统统计信息 |
| `adminBackupData()` | 数据备份（将 data 目录备份到 backup_时间戳 目录） |
| `adminRestoreData()` | 数据恢复（从备份目录恢复，覆盖当前数据，带确认） |
| `adminInitSystem()` | 初始化系统 |
| `stringContains(text, pattern)` | 子串搜索 |
| `toLower(s)` | 转小写 |
| `fuzzyMatch(text, pattern)` | 不区分大小写的模糊匹配 |
