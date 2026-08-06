/**
 * 数据恢复诊断工具
 * 读取 data 目录下的实际文件，尝试所有可能的解密方式，输出明文
 * 用于诊断数据损坏情况并尝试恢复
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include "crypto.h"
#include "types.h"

using namespace std;

static string readFile(const string& filepath) {
    ifstream file(filepath, ios::binary);
    if (!file.is_open()) return "";
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return content;
}

static string toHex(const string& s) {
    const char* hex = "0123456789ABCDEF";
    string r;
    for (size_t i = 0; i < s.size() && i < 64; i++) {
        unsigned char c = (unsigned char)s[i];
        r += hex[c >> 4];
        r += hex[c & 0xF];
        r += ' ';
    }
    if (s.size() > 64) r += "...";
    return r;
}

static bool isPrintable(const string& s) {
    int printable = 0;
    for (size_t i = 0; i < s.size(); i++) {
        unsigned char c = (unsigned char)s[i];
        if ((c >= 32 && c <= 126) || c == '\n' || c == '\r' || c == '\t') {
            printable++;
        }
    }
    return s.size() > 0 && (printable * 100 / (int)s.size() > 70);
}

static void tryDecrypt(const string& label, const string& content) {
    cout << "--- " << label << " ---" << endl;

    // 方式1: 主程序 decryptData（自动识别新旧格式）
    string result1 = decryptData(content);
    cout << "  [decryptData] 可打印: " << (isPrintable(result1) ? "YES" : "NO");
    if (isPrintable(result1)) {
        cout << "  内容:" << endl;
        cout << "    " << result1 << endl;
    } else {
        cout << "  hex: " << toHex(result1) << endl;
    }

    // 方式2: 旧版 XOR
    string result2 = legacyXorCrypt(content);
    cout << "  [legacyXor]  可打印: " << (isPrintable(result2) ? "YES" : "NO");
    if (isPrintable(result2)) {
        cout << "  内容:" << endl;
        cout << "    " << result2 << endl;
    } else {
        cout << "  hex: " << toHex(result2) << endl;
    }

    cout << endl;
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    setlocale(LC_ALL, "Chinese_People's Republic of China.utf8");

    cout << "========================================" << endl;
    cout << "  数据恢复诊断工具" << endl;
    cout << "========================================" << endl;
    cout << endl;

    string dataDir = "data\\";
    WIN32_FIND_DATA findData;
    string searchPath = dataDir + "*.dat";
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        cout << "未找到任何 .dat 文件" << endl;
        system("pause");
        return 1;
    }

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            string filename = findData.cFileName;
            if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".dat") {
                string filepath = dataDir + filename;
                string content = readFile(filepath);
                cout << "文件: " << filepath << "  (" << content.size() << " 字节)" << endl;
                cout << "  首字节: 0x" << hex << (int)(unsigned char)content[0] << dec << endl;
                cout << "  原始hex: " << toHex(content) << endl;
                cout << endl;
                tryDecrypt(filename, content);
                cout << "========================================" << endl;
            }
        }
    } while (FindNextFile(hFind, &findData));
    FindClose(hFind);

    cout << endl << "按任意键退出..." << endl;
    system("pause >nul");
    return 0;
}
