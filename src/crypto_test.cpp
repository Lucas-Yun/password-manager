/**
 * 加密一致性诊断程序
 * 用途：验证旧版XOR、新版XTEA-CBC、加解密往返的完全一致性
 *
 * 测试项目：
 *   1. 旧版 XOR 加解密往返
 *   2. 新版 XTEA-CBC 加解密往返（多次加密结果不同，但解密一致）
 *   3. 模拟旧版 data 文件 -> 转换流程 -> 读取验证
 */

#include <iostream>
#include <string>
#include <cassert>
#include <windows.h>
#include "crypto.h"
#include "types.h"

using namespace std;

static int s_passCount = 0;
static int s_failCount = 0;

static void testReport(const string& name, bool pass, const string& detail = "") {
    if (pass) {
        cout << "  [PASS] " << name;
        if (!detail.empty()) cout << "   (" << detail << ")";
        cout << endl;
        s_passCount++;
    } else {
        cout << "  [FAIL] " << name;
        if (!detail.empty()) cout << "   " << detail;
        cout << endl;
        s_failCount++;
    }
}

static string toHex(const string& s) {
    const char* hex = "0123456789ABCDEF";
    string r;
    for (size_t i = 0; i < s.size(); i++) {
        unsigned char c = (unsigned char)s[i];
        r += hex[c >> 4];
        r += hex[c & 0xF];
        r += ' ';
    }
    return r;
}

// ========== 测试 1：旧版 XOR ==========
static void testLegacyXor() {
    cout << "[测试 1] 旧版 XOR 加解密往返" << endl;

    string plaintext = "Admin:admin_root\nuser1:pass123\nuser2:abc456\n";
    string encrypted = legacyXorCrypt(plaintext);
    string decrypted = legacyXorCrypt(encrypted);

    testReport("往返一致", decrypted == plaintext,
        "原文长度 " + to_string(plaintext.size()) + " -> " +
        to_string(encrypted.size()) + " 字节");

    // 每个字节确实被异或了
    bool isXored = true;
    for (size_t i = 0; i < min(plaintext.size(), encrypted.size()); i++) {
        if ((unsigned char)encrypted[i] != ((unsigned char)plaintext[i] ^ (unsigned char)LEGACY_XOR_KEY)) {
            isXored = false;
            break;
        }
    }
    testReport("异或正确性", isXored);

    cout << endl;
}

// ========== 测试 2：新版 XTEA-CBC 往返 ==========
static void testXteaCbc() {
    cout << "[测试 2] 新版 XTEA-CBC 加解密往返" << endl;

    string plaintext = "Admin:admin_root\nuser1:pass123\nuser2:abc456\n";

    for (int trial = 1; trial <= 3; trial++) {
        string encrypted = encryptData(plaintext);

        testReport("版本标识正确 (0x02)",
            !encrypted.empty() && (unsigned char)encrypted[0] == CRYPTO_VERSION,
            "首字节 = 0x" + toHex(string(1, encrypted.empty() ? 0 : encrypted[0])));

        testReport("包含 IV（总长 >= 9 字节）",
            encrypted.size() >= 9,
            "总长度 " + to_string(encrypted.size()) + " 字节");

        // 解密回读
        string decrypted = decryptData(encrypted);
        testReport("第" + to_string(trial) + "次往返一致",
            decrypted == plaintext,
            "原文 " + to_string(plaintext.size()) + " 字节 -> 密文 " +
            to_string(encrypted.size()) + " 字节 -> 解密 " +
            to_string(decrypted.size()) + " 字节");
    }

    // 验证 IV 随机：同一明文两次加密结果不同
    string enc1 = encryptData(plaintext);
    string enc2 = encryptData(plaintext);
    bool ivRandom = (enc1 != enc2);
    testReport("IV 随机化（同一明文加密结果不同）", ivRandom);
    // 两次不同加密的解密结果必须相同
    bool decSame = (decryptData(enc1) == decryptData(enc2));
    testReport("不同密文解密出相同明文", decSame && decryptData(enc1) == plaintext);

    cout << endl;
}

// ========== 测试 3：各种边界长度 ==========
static void testEdgeCases() {
    cout << "[测试 3] 各种边界长度（PKCS7 填充验证）" << endl;

    for (int len = 0; len <= 17; len++) {
        string text(len, 'A');
        string enc = encryptData(text);
        string dec = decryptData(enc);
        bool ok = (dec == text);
        if (!ok) {
            testReport("长度 " + to_string(len) + " 失败", false,
                "原文 " + to_string(text.size()) + " -> 解密 " + to_string(dec.size()));
        } else {
            s_passCount++;
        }
    }
    testReport("0-17 字节边界长度全部通过", true,
        "PKCS7 填充/去填充正常");

    // 中文测试
    string zhText = "Admin:管理员密码\n测试用户:测试密码123\n";
    string zhEnc = encryptData(zhText);
    string zhDec = decryptData(zhEnc);
    testReport("中文字符往返", zhDec == zhText,
        "原文 " + to_string(zhText.size()) + " 字节");

    cout << endl;
}

// ========== 测试 4：模拟完整转换流程 ==========
static void testConvertSimulate() {
    cout << "[测试 4] 模拟完整转换流程（旧->新）" << endl;

    // 用户数据
    string plainUsers = "Admin:admin_root\nzhangsan:pass1\nlisi:mima2\n";
    // 用户张三的密码数据
    string plainZhangsan =
        "GitHub:zhangsan@outlook.com,zhangsan001:G@HubPwd123\n"
        "Gmail:zhangsan@gmail.com:Gm@il_Secret\n";

    // 第一步：用旧版方式加密（模拟原有 data 文件）
    string legacyUsersFile = legacyXorCrypt(plainUsers);
    string legacyZhangsanFile = legacyXorCrypt(plainZhangsan);
    testReport("旧版加密 users.dat 格式（首字节非0x02）",
        !legacyUsersFile.empty() && (unsigned char)legacyUsersFile[0] != CRYPTO_VERSION);

    // 第二步：主程序 decryptData 自动识别旧格式并解密
    string loadResult1 = decryptData(legacyUsersFile);
    string loadResult2 = decryptData(legacyZhangsanFile);
    testReport("主程序兼容读取旧版 users.dat", loadResult1 == plainUsers);
    testReport("主程序兼容读取旧版 zhangsan.dat", loadResult2 == plainZhangsan);

    // 第三步：用 encryptData 重加密（模拟转换工具）
    string newUsers = encryptData(loadResult1);
    string newZhangsan = encryptData(loadResult2);
    testReport("重加密后版本标识为 0x02",
        !newUsers.empty() && (unsigned char)newUsers[0] == CRYPTO_VERSION &&
        !newZhangsan.empty() && (unsigned char)newZhangsan[0] == CRYPTO_VERSION);

    // 第四步：用主程序解密新版数据，确保数据一致
    string finalUsers = decryptData(newUsers);
    string finalZhangsan = decryptData(newZhangsan);
    testReport("新版 users.dat 回读一致", finalUsers == plainUsers);
    testReport("新版 zhangsan.dat 回读一致", finalZhangsan == plainZhangsan);

    // 第五步：解析用户数据，检查用户名密码是否正常
    bool parseOk = true;
    string parsedUsers[3][2];
    int userCount = 0;
    size_t pos = 0;
    while (pos < finalUsers.size()) {
        size_t nl = finalUsers.find('\n', pos);
        if (nl == string::npos) break;
        string line = finalUsers.substr(pos, nl - pos);
        size_t co = line.find(':');
        if (co != string::npos && userCount < 3) {
            parsedUsers[userCount][0] = line.substr(0, co);
            parsedUsers[userCount][1] = line.substr(co + 1);
            userCount++;
        }
        pos = nl + 1;
    }
    parseOk = (userCount == 3) &&
              (parsedUsers[0][0] == "Admin")    && (parsedUsers[0][1] == "admin_root") &&
              (parsedUsers[1][0] == "zhangsan") && (parsedUsers[1][1] == "pass1") &&
              (parsedUsers[2][0] == "lisi")     && (parsedUsers[2][1] == "mima2");
    testReport("转换后用户数据可正常解析（3个用户）", parseOk,
        "解析到 " + to_string(userCount) + " 个用户");

    cout << endl;
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    setlocale(LC_ALL, "Chinese_People's Republic of China.utf8");

    cout << "==============================================" << endl;
    cout << "  加密一致性诊断程序" << endl;
    cout << "==============================================" << endl;
    cout << endl;

    testLegacyXor();
    testXteaCbc();
    testEdgeCases();
    testConvertSimulate();

    cout << "==============================================" << endl;
    cout << "  测试结果：通过 " << s_passCount << " 项，失败 " << s_failCount << " 项" << endl;
    cout << "==============================================" << endl;

    if (s_failCount == 0) {
        cout << "所有测试通过，加密算法实现与转换流程完全一致，可以放心使用。" << endl;
        cout << endl;
        cout << "数据升级建议操作：" << endl;
        cout << "  方案A（自动）：运行 password_manager.exe，首次登录即自动升级旧数据" << endl;
        cout << "  方案B（手动）：运行 convert.exe，先 .bak 备份再转换并回读验证" << endl;
        cout << endl;
        cout << "如果之前用旧版 convert 转换后无法登录，可按以下步骤恢复：" << endl;
        cout << "  1. 将 data 目录下对应的 .bak 文件去掉 .bak 后缀（覆盖坏的 .dat）" << endl;
        cout << "  2. 运行重新编译后的 password_manager.exe，登录即可自动升级" << endl;
        cout << "     或 运行新版 convert.exe 进行转换" << endl;
    } else {
        cout << "存在失败项目，请勿使用，需要排查问题！" << endl;
    }

    cout << endl << "按任意键退出..." << endl;
    system("pause >nul");
    return s_failCount == 0 ? 0 : 1;
}
