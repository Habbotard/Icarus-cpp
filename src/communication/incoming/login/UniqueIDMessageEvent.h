/**
* Icarus - A multi-platform C++ server
*
* Copyright 2016 Alex "Quackster" Miller
*
* Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
* (see https://creativecommons.org/licenses/by-nc-sa/4.0/, or LICENSE.txt for a full license
*/
#pragma once
#include "communication/incoming/MessageEvent.h"
#include "communication/outgoing/login/AuthenticateMessageComposer.h"

#include "dao/UserDao.h"

class UniqueIDMessageEvent : public MessageEvent {

public:
    UniqueIDMessageEvent() { }

    void handle(Player *player, Request &request) {

        request.readString(); // read junk str
        player->setUniqueId(request.readString());
    }
};