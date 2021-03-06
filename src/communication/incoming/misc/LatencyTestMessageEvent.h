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
#include "communication/outgoing/messenger/MessengerUpdateMessageComposer.h"

class LatencyTestMessageEvent : public MessageEvent {

public:
    LatencyTestMessageEvent() { }

    void handle(Player *player, Request &request) {

        /*if (player->getMessenger() != nullptr) {

            if (!player->getMessenger()->isInitialised()) {
                player->getMessenger()->setInitialised(true);
            }

            for (auto kvp : player->getMessenger()->getFriends()) {
                player->send(MessengerUpdateMessageComposer(kvp.second, false));
            }
        }*/
    }
};