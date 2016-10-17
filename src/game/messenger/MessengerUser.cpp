/**
* Icarus - A multi-platform C++ server
*
* Copyright 2016 Alex "Quackster" Miller
*
* Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
* (see https://creativecommons.org/licenses/by-nc-sa/4.0/, or LICENSE.txt for a full license
*/
#include "stdafx.h"

#include "boot/Icarus.h"
#include "game/player/Player.h"
#include "dao/UserDao.h"
#include "game/messenger/MessengerUser.h"

MessengerUser::MessengerUser(int user_id) :
    user_id(user_id),
    details(UserDao::getDetails(user_id)) {

    this->update();
}

MessengerUser::~MessengerUser() { }

void MessengerUser::update() {
    this->session = Icarus::getPlayerManager()->getPlayerById(this->user_id);
}

void MessengerUser::serialise(Response &response, const bool force_offline) { 

    response.writeInt(this->details->getId());
    response.writeString(this->details->getUsername());
    response.writeInt(1); // gender
    response.writeBool(force_offline ? false : this->isOnline());
    response.writeBool(force_offline ? false : this->inRoom());

    if (force_offline) {
        response.writeString("");
        response.writeInt(0);
        response.writeString("");
    }
    else {
        response.writeString(this->isOnline() ? this->details->getFigure() : "");
        response.writeInt(0);
        response.writeString(this->isOnline() ? this->details->getMotto() : "");
    }

    response.writeString("");
    response.writeString("");
    response.writeBool(true);
    response.writeBool(false);
    response.writeBool(false);
    response.writeShort(0);
}

void MessengerUser::serialiseSearch(Response &response) {

    response.writeInt(this->getDetails()->getId());
    response.writeString(this->getDetails()->getUsername());
    response.writeString(this->getDetails()->getMotto());
    response.writeBool(this->isOnline());
    response.writeBool(this->inRoom());
    response.writeString("");
    response.writeInt(0);
    response.writeString(this->isOnline() ? this->getDetails()->getFigure() : "");
    response.writeString("");
}

bool MessengerUser::inRoom() {

    if (this->isOnline()) {
        return this->session->getRoomUser()->inRoom();
    }

    return false;
}