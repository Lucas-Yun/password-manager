#include "crypto.h"
#include <conio.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

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

string encryptData(const string& data) {
    string salt = generateSalt();
    string keyStream = generateKeyStream(salt, data.length());
    string encrypted;
    encrypted.reserve(data.length());
    for (size_t i = 0; i < data.length(); i++) {
        encrypted.push_back(data[i] ^ keyStream[i]);
    }
    return salt + encrypted;
}

string decryptData(const string& data) {
    if (data.length() < 8) {
        return "";
    }
    string salt = data.substr(0, 8);
    string encrypted = data.substr(8);
    string keyStream = generateKeyStream(salt, encrypted.length());
    string decrypted;
    decrypted.reserve(encrypted.length());
    for (size_t i = 0; i < encrypted.length(); i++) {
        decrypted.push_back(encrypted[i] ^ keyStream[i]);
    }
    return decrypted;
}

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

string checkPasswordStrength(const string& password) {
    int score = 0;
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;

    for (char c : password) {
        if (islower(c)) hasLower = true;
        else if (isupper(c)) hasUpper = true;
        else if (isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }

    if (hasLower) score++;
    if (hasUpper) score++;
    if (hasDigit) score++;
    if (hasSpecial) score++;
    if (password.length() >= 8) score++;
    if (password.length() >= 12) score++;

    if (score <= 2) return "弱";
    if (score <= 4) return "中";
    return "强";
}

string generateRandomPassword(int length) {
    const string chars =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "!@#$%^&*()_+-=[]{}|;:,.<>?";

    srand(static_cast<unsigned int>(time(nullptr)));
    string password;
    for (int i = 0; i < length; i++) {
        password += chars[rand() % chars.size()];
    }
    return password;
}
