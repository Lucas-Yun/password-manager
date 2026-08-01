#include "crypto.h"
#include <conio.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>

using namespace std;

// ========== XTEA 核心算法 ==========

static void xteaEncipherBlock(uint32_t v[2], const uint32_t key[4], unsigned int rounds) {
    uint32_t v0 = v[0], v1 = v[1], sum = 0;
    const uint32_t delta = 0x9E3779B9;
    for (unsigned int i = 0; i < rounds; i++) {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
    }
    v[0] = v0;
    v[1] = v1;
}

static void xteaDecipherBlock(uint32_t v[2], const uint32_t key[4], unsigned int rounds) {
    uint32_t v0 = v[0], v1 = v[1];
    const uint32_t delta = 0x9E3779B9;
    uint32_t sum = delta * rounds;
    for (unsigned int i = 0; i < rounds; i++) {
        v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
        sum -= delta;
        v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
    }
    v[0] = v0;
    v[1] = v1;
}

// ========== 辅助函数：字节与 uint32_t 转换 ==========

static void bytesToBlock(const unsigned char* bytes, uint32_t v[2]) {
    v[0] = ((uint32_t)bytes[0]) | ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
    v[1] = ((uint32_t)bytes[4]) | ((uint32_t)bytes[5] << 8) |
           ((uint32_t)bytes[6] << 16) | ((uint32_t)bytes[7] << 24);
}

static void blockToBytes(const uint32_t v[2], unsigned char* bytes) {
    bytes[0] = v[0] & 0xFF;
    bytes[1] = (v[0] >> 8) & 0xFF;
    bytes[2] = (v[0] >> 16) & 0xFF;
    bytes[3] = (v[0] >> 24) & 0xFF;
    bytes[4] = v[1] & 0xFF;
    bytes[5] = (v[1] >> 8) & 0xFF;
    bytes[6] = (v[1] >> 16) & 0xFF;
    bytes[7] = (v[1] >> 24) & 0xFF;
}

static void xorBlocks(unsigned char* dst, const unsigned char* a, const unsigned char* b) {
    for (int i = 0; i < (int)XTEA_BLOCK_SIZE; i++) {
        dst[i] = a[i] ^ b[i];
    }
}

// ========== PKCS7 填充/去填充 ==========

static string pkcs7Pad(const string& data) {
    string padded = data;
    size_t padLen = XTEA_BLOCK_SIZE - (data.size() % XTEA_BLOCK_SIZE);
    if (padLen == 0) padLen = XTEA_BLOCK_SIZE;
    for (size_t i = 0; i < padLen; i++) {
        padded += (char)(unsigned char)padLen;
    }
    return padded;
}

static string pkcs7Unpad(const string& data) {
    if (data.empty()) return data;
    unsigned char padLen = (unsigned char)data[data.size() - 1];
    if (padLen == 0 || padLen > XTEA_BLOCK_SIZE) return data;
    if (data.size() < padLen) return data;
    for (size_t i = 0; i < padLen; i++) {
        if ((unsigned char)data[data.size() - 1 - i] != padLen) return data;
    }
    return data.substr(0, data.size() - padLen);
}

// ========== 新版加密/解密（XTEA-CBC） ==========

string encryptData(const string& data) {
    if (data.empty()) {
        string result;
        result += (char)CRYPTO_VERSION;
        return result;
    }

    // PKCS7 填充
    string padded = pkcs7Pad(data);

    // 生成随机 IV（8 字节）
    unsigned char iv[XTEA_BLOCK_SIZE];
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }
    for (size_t i = 0; i < XTEA_BLOCK_SIZE; i++) {
        iv[i] = (unsigned char)(rand() & 0xFF);
    }

    // 构造输出: [版本标识(1)] + [IV(8)] + [密文]
    string result;
    result += (char)CRYPTO_VERSION;
    result += string((char*)iv, XTEA_BLOCK_SIZE);

    // CBC 模式加密
    unsigned char prevBlock[XTEA_BLOCK_SIZE];
    memcpy(prevBlock, iv, XTEA_BLOCK_SIZE);

    size_t numBlocks = padded.size() / XTEA_BLOCK_SIZE;
    for (size_t i = 0; i < numBlocks; i++) {
        const unsigned char* plainBlock = (const unsigned char*)padded.c_str() + i * XTEA_BLOCK_SIZE;

        // 明文块 XOR 前一个密文块
        unsigned char xoredBlock[XTEA_BLOCK_SIZE];
        xorBlocks(xoredBlock, plainBlock, prevBlock);

        // 加密
        uint32_t v[2];
        bytesToBlock(xoredBlock, v);
        xteaEncipherBlock(v, XTEA_KEY, XTEA_ROUNDS);
        unsigned char cipherBlock[XTEA_BLOCK_SIZE];
        blockToBytes(v, cipherBlock);

        result += string((char*)cipherBlock, XTEA_BLOCK_SIZE);
        memcpy(prevBlock, cipherBlock, XTEA_BLOCK_SIZE);
    }

    return result;
}

string decryptData(const string& data) {
    if (data.empty()) return data;

    // 检查版本标识
    if ((unsigned char)data[0] != CRYPTO_VERSION) {
        // 旧版数据，尝试用旧版 XOR 解密（向后兼容）
        return legacyXorCrypt(data);
    }

    // 新版格式: [版本(1)] + [IV(8)] + [密文]
    if (data.size() < 1 + XTEA_BLOCK_SIZE) return "";

    const unsigned char* iv = (const unsigned char*)data.c_str() + 1;
    const char* ciphertext = data.c_str() + 1 + XTEA_BLOCK_SIZE;
    size_t cipherLen = data.size() - 1 - XTEA_BLOCK_SIZE;

    if (cipherLen == 0) return "";
    if (cipherLen % XTEA_BLOCK_SIZE != 0) return "";

    string result;
    unsigned char prevBlock[XTEA_BLOCK_SIZE];
    memcpy(prevBlock, iv, XTEA_BLOCK_SIZE);

    size_t numBlocks = cipherLen / XTEA_BLOCK_SIZE;
    for (size_t i = 0; i < numBlocks; i++) {
        const unsigned char* cipherBlock = (const unsigned char*)ciphertext + i * XTEA_BLOCK_SIZE;

        // 解密
        uint32_t v[2];
        bytesToBlock(cipherBlock, v);
        xteaDecipherBlock(v, XTEA_KEY, XTEA_ROUNDS);
        unsigned char decryptedBlock[XTEA_BLOCK_SIZE];
        blockToBytes(v, decryptedBlock);

        // 解密后 XOR 前一个密文块
        unsigned char plainBlock[XTEA_BLOCK_SIZE];
        xorBlocks(plainBlock, decryptedBlock, prevBlock);

        result += string((char*)plainBlock, XTEA_BLOCK_SIZE);
        memcpy(prevBlock, cipherBlock, XTEA_BLOCK_SIZE);
    }

    // 去除 PKCS7 填充
    return pkcs7Unpad(result);
}

// ========== 旧版 XOR 加密（供转换程序使用） ==========

string legacyXorCrypt(const string& data) {
    string result = data;
    for (size_t i = 0; i < result.size(); i++) {
        result[i] ^= LEGACY_XOR_KEY;
    }
    return result;
}

// ========== 密码隐藏输入 ==========

string getHiddenInput() {
    string password;
    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
        } else if (ch >= 32 && ch <= 126) {
            password.push_back(ch);
            cout << '*';
        }
    }
    cout << endl;
    return password;
}
