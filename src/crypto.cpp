#include "crypto.h"
#include <conio.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

string xorEncryptDecrypt(const string& data) {
    string result = data;
    for (size_t i = 0; i < result.size(); i++) {
        result[i] ^= ENCRYPT_KEY;
    }
    return result;
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
