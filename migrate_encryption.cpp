// =========================================================================
//  密码管理器加密格式迁移工具
//  将旧版单字节 XOR 加密的数据文件转换为新版盐值+多字节密钥流加密
// =========================================================================
//  使用方法:
//    1. 将此文件放到 PasswordManager 项目根目录
//    2. 确保 data/ 目录下有旧加密格式的数据文件
//    3. 编译并运行: g++ migrate_encryption.cpp -o migrate_encryption.exe
//    4. 运行前请先备份 data/ 目录下的所有 .dat 文件
//  注意事项:
//    - 运行前务必备份原数据文件！
//    - 旧文件会被备份为 .bak 文件
//    - 转换后的数据文件可直接被新版密码管理器读取
// =========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <direct.h>

using namespace std;

// ========== 配置常量 ==========
const string DATA_DIR = "data\\";
const string USER_FILE = "users.dat";
const string MASTER_KEY = "P@ssw0rdMgr2024!";
const char OLD_ENCRYPT_KEY = 0x5A;

// ========== 旧加密算法（单字节 XOR） ==========
string oldDecrypt(const string& data) {
    string result = data;
    for (size_t i = 0; i < result.size(); i++) {
        result[i] ^= OLD_ENCRYPT_KEY;
    }
    return result;
}

// ========== 新加密算法（盐值+多字节密钥流） ==========
static unsigned char rotateLeft(unsigned char value, int shift) {
    return (value << shift) | (value >> (8 - shift));
}

static string generateSalt() {
    const string chars =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "!@#$%^&*";
    string salt;
    salt.reserve(8);
    for (int i = 0; i < 8; i++) {
        salt += chars[rand() % chars.size()];
    }
    return salt;
}

static string generateKeyStream(const string& salt, size_t length) {
    string keyStream;
    keyStream.reserve(length);
    for (size_t i = 0; i < length; i++) {
        unsigned char byte = MASTER_KEY[i % MASTER_KEY.length()];
        byte ^= salt[i % salt.length()];
        byte = rotateLeft(byte, (i + static_cast<unsigned char>(salt[i % salt.length()])) % 8);
        byte ^= ((i * 31 + static_cast<unsigned char>(salt[(i * 7) % salt.length()])) % 256);
        byte ^= MASTER_KEY[(i * 3 + static_cast<unsigned char>(salt[i % salt.length()])) % MASTER_KEY.length()];
        keyStream.push_back(static_cast<char>(byte));
    }
    return keyStream;
}

string newEncrypt(const string& data) {
    string salt = generateSalt();
    string keyStream = generateKeyStream(salt, data.length());
    string encrypted;
    encrypted.reserve(data.length());
    for (size_t i = 0; i < data.length(); i++) {
        encrypted.push_back(data[i] ^ keyStream[i]);
    }
    return salt + encrypted;
}

// ========== 工具函数 ==========
bool fileExists(const string& filepath) {
    ifstream file(filepath, ios::binary);
    return file.good();
}

string readFileBinary(const string& filepath) {
    ifstream file(filepath, ios::binary);
    if (!file.is_open()) {
        return "";
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return content;
}

bool writeFileBinary(const string& filepath, const string& content) {
    ofstream file(filepath, ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.write(content.c_str(), content.size());
    file.close();
    return true;
}

bool backupFile(const string& filepath) {
    string bakPath = filepath + ".bak";
    string content = readFileBinary(filepath);
    if (content.empty()) return false;
    return writeFileBinary(bakPath, content);
}

// ========== 从用户数据文件中提取用户名列表 ==========
vector<string> extractUsernames(const string& userData) {
    vector<string> users;
    size_t pos = 0;
    while (pos < userData.size()) {
        size_t newlinePos = userData.find('\n', pos);
        if (newlinePos == string::npos) break;
        string line = userData.substr(pos, newlinePos - pos);
        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            string username = line.substr(0, colonPos);
            if (!username.empty() && username != "Admin") {
                users.push_back(username);
            }
        }
        pos = newlinePos + 1;
    }
    return users;
}

// ========== 验证数据格式合法性 ==========
bool validateUserData(const string& data) {
    if (data.empty()) return false;
    int colonCount = 0;
    for (char c : data) {
        if (c == ':') colonCount++;
    }
    return colonCount > 0;
}

bool validatePasswordData(const string& data) {
    if (data.empty()) return true;  // 空文件是合法的（新建用户）
    int colonCount = 0;
    for (char c : data) {
        if (c == ':') colonCount++;
    }
    return colonCount > 0;
}

// ========== 转换单个文件 ==========
bool convertFile(const string& filepath, const string& fileDesc) {
    cout << "  正在转换: " << fileDesc << " (" << filepath << ")" << endl;

    if (!fileExists(filepath)) {
        cout << "    [跳过] 文件不存在" << endl;
        return true;
    }

    // 读取旧文件
    string oldData = readFileBinary(filepath);
    if (oldData.empty()) {
        cout << "    [跳过] 文件为空" << endl;
        return true;
    }

    // 用旧算法解密
    string plaintext = oldDecrypt(oldData);

    // 验证解密后数据是否合理
    bool valid = false;
    if (fileDesc.find("用户") != string::npos) {
        valid = validateUserData(plaintext);
    } else {
        valid = validatePasswordData(plaintext);
    }

    if (!valid) {
        cout << "    [警告] 解密后数据格式异常，可能已经是新格式或文件已损坏" << endl;
        cout << "    [跳过] 不进行转换" << endl;
        return true;
    }

    // 备份旧文件
    if (!backupFile(filepath)) {
        cout << "    [错误] 备份旧文件失败，终止转换" << endl;
        return false;
    }
    cout << "    已备份原文件为: " << filepath << ".bak" << endl;

    // 用新算法加密
    string newData = newEncrypt(plaintext);

    // 写入新文件
    if (!writeFileBinary(filepath, newData)) {
        cout << "    [错误] 写入新文件失败" << endl;
        return false;
    }

    cout << "    [成功] 转换完成 (" << oldData.size() << " 字节 → " << newData.size() << " 字节)" << endl;
    return true;
}

// ========== 主流程 ==========
int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    cout << "================================================================" << endl;
    cout << "  密码管理器加密格式迁移工具" << endl;
    cout << "  旧格式: 单字节 XOR  →  新格式: 盐值+多字节密钥流" << endl;
    cout << "================================================================" << endl;
    cout << endl;
    cout << "  注意事项:" << endl;
    cout << "    1. 转换前请务必备份 data/ 目录下的所有 .dat 文件" << endl;
    cout << "    2. 本工具会自动为每个旧文件创建 .bak 备份" << endl;
    cout << "    3. 如果数据已经是新格式，将自动跳过" << endl;
    cout << endl;

    string userFile = DATA_DIR + USER_FILE;

    if (!fileExists(userFile)) {
        cout << "[错误] 未找到用户数据文件: " << userFile << endl;
        cout << "请确保 data/ 目录存在且包含旧格式的 users.dat 文件。" << endl;
        cout << endl << "按任意键退出..." << endl;
        cin.get();
        return 1;
    }

    // 确认提示
    cout << "是否开始转换？(输入 Y 确认，其他键取消): ";
    char confirm;
    cin.get(confirm);
    cin.ignore();

    if (confirm != 'Y' && confirm != 'y') {
        cout << "已取消转换。" << endl;
        cout << endl << "按任意键退出..." << endl;
        cin.get();
        return 0;
    }

    cout << endl << "开始转换..." << endl;
    cout << "--------------------------------" << endl;

    int successCount = 0;
    int failCount = 0;

    // 1. 转换用户数据文件
    if (convertFile(userFile, "用户数据")) {
        successCount++;
    } else {
        failCount++;
    }

    // 2. 读取用户列表，逐个转换用户密码文件
    string userData = readFileBinary(userFile + ".bak");  // 从备份读取（刚才已备份）
    if (userData.empty()) {
        // 如果备份为空，说明用户文件为空或刚才跳过了备份
        userData = readFileBinary(userFile);
        string plaintext;
        if (!userData.empty() && userData.length() >= 8) {
            // 已经转换过了，用新解密方式尝试获取用户列表
            string salt = userData.substr(0, 8);
            string encrypted = userData.substr(8);
            string keyStream = generateKeyStream(salt, encrypted.length());
            for (size_t i = 0; i < encrypted.length(); i++) {
                plaintext.push_back(encrypted[i] ^ keyStream[i]);
            }
        } else {
            plaintext = oldDecrypt(userData);
        }
        userData = plaintext;
    } else {
        // 从备份读取旧数据并解密
        userData = oldDecrypt(userData);
    }

    vector<string> users = extractUsernames(userData);

    if (!users.empty()) {
        cout << endl << "  检测到 " << users.size() << " 个用户，开始转换用户密码文件..." << endl;
        for (const string& user : users) {
            string userPwdFile = DATA_DIR + user + ".dat";
            if (convertFile(userPwdFile, "用户 [" + user + "] 密码数据")) {
                successCount++;
            } else {
                failCount++;
            }
        }
    }

    // 汇总结果
    cout << endl << "--------------------------------" << endl;
    cout << "转换完成！" << endl;
    cout << "  成功: " << successCount << " 个文件" << endl;
    cout << "  失败: " << failCount << " 个文件" << endl;

    if (failCount == 0) {
        cout << endl << "所有数据文件已成功转换为新加密格式。" << endl;
        cout << "原始文件备份为 .bak，确认无误后可自行删除。" << endl;
    } else {
        cout << endl << "部分文件转换失败，请检查后重试。" << endl;
    }

    cout << endl << "按任意键退出..." << endl;
    cin.get();
    return failCount == 0 ? 0 : 1;
}
