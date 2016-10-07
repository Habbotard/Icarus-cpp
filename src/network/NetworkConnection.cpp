#include "stdafx.h"
#include "boot/Icarus.h"
#include "network/NetworkConnection.h"
#include "communication/outgoing/login/AuthenticateMessageComposer.h"

/*
NetworkConnection constructor

@param connection id
@param tcp::socket connection socket

@return instance
*/
NetworkConnection::NetworkConnection(int connectionID, tcp::socket socket) : connectionID(connectionID), socket(std::move(socket)) {
    this->connectionState = true;
}

NetworkConnection::~NetworkConnection() { }

/*
Receive data handle

@return none
*/
void NetworkConnection::recieveData() {

    if (!this->connectionState) {
        return; // Person disconnected, stop listing for data, in case somehow it still is (when it shouldn't) ;)
    }

    auto self(shared_from_this());

    // only 4 bytes for now, the length
    socket.async_read_some(boost::asio::buffer(buffer, 4), [this, self](boost::system::error_code ec, std::size_t length) {

        if (!ec) {

            // If the first part of the packet starts with '<'
            //    then we send the flash policy back

            if (buffer[0] == 60) {
                this->sendPolicy();

                // Read rest of policy request
                socket.async_read_some(boost::asio::buffer(buffer, 18), [this, self](boost::system::error_code ec, std::size_t length) {});
            }
            else {

                // Use bitwise operators to get the length needed to read the rest of the message
                int messageLength = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3]);

                // Read rest of message, to prevent any combined packets
                socket.async_read_some(boost::asio::buffer(buffer, messageLength), [this, self, messageLength](boost::system::error_code ec, std::size_t length) {

                    if (length > 0) {
                        Request request(buffer);

                        if (request.getMessageId() > 0) {
                            this->handleData(request);
                        }
                    }
                });
            }

            this->recieveData();
        }
        else {

            // Handle session disconnect
            if (Icarus::getSessionManager()->containsSession(this->connectionID)) {
                Icarus::getSessionManager()->removeSession(this->connectionID);
            }
            else {
                // Remove connection if it was just a policy request
                Icarus::getNetworkServer()->removeNetworkConnection(this);
            }
        }

    });
}

/*
Write data handle

@return none
*/
void NetworkConnection::writeData(char* data, int length) {

    auto self(shared_from_this());

    boost::asio::async_write(socket, boost::asio::buffer(data, /*this->max_length*/length), [this, self, data](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
            // send success
        }
    });
}

/*
Handle incoming data

@return none
*/
void NetworkConnection::handleData(Request request) {

    // Once we passed through the policy, create a session and handle it
    if (!Icarus::getSessionManager()->containsSession(this->connectionID)) {
        Session *session = new Session(this);
        Icarus::getSessionManager()->addSession(session, this->getConnectionId());
    }

    //cout << " [SESSION] [CONNECTION: " << connectionID << "] " << request.getMessageId() << endl;
    Icarus::getMessageHandler()->invoke(request.getMessageId(), request, Icarus::getSessionManager()->getSession(this->connectionID));

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
void NetworkConnection::send(MessageComposer &composer) {

    // Compose message
    Response response = composer.compose();

    // Write to socket
    this->writeData(response.getData(), response.getBytesWritten());
}


/*
Send policy to the socket

@return void

*/
void NetworkConnection::sendPolicy() {
    char* policy = "<?xml version=\"1.0\"?>\r\n<!DOCTYPE cross-domain-policy SYSTEM \"/xml/dtds/cross-domain-policy.dtd\">\r\n<cross-domain-policy>\r\n<allow-access-from domain=\"*\" to-ports=\"*\" />\r\n</cross-domain-policy>\0";
    this->writeData(policy, (int)strlen(policy) + 1);
}

/*
Returns the connection ID (incremented from 0 when server starts)
This is used to get the session from Session Manager

WARNING: If the user hasn't passed the flash policy, using this to get the session
         is utterly useless

@return connectionID integer
*/
int NetworkConnection::getConnectionId() { 
    return connectionID; 
};

/*
Gets the connection state of the user. True for connected, false for disconnected

@return connection flag
*/
bool NetworkConnection::getConnectionState() { 
    return connectionState; 
};

/*
Sets the connection state, if set to false, the user will no longer receive
any incoming packets

@return none
*/
void NetworkConnection::setConnectionState(bool connectionState) {
    this->connectionState = connectionState; 
};
