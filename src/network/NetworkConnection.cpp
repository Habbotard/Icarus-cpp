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
#include "NetworkConnection.h"

/*
NetworkConnection constructor

@param connection id
@param tcp::socket connection socket

@return instance
*/
NetworkConnection::NetworkConnection(int connection_id, tcp::socket socket) : 
    connection_id(connection_id), 
    socket(std::move(socket)), 
    connection_state(true) { }

NetworkConnection::~NetworkConnection() { }

/*
Receive data handle

@return none
*/
void NetworkConnection::recieveData() {

    if (!this->connection_state) {
        return; // Person disconnected, stop listing for data, in case somehow it still is (when it shouldn't) ;)
    }

    auto self(shared_from_this());

    // only 4 bytes for now, the length
    socket.async_receive(boost::asio::buffer(buffer, 4), [this, self](boost::system::error_code ec, std::size_t length) {

        if (!ec) {

            // If the first part of the packet starts with '<'
            //    then we send the flash policy back

            if (buffer[0] == 60) {
                this->sendPolicy();
                this->recieveData();
            }
            else {

                long message_length = (int)buffer[3] & 0xff;
                message_length |= ((int)buffer[2] & 0xff) << 8;
                message_length |= ((int)buffer[1] & 0xff) << 16;
                message_length |= ((int)buffer[1] & 0xff) << 24;


                // Read rest of message, to prevent any combined packets
                socket.async_receive(boost::asio::buffer(buffer, message_length), [this, self, message_length](boost::system::error_code ec, std::size_t length) {

                    if (length > 0) {
                        Request request(message_length, buffer);

                        if (request.getMessageId() > 0) {
                            this->handleData(request);
                            this->recieveData();
                        }
                    }
                });
            }
        }
        else {

            // Handle session disconnect
            if (length == 0) {
                if (Icarus::getPlayerManager()->containsSession(this->connection_id)) {
                    Icarus::getPlayerManager()->removeSession(this->connection_id);
                }
                else {
                    // Remove connection if it was just a policy request
                    Icarus::getNetworkServer()->removeNetworkConnection(this);
                }
            }
        }

    });
}


/*S
    Handle incoming data

    @return none
*/
void NetworkConnection::handleData(Request request) {

    // Once we passed through the policy, create a session and handle it
    if (!Icarus::getPlayerManager()->containsSession(this->connection_id)) {
        Player *player = new Player(this);
        Icarus::getPlayerManager()->addSession(player, this->getConnectionId());
    }
    
    if (Icarus::getLogConfiguration()->getBool("log.network.rawpacket")) {
        cout << " [SESSION] [CONNECTION: " << this->connection_id << "] " << request.getMessageId() << " / ";

        for (int i = 0; i < request.getMessageLength(); i++) {

            char ch = request.getBuffer()[i];
            int ch_int = (int)ch;

            if (ch_int > -1 && ch_int < 14) {
                cout << "[" << ch_int << "]";
            }
            else {
                cout << request.getBuffer()[i];
            }
        }

        cout << endl;
    }
    
    Icarus::getMessageHandler()->invoke(request.getMessageId(), request, Icarus::getPlayerManager()->getSession(this->connection_id));

}

/*
    Send response class to socket

    @return none
*/
void NetworkConnection::send(Response response) {
    this->writeData(response.getData(), response.getBytesWritten());
}

/*
    Send compose class to socket

    @return none
*/
void NetworkConnection::send(const MessageComposer &composer) {
    Response response = composer.compose();
    this->writeData(response.getData(), response.getBytesWritten());
}


/*
    Write data handle

    @return none
*/
void NetworkConnection::writeData(const char* data, int length) {

    if (!this->connection_state) {
        return; // Person disconnected, stop listing for data, in case somehow it still is (when it shouldn't) ;)
    }

    auto self(shared_from_this());

    boost::asio::async_write(socket, boost::asio::buffer(data, /*this->max_length*/length), [this, self, data](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
            // send success
        }
    });
}


/*
    Send policy to the socket

    @return void

*/
void NetworkConnection::sendPolicy() {

    this->writeData("<?xml version=\"1.0\"?>\r\n<!DOCTYPE cross-domain-policy SYSTEM \"/xml/dtds/cross-domain-policy.dtd\">\r\n<cross-domain-policy>\r\n<allow-access-from domain=\"*\" to-ports=\"*\" />\r\n</cross-domain-policy>\0");
    
    auto self(shared_from_this());
    socket.async_receive(boost::asio::buffer(buffer, 18), [this, self](boost::system::error_code ec, std::size_t length) {});
}

/*
    Returns the connection ID (incremented from 0 when server starts)
    This is used to get the session from Session Manager

    WARNING: If the user hasn't passed the flash policy, using this to get the session
             is utterly useless

    @return connectionID integer
*/
int NetworkConnection::getConnectionId() { 
    return connection_id; 
};

/*
    Gets the connection state of the user. True for connected, false for disconnected

    @return connection flag
*/
bool NetworkConnection::getConnectionState() { 
    return connection_state; 
};

/*
    Sets the connection state, if set to false, the user will no longer receive
    any incoming packets

    @return none
*/
void NetworkConnection::setConnectionState(bool connection_state) {
    this->connection_state = connection_state; 
};
