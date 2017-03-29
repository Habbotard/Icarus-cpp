/**
* Icarus - A multi-platform C++ server
*
* Copyright 2016 Alex "Quackster" Miller
*
* Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
* (see https://creativecommons.org/licenses/by-nc-sa/4.0/, or LICENSE.txt for a full license
*/
#pragma once
#include "boot/Icarus.h"
#include "communication/incoming/MessageEvent.h"
#include "communication/outgoing/user/CreditsMessageComposer.h"
#include "communication/outgoing/user/UserObjectMessageComposer.h"
//#include "communication/outgoing/user/SendPerkAllowancesMessageComposer.h"

class InfoRetrieveMessageEvent : public MessageEvent {

public:
    InfoRetrieveMessageEvent() { }

    void handle(Player *player, Request &request) {

    }
};