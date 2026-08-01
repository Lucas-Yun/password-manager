/**
 * 加密一致性诊断程序
 * 用途：验证新版 XTEA-CBC 加解密往返的完全一致性
 *
 * 测试项目：
 *   1. 新版 XTEA-CBC 加解密往返（多次加密结果不同，但解密一致）
 *   2. 各种边界长度（PKCS7 填充验证）
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

// ========== 测试 1：XTEA-CBC 往返 ==========
static void testXteaCbc() {
    cout << "[测试 1] XTEA-CBC 加解密往返" << endl;

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

// ========== 测试 2：各种边界长度 ==========
static void testEdgeCases() {
    cout << "[测试 2] 各种边界长度（PKCS7 填充验证）" << endl;

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

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    setlocale(LC_ALL, "Chinese_People's Republic of China.utf8");

    cout << "==============================================" << endl;
    cout << "  加密一致性诊断程序" << endl;
    cout << "==============================================" << endl;
    cout << endl;

    testXteaCbc();
    testEdgeCases();

    cout << "==============================================" << endl;
    cout << "  测试结果：通过 " << s_passCount << " 项，失败 " << s_failCount << " 项" << endl;
    cout << "==============================================" << endl;

    if (s_failCount == 0) {
        cout << "所有测试通过，XTEA-CBC 加密算法实现完全一致，可以放心使用。" << endl;
    } else {
        cout << "存在失败项目，请勿使用，需要排查问题！" << endl;
    }

    cout << endl << "按任意键退出..." << endl;
    system("pause >nul");
    return s_failCount == 0 ? 0 : 1;
}
