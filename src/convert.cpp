/**
 * 数据格式转换工具
 * 将旧版 XOR 加密的 data 文件转换为新版 XTEA-CBC 加密格式
 *
 * 核心安全设计：
 *   本程序直接复用主程序的 crypto 模块（encryptData / decryptData / legacyXorCrypt），
 *   与主程序共享同一套加密算法实现，彻底避免算法分叉导致转换后无法读取。
 *
 * 用法: convert.exe [data目录路径]
 *   不带参数时默认转换当前目录下的 data 文件夹
 *
 * 文件格式:
 *   旧版: 纯 XOR 加密数据（无文件头）
 *   新版: [0x02版本标识] + [8字节IV] + [XTEA-CBC密文]
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include "crypto.h"
#include "types.h"

using namespace std;

// ========== 文件操作 ==========

static string readFile(const string& filepath) {
    ifstream file(filepath, ios::binary);
    if (!file.is_open()) return "";
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return content;
}

static bool writeFile(const string& filepath, const string& content) {
    ofstream file(filepath, ios::binary | ios::trunc);
    if (!file.is_open()) return false;
    file.write(content.c_str(), content.size());
    file.close();
    return true;
}

static bool isLegacyFormat(const string& data) {
    return !data.empty() && (unsigned char)data[0] != CRYPTO_VERSION;
}

static string backupFile(const string& filepath) {
    string backupPath = filepath + ".bak";
    string content = readFile(filepath);
    if (!content.empty()) {
        writeFile(backupPath, content);
    }
    return backupPath;
}

static vector<string> getDatFiles(const string& dir) {
    vector<string> files;
    string searchPath = dir + "*.dat";
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                string filename = findData.cFileName;
                // 跳过 .bak 文件
                if (filename.size() >= 8 &&
                    filename.substr(filename.size() - 8) == ".dat.bak") {
                    continue;
                }
                if (filename.size() >= 4 &&
                    filename.substr(filename.size() - 4) == ".dat") {
                    files.push_back(dir + filename);
                }
            }
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }
    return files;
}

// ========== 主程序 ==========

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    setlocale(LC_ALL, "Chinese_People's Republic of China.utf8");

    string dataDir;
    if (argc >= 2) {
        dataDir = argv[1];
        if (!dataDir.empty() && dataDir[dataDir.size() - 1] != '\\') {
            dataDir += "\\";
        }
    } else {
        dataDir = "data\\";
    }

    cout << "========================================" << endl;
    cout << "  数据格式转换工具" << endl;
    cout << "  XOR(旧) -> XTEA-CBC(新)" << endl;
    cout << "========================================" << endl;
    cout << "数据目录: " << dataDir << endl;
    cout << endl;

    DWORD attr = GetFileAttributes(dataDir.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        cout << "错误: 数据目录不存在！" << endl;
        cout << "请确认路径是否正确。" << endl;
        cout << "按任意键退出..." << endl;
        system("pause >nul");
        return 1;
    }

    vector<string> datFiles = getDatFiles(dataDir);
    if (datFiles.empty()) {
        cout << "未找到任何 .dat 文件。" << endl;
        cout << "按任意键退出..." << endl;
        system("pause >nul");
        return 0;
    }

    cout << "找到 " << datFiles.size() << " 个数据文件:" << endl;
    for (size_t i = 0; i < datFiles.size(); i++) {
        cout << "  " << (i + 1) << ". " << datFiles[i] << endl;
    }
    cout << endl;

    cout << "警告：此操作将转换所有旧版数据文件的加密格式。" << endl;
    cout << "转换前会自动创建 .bak 备份文件（可用于回滚）。" << endl;
    cout << "确认开始转换？(输入 Y 确认，其他键取消): ";
    char confirm;
    cin >> confirm;
    cout << endl;

    if (confirm != 'Y' && confirm != 'y') {
        cout << "已取消转换。" << endl;
        cout << "按任意键退出..." << endl;
        system("pause >nul");
        return 0;
    }

    int successCount = 0;
    int skipCount = 0;
    int failCount = 0;

    for (size_t i = 0; i < datFiles.size(); i++) {
        const string& filepath = datFiles[i];
        cout << "[" << (i + 1) << "/" << datFiles.size() << "] 处理: " << filepath << endl;

        string content = readFile(filepath);
        if (content.empty()) {
            cout << "  -> 跳过（文件为空或无法读取）" << endl;
            skipCount++;
            continue;
        }

        if (!isLegacyFormat(content)) {
            cout << "  -> 跳过（已是新版加密格式）" << endl;
            skipCount++;
            continue;
        }

        // 旧版 XOR 解密
        string plainData = legacyXorCrypt(content);

        // 验证明文格式（查找 '\n' 或 ':'，确保解密结果合理）
        bool looksValid = plainData.find('\n') != string::npos || plainData.find(':') != string::npos;
        if (plainData.empty() || !looksValid) {
            cout << "  -> 警告：解密后数据格式可疑，仍继续转换..." << endl;
        }

        // 新版 XTEA-CBC 加密（使用主程序同一套算法）
        string encrypted = encryptData(plainData);

        // 立即验证：用主程序解密函数回读，确保一致
        string verify = decryptData(encrypted);
        if (verify != plainData) {
            cout << "  -> 错误：加密后回读失败，跳过此文件！" << endl;
            failCount++;
            continue;
        }

        string backupPath = backupFile(filepath);
        cout << "  -> 备份已创建: " << backupPath << endl;

        if (writeFile(filepath, encrypted)) {
            cout << "  -> 转换成功（已回读验证一致）" << endl;
            successCount++;
        } else {
            cout << "  -> 转换失败！（写入错误）" << endl;
            failCount++;
        }
    }

    cout << endl;
    cout << "========================================" << endl;
    cout << "  转换完成！" << endl;
    cout << "========================================" << endl;
    cout << "成功转换: " << successCount << " 个文件" << endl;
    cout << "跳过:     " << skipCount << " 个文件" << endl;
    cout << "失败:     " << failCount << " 个文件" << endl;
    if (successCount > 0) {
        cout << endl;
        cout << "提示：" << endl;
        cout << "  - 所有成功转换的文件在写入前都已用主程序同一套解密算法验证。" << endl;
        cout << "  - 旧数据已备份为 .bak 文件，确认转换无误后可手动删除。" << endl;
        cout << "  - 即使忘记转换，主程序读取旧数据时也会自动升级格式。" << endl;
    }
    cout << endl;
    cout << "按任意键退出..." << endl;
    system("pause >nul");

    return 0;
}
