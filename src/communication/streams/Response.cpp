#include "stdafx.h"
#include "communication/streams/Response.h"

/*
Response constructor, it will initialise the deque, and append the header in raw bytes

@parameter header as short
@return response instance
*/
Response::Response(short header) : header(header) {
    this->bytesWritten = 0;
    this->used = false;
    this->message = vector <char>(0);
    this->writeShort(header);
}

Response::~Response() { }

/*
Converts a short (16 bits) to little-endian represented in a char array

@parameter reverse, if true, will return in big endian format
@return char array
*/
unsigned char* Response::getBytes(short i) {

    /*char* output = new char[2] {
        (char)(i >> 8),
        (char)i
    };*/

    unsigned char output[2];

    output[0] = (unsigned char)(i >> 8);
    output[1] = (unsigned char)(i);

    return output;
}

/*
Converts a integer (32 bits) to little-endian represented in a char array

@parameter reverse, if true, will return in big endian format
@return char array
*/
unsigned char* Response::getBytes(int i, bool reverse) {

    unsigned char output[4];

    if (reverse) {
        output[0] = (unsigned char)i;
        output[1] = (unsigned char)(i >> 8);
        output[2] = (unsigned char)(i >> 16);
        output[3] = (unsigned char)(i >> 24);

    } else {
        output[3] = (unsigned char)i;
        output[2] = (unsigned char)(i >> 8);
        output[1] = (unsigned char)(i >> 16);
        output[0] = (unsigned char)(i >> 24);
    };

    return output;
}

/*
Writes a given integer in 4 bytes in little-endian format to the deque
and increases the bytes written by 4

@return char array
*/
void Response::writeInt(int number) {

    unsigned char* bytes = this->getBytes(number);

    for (int i = 0; i < 4; i++) {
        this->message.push_back(bytes[i]);
    }

    this->bytesWritten = this->bytesWritten + 4;
}

/*
Writes a given short in 2 bytes in little-endian format to the deque
and increases the bytes written by 2

@return none
*/
void Response::writeShort(short number) {

    unsigned char* bytes = this->getBytes(number);
    
    for (int i = 0; i < 2; i++) {
        this->message.push_back(bytes[i]);
    }

    this->bytesWritten = this->bytesWritten + 2;
}

/*
Writes a given string with length prefixed in UTF-8 format
and increases the bytes written by 2

@return none
*/
void Response::writeCChar(const char* str) {

    short length = (short)strlen(str);
    this->writeShort(length);

    for (int i = 0; i < length; i++) {
        this->message.push_back(str[i]);
    }

    this->bytesWritten = this->bytesWritten + length;
}


/*
Gets the entire collection of chars from deque into a single char array
with 32 bit length prefixed

@return char array of packet
*/
char* Response::getData() {
    
    if (!this->used) {
        this->used = true;

        // Get the size in raw 4 int 32 length prefixed, but reversed
        // as this needs to be inserted at the front
        unsigned char* size = this->getBytes(this->bytesWritten, true);

        for (int i = 0; i < 4; i++) {

            // Insert at the front of vector
            this->message.insert(this->message.begin(), size[i]);
        }
    }
        
    return this->message.data();
}
