#include "stdafx.h"

#include "game/player/Player.h"
#include "game/player/PlayerDetails.h"
#include "boot/Icarus.h"

#include "communication/outgoing/navigator/FlatCategoriesMessageComposer.h"
#include "communication/outgoing/navigator/NavigatorCategoriesComposer.h"
#include "communication/outgoing/navigator/NavigatorMetaDataComposer.h"

/*
    Session constructor

    @param NetworkConnection ptr
    @return session instance
*/
Player::Player(NetworkConnection *network_connection) : 
    network_connection(network_connection), session_details(nullptr), room_user(nullptr) {
    
    std::cout << " [SESSION] Client connected with ID: " << this->getNetworkConnection()->getConnectionId() << std::endl;
}

/*
    Initialise player and load their data

    @return none
*/
void Player::login() {

    // Load player rooms
    Icarus::getGame()->getRoomManager()->createPlayerRooms(this->session_details->getId());

    // New room user instance
    this->room_user = new RoomUser();

    // authenticated
    this->logged_in = true;
}

/*
    Send message composer to the session's socket

    @param MessageComposer ptr
    @return none
*/
void Player::send(MessageComposer &composer) {
    this->network_connection->send(composer);
}

/*
    Clear session of any responsibilities

    @return none
*/
void Player::clear() {
     
    // Can't continue clearing if session details is null...
    if (session_details == nullptr) {
        return;
    }

    if (this->logged_in) {
        if (this->room_user->getRoom() != nullptr) {
            this->room_user->getRoom()->leave(this, false, true);
        }

        // Dispose player rooms
        std::vector<Room*> rooms = Icarus::getGame()->getRoomManager()->getPlayerRooms(this->session_details->getId()); {
            for (Room *room : rooms) {
                room->dispose();
            }
        }
    }
}

/*
    Session deconstructor, called when session disconnects, 
    is deleted from SessionManager class

    networkManager->connectionState has already been set to false 
    so there won't be any more packet receiving
*/
Player::~Player() {
    std::cout << " [SESSION] Client disconnected with ID: " << this->getNetworkConnection()->getConnectionId() << std::endl;

    if (!logged_in) {
        return;
    }

    if (session_details != nullptr) {
        delete session_details;
    }

    if (session_details != nullptr) {
        delete room_user;
    }
}