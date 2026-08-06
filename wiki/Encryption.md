# 加密方案详解

## 方案对比

| 项目 | 旧版 (v1) | 新版 (v2) |
|------|----------|----------|
| 算法 | 单字节 XOR | XTEA 块加密 |
| 密钥长度 | 8 位 (`0x5A`) | 128 位 (4 x 32位) |
| 工作模式 | 无 | CBC（密码块链接） |
| 填充方式 | 无 | PKCS7 |
| IV | 无 | 8 字节随机生成 |
| 加密轮数 | 1 | 32 |
| 版本标识 | 无（首字节为密文） | `0x02`（首字节） |
| 安全性 | 极低，可暴力破解 | 高，抗已知明文攻击 |

## XTEA 算法

### 算法简介

XTEA (eXtended Tiny Encryption Algorithm) 是一种轻量级分组密码：

- **分组大小**：64 位（8 字节，2 x `uint32_t`）
- **密钥大小**：128 位（16 字节，4 x `uint32_t`）
- **Feistel 结构**：每轮对两个 32 位半块进行交替变换
- **轮数**：本项目使用 32 轮（推荐值）

### 核心代码

加密单块（定义在 `crypto.cpp`）：

```cpp
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
```

解密单块（逆运算）：

```cpp
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
```

### 密钥

密钥定义在 `types.h`：

```cpp
const uint32_t XTEA_KEY[4] = {0x4D616E31, 0x61676572, 0x53656372, 0x33744B21};
// 对应 ASCII: "Man1agerSecr3tK!"
```

> 密钥以源码常量形式硬编码。如需更换密钥，修改 `types.h` 中的 `XTEA_KEY` 后重新编译，但旧数据将无法解密。

## CBC 模式

### 工作原理

CBC (Cipher Block Chaining) 模式将每个明文块与前一个密文块异或后再加密，使得相同明文加密出不同密文：

```
明文块1    明文块2    明文块3
  │          │          │
  ▼          ▼          ▼
 XOR IV    XOR C1     XOR C2
  │          │          │
  ▼          ▼          ▼
 XTEA加密  XTEA加密   XTEA加密
  │          │          │
  ▼          ▼          ▼
密文块1    密文块2    密文块3
```

### 解密过程

```
密文块1    密文块2    密文块3
  │          │          │
  ▼          ▼          ▼
XTEA解密   XTEA解密   XTEA解密
  │          │          │
  ▼          ▼          ▼
 XOR IV    XOR C1     XOR C2
  │          │          │
  ▼          ▼          ▼
明文块1    明文块2    明文块3
```

### IV 生成

每次加密时生成 8 字节随机 IV：

```cpp
srand((unsigned int)time(NULL));
for (size_t i = 0; i < XTEA_BLOCK_SIZE; i++) {
    iv[i] = (unsigned char)(rand() & 0xFF);
}
```

IV 随密文一起存储，无需保密。同一明文每次加密的 IV 不同，因此密文也不同。

## PKCS7 填充

XTEA 要求明文长度是 8 字节的整数倍。PKCS7 填充规则：

- 计算需要填充的字节数 `padLen = 8 - (data.size() % 8)`，如果正好整除则 `padLen = 8`
- 在数据末尾追加 `padLen` 个值为 `padLen` 的字节

示例：

| 原始长度 | padLen | 填充字节（十六进制） |
|---------|--------|-------------------|
| 0 字节 | 8 | `08 08 08 08 08 08 08 08` |
| 5 字节 | 3 | `03 03 03` |
| 8 字节 | 8 | `08 08 08 08 08 08 08 08` |
| 13 字节 | 3 | `03 03 03` |

去填充时读取最后一个字节的值，验证末尾对应数量的字节是否一致后截断。

## 加密文件格式

```
┌──────────┬──────────────┬──────────────────────────┐
│ 版本标识  │     IV       │       XTEA-CBC 密文       │
│  1 字节   │   8 字节     │   N x 8 字节 (N >= 1)    │
│  0x02    │  随机生成     │   PKCS7 填充后的加密数据   │
└──────────┴──────────────┴──────────────────────────┘
```

- **版本标识**：`0x02`，用于区分新旧格式
- **IV**：8 字节随机初始化向量
- **密文**：PKCS7 填充后的明文经 CBC 模式 XTEA 加密后的数据

## 向后兼容

`decryptData()` 函数会自动检测版本标识：

```cpp
string decryptData(const string& data) {
    if (data.empty()) return data;

    // 检查版本标识
    if ((unsigned char)data[0] != CRYPTO_VERSION) {
        // 旧版数据，用 XOR 解密
        return legacyXorCrypt(data);
    }

    // 新版格式解密...
}
```

- 首字节 == `0x02`：新版 XTEA-CBC 解密
- 首字节 != `0x02`：旧版 XOR 解密（向后兼容）

## 旧版 XOR 加密

仅供转换和兼容使用：

```cpp
const char LEGACY_XOR_KEY = 0x5A;

string legacyXorCrypt(const string& data) {
    string result = data;
    for (size_t i = 0; i < result.size(); i++) {
        result[i] ^= LEGACY_XOR_KEY;
    }
    return result;
}
```

XOR 加密是对称的，同一函数既用于加密也用于解密。

## 密码隐藏输入

所有密码输入使用 `getHiddenInput()`，通过 `_getch()` 逐字符读取（不回显），显示 `*` 代替：

```cpp
string getHiddenInput() {
    string password;
    char ch;
    while ((ch = _getch()) != '\r') {        // 回车结束
        if (ch == '\b') {                     // 退格删除
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";              // 擦除星号
            }
        } else if (ch >= 32 && ch <= 126) {   // 可打印 ASCII
            password.push_back(ch);
            cout << '*';
        }
    }
    cout << endl;
    return password;
}
```

支持退格键删除，只接受可打印 ASCII 字符（32-126）。
