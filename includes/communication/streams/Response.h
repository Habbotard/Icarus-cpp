#pragma once
#include <vector>
#include <iostream>
#include <cstring> // Required for G++/GCC compilation with function "strlen"

class Response
{

public:
    Response(short header);
    ~Response();
    void writeInt(int number);
    void writeInt(bool flag) { this->writeInt(flag ? 1 : 0);  }
    void writeInt(size_t number) { this->writeInt((int)number); }
    void writeBool(bool flag) {
        this->message.push_back(flag ? (char)1 : (char)0);
        this->bytesWritten = this->bytesWritten + 1;
    }
    void writeShort(short numberr);
    void writeString(const char* str) { this->writeCChar(str); };
    void writeString(std::string str) { this->writeCChar(str.c_str()); };

    char* getData();
    char* getBytes(short num);
    char* getBytes(int num, bool reverse = false);

    int getBytesWritten() { return bytesWritten + 4/*the length at the start*/;  }
    std::vector<char> getMessage() { return message; }

private:
    short header;
    int bytesWritten;
    bool used;
    std::vector <char> message;
    void writeCChar(const char* str);
    enum { MAX_RESPONSE_SIZE = 1024 };
};

