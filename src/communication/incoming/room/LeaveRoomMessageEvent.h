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

class LeaveRoomMessageEvent : public MessageEvent {

public:
    LeaveRoomMessageEvent() { }

    void handle(Player *player, Request &request) {
        player->getRoomUser()->leaveRoom();
    }
};