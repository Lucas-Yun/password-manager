#include "file_io.h"
#include "crypto.h"
#include <fstream>
#include <windows.h>

using namespace std;

bool createDirectory(const string& dir) {
    return CreateDirectory(dir.c_str(), NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
}

map<string, string> loadUsers() {
    map<string, string> users;
    string filepath = DATA_DIR + USER_FILE;
    ifstream file(filepath, ios::binary);
    if (file.is_open()) {
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        if (!content.empty()) {
            string decrypted = xorEncryptDecrypt(content);
            size_t pos = 0;
            while (pos < decrypted.size()) {
                size_t newlinePos = decrypted.find('\n', pos);
                if (newlinePos == string::npos) break;
                string line = decrypted.substr(pos, newlinePos - pos);
                size_t colonPos = line.find(':');
                if (colonPos != string::npos) {
                    string username = line.substr(0, colonPos);
                    string password = line.substr(colonPos + 1);
                    users[username] = password;
                }
                pos = newlinePos + 1;
            }
        }
    }
    return users;
}

void saveUsers(const map<string, string>& users) {
    string data;
    for (const auto& pair : users) {
        data += pair.first + ":" + pair.second + "\n";
    }
    string encrypted = xorEncryptDecrypt(data);
    string filepath = DATA_DIR + USER_FILE;
    ofstream file(filepath, ios::binary);
    if (file.is_open()) {
        file.write(encrypted.c_str(), encrypted.size());
        file.close();
    }
}

map<string, PlatformInfo> loadPasswords(const string& username) {
    map<string, PlatformInfo> passwords;
    string filepath = DATA_DIR + username + ".dat";
    ifstream file(filepath, ios::binary);
    if (file.is_open()) {
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        if (!content.empty()) {
            string decrypted = xorEncryptDecrypt(content);
            size_t pos = 0;
            while (pos < decrypted.size()) {
                size_t newlinePos = decrypted.find('\n', pos);
                if (newlinePos == string::npos) break;
                string line = decrypted.substr(pos, newlinePos - pos);

                size_t firstColon = line.find(':');
                if (firstColon != string::npos) {
                    string platform = line.substr(0, firstColon);
                    string rest = line.substr(firstColon + 1);

                    size_t secondColon = rest.find(':');
                    if (secondColon != string::npos) {
                        string accountsStr = rest.substr(0, secondColon);
                        string password = rest.substr(secondColon + 1);

                        PlatformInfo info;
                        info.password = password;

                        size_t accountPos = 0;
                        while (accountPos < accountsStr.size()) {
                            size_t commaPos = accountsStr.find(',', accountPos);
                            if (commaPos == string::npos) {
                                string account = accountsStr.substr(accountPos);
                                if (!account.empty()) {
                                    info.accounts.push_back(account);
                                }
                                break;
                            } else {
                                string account = accountsStr.substr(accountPos, commaPos - accountPos);
                                if (!account.empty()) {
                                    info.accounts.push_back(account);
                                }
                                accountPos = commaPos + 1;
                            }
                        }

                        passwords[platform] = info;
                    }
                }
                pos = newlinePos + 1;
            }
        }
    }
    return passwords;
}

void savePasswords(const string& username, const map<string, PlatformInfo>& passwords) {
    string data;
    for (const auto& pair : passwords) {
        data += pair.first + ":";
        for (size_t i = 0; i < pair.second.accounts.size(); i++) {
            if (i > 0) data += ",";
            data += pair.second.accounts[i];
        }
        data += ":" + pair.second.password + "\n";
    }
    string encrypted = xorEncryptDecrypt(data);
    string filepath = DATA_DIR + username + ".dat";
    ofstream file(filepath, ios::binary);
    if (file.is_open()) {
        file.write(encrypted.c_str(), encrypted.size());
        file.close();
    }
}
