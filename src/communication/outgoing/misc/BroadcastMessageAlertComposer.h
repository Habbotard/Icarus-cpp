/**
* Icarus - A multi-platform C++ server
*
* Copyright 2016 Alex "Quackster" Miller
*
* Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
* (see https://creativecommons.org/licenses/by-nc-sa/4.0/, or LICENSE.txt for a full license
*/
#pragma once
#include "communication/outgoing/MessageComposer.h"

class BroadcastMessageAlertComposer : public MessageComposer {

public:
    BroadcastMessageAlertComposer(std::string message, std::string url = "") :
        message(message),
        url(url) { }

    const Response compose() const {
        Response response = this->createResponse();
        response.writeString(message);
        response.writeString(url);
        return response;
    }

    const int getHeader() const {
        return Outgoing::BroadcastMessageAlertComposer;
    }

private:
    std::string message;
    std::string url;
};