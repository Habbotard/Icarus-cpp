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

#include "game/messenger/MessengerUser.h"
#include "dao/MessengerDao.h"
#include "dao/MySQLDao.h"

/*
    Return all friends and the messenger user instance

    @param user id

    @return map of user id keys and messager user ptr instance
*/
std::map<int, MessengerUser*> MessengerDao::getFriends(int user_id) {

    std::map<int, MessengerUser*> friends;// = new std::map<int, MessengerUser*>();
    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::Statement> statement = std::shared_ptr<sql::Statement>(sql_connection->createStatement());
        std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery("SELECT * FROM messenger_friendships WHERE (sender = " + std::to_string(user_id) + ") OR (receiver = " + std::to_string(user_id) + ")"));

        while (result_set->next()) {

            MessengerUser *buddy;

            if (result_set->getInt("sender") != user_id) {
                buddy = new MessengerUser(result_set->getInt("sender"));
            }
            else {
                buddy = new MessengerUser(result_set->getInt("receiver"));
            }

            friends.insert(std::make_pair(buddy->getDetails()->id, buddy));
        }

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return friends;
}

/*
    Returns all friend request that have been sent to the user

    @param user id

    @return map of user id keys and messager user ptr instance
*/
std::map<int, MessengerUser*> MessengerDao::getRequests(int user_id) {

    std::map<int, MessengerUser*> friends;// = new std::map<int, MessengerUser*>();
    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::Statement> statement = std::shared_ptr<sql::Statement>(sql_connection->createStatement());
        std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery("SELECT * FROM messenger_requests WHERE to_id = " + std::to_string(user_id)));

        while (result_set->next()) {

            MessengerUser *buddy = new MessengerUser(result_set->getInt("from_id"));
            friends.insert(std::make_pair(buddy->getDetails()->id, buddy));
        }

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return friends;

}

/*
    Returns list of users if their name is simillar to the search query

    @param query

    @return vector of user ids
*/
std::vector<int> MessengerDao::search(std::string query) {

    std::vector<int> results;
    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("SELECT id FROM users WHERE username LIKE ? LIMIT 30")); {
            statement->setString(1, "%" + query + "%");
        }

        std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

        while (result_set->next()) {
            results.push_back(result_set->getInt("id"));
        }

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return results;
}

/*
    Create new friend request entry

    @param the friend request sender
    @param the friend request receiver

    @return none
*/
bool MessengerDao::newRequest(int fromId, int toId) {

    bool success = false;

    if (!MySQLDao::exists("SELECT * FROM messenger_requests WHERE to_id = '" + std::to_string(toId) + "' AND from_id = '" + std::to_string(fromId) + "'") &&
        !MySQLDao::exists("SELECT * FROM messenger_requests WHERE to_id = '" + std::to_string(toId) + "' AND from_id = '" + std::to_string(fromId) + "'")) {


        std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

        try {

            std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
            std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("INSERT INTO messenger_requests (to_id, from_id) VALUES (?, ?)")); {
                statement->setInt(1, toId);
                statement->setInt(2, fromId);
            }

            statement->execute();
            success = true;

        }
        catch (sql::SQLException &e) {
            Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
        }

        Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    }

    return success;
}

/*
    Remove friend requests

    @param the friend request sender
    @param the friend request receiver

    @return none
*/
bool MessengerDao::removeRequest(int fromId, int toId) {

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {
        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::Statement> statement = std::shared_ptr<sql::Statement>(sql_connection->createStatement());
        statement->execute("DELETE FROM messenger_requests WHERE from_id = " + std::to_string(fromId) + " AND to_id = " + std::to_string(toId));
    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return false;
}

/*
    Removes friendship entry in database

    @param the original friend request sender
    @param the friend request receiver

    @return none
*/
bool MessengerDao::removeFriend(int friendId, int userId) {

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {
        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::Statement> statement = std::shared_ptr<sql::Statement>(sql_connection->createStatement());
        statement->execute("DELETE FROM messenger_friendships WHERE(sender = " + std::to_string(userId) + " AND receiver = " + std::to_string(friendId) + ") OR (receiver = " + std::to_string(userId) + " AND sender = " + std::to_string(friendId) + ")");
    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return false;
}

/*  
    Creates new friendship entry in database

    @param the original friend request sender
    @param the friend request receiver

    @return none
*/
bool MessengerDao::newFriend(int sender, int receiver) {

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("INSERT INTO messenger_friendships (sender, receiver) VALUES (?, ?)")); {
            statement->setInt(1, sender);
            statement->setInt(2, receiver);
        }

        statement->execute();

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return false;
}

/*
    Logs offline message with unread/read

    @param to_id, the user the message is sent to
    @param from_id, the user the message is sent from
    @param message, the message contents
    @param unread or not

    @return none
*/
void MessengerDao::offlineMessage(int to, int from, std::string message, bool unread) {

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("INSERT INTO messenger_messages (to_id, from_id, unread, message) VALUES (?, ?, ?, ?)")); {
            statement->setInt(1, to);
            statement->setInt(2, from);
            statement->setInt(3, unread ? 1 : 0);
            statement->setString(4, message);
        }

        statement->execute();

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

}

/*
    Returns all messages sent to the user while they were offline in map<message, from_id> format
    Will mark them as read once this function has been called

    @param id of the user the messages were sent to
    @return map
*/
std::map<std::string, int> MessengerDao::getOfflineMessages(int user_id) {

    std::map<std::string, int> results;
    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("SELECT to_id, from_id, unread, message FROM messenger_messages WHERE unread = 1 AND to_id = ?")); {
            statement->setInt(1, user_id);
        }

        std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

        while (result_set->next()) {
            results.insert(std::make_pair(result_set->getString("message"), result_set->getInt("from_id")));
        }

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }
    
    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);
    
    return results;

}

/*
    Update database to show all offline messages have been read by given 'sent to' user id

    @param user id of where the messages were sent to
    @return none
*/
void MessengerDao::readMessages(int user_id) {

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("UPDATE messenger_messages SET unread = 0 WHERE unread = 1 AND to_id = ?")); {
            statement->setInt(1, user_id);
        }

        statement->execute();
    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

}
