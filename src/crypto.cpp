#include "crypto.h"
#include <conio.h>
#include <iostream>

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
