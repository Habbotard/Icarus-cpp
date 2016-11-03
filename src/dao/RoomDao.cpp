/**
* Icarus - A multi-platform C++ server
*
* Copyright 2016 Alex "Quackster" Miller
*
* Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
* (see https://creativecommons.org/licenses/by-nc-sa/4.0/, or LICENSE.txt for a full license
*/
#include "stdafx.h"

#include "UserDao.h"
#include "RoomDao.h"

#include "boot/Icarus.h"
#include "misc/Utilities.h"

/*
    Get all room models
    
    @return room model ptr instances
*/
std::map<std::string, RoomModel*> *RoomDao::getModels() {

    std::map<std::string, RoomModel*> *models = new std::map<std::string, RoomModel*>();
    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sqlConnection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("SELECT id, heightmap, door_x, door_y, door_z, door_dir FROM room_models "));

        std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

        while (result_set->next()) {
            models->insert(std::make_pair(result_set->getString("id"), 
                new RoomModel(
                    result_set->getString("id"), 
                    result_set->getString("heightmap"), 
                    result_set->getInt("door_x"), 
                    result_set->getInt("door_y"), 
                    result_set->getInt("door_z"), 
                    result_set->getInt("door_dir"))));
        }

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return models;

}

/*
    Load public rooms

    @return none
*/
void RoomDao::addPublicRooms() {

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sqlConnection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("SELECT id FROM rooms WHERE room_type = 1 "));

        std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

        while (result_set->next()) {
            Room *room = getRoom(result_set->getInt("id"));
            Icarus::getGame()->getRoomManager()->getPublicRooms()->push_back(room);
            Icarus::getGame()->getRoomManager()->getRooms()->insert(std::make_pair(room->getId(), room));
        }

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

}

/*
    Get list of room ids that the player owns

    @param room id
    @return vector of room ids
*/
std::vector<int> RoomDao::getPlayerRooms(int user_id) {

    std::vector<int> rooms;

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sqlConnection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("SELECT id FROM rooms WHERE owner_id = ? "));
        statement->setInt(1, user_id);

        std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

        while (result_set->next()) {
            rooms.push_back(result_set->getInt("id"));
        }

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return rooms;
}

/*
    Get a single room by room id

    @param room id
    @param room ptr instance
*/
Room* RoomDao::getRoom(int room_id) {

    // Convert room id to a vector if we want to get one room
    std::vector<int> rooms = { room_id };

    // Return single room
    return getRooms(rooms)[0];
}



/*
    Get list of room instances by list of room ids

    @param vector room ids
    @return room ptr instances
*/
std::vector<Room*> RoomDao::getRooms(std::vector<int> room_ids) {

    std::vector<Room*> rooms;
    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sqlConnection;

        for (int room_id : room_ids) {

            std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("SELECT id, name, room_type, thumbnail, owner_id, group_id, description, password, users_max, model, wallpaper, floor, outside, tags, trade_state, state, score, category, allow_pets, allow_pets_eat, allow_walkthrough, hidewall, wall_thickness, floor_thickness, chat_mode, chat_size, chat_speed, chat_flood, chat_distance, who_can_mute, who_can_kick, who_can_ban FROM rooms WHERE id = ? ")); {
                statement->setInt(1, room_id);
            }

            std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

            while (result_set->next()) {

                // These pointers get deleted when either
                // 
                // The user leaves the room and they're not the owner, and they're the last one in the room
                // The user disconnects and there's no one else in the room so they get deleted
                //
                // 'delete' called from the .deleteRoom function from RoomManager
                Room *room = new Room(room_id);

                RoomData *room_data = new RoomData();
                room->setRoomData(room_data);

                room_data->id = room_id;
                room_data->name = result_set->getString("name");
                room_data->room_type = (char)result_set->getInt("room_type");
                room_data->thumbnail = result_set->getString("thumbnail");
                room_data->owner_id = result_set->getInt("owner_id");
                room_data->owner_name = UserDao::getName(result_set->getInt("owner_id"));
                room_data->group_id = result_set->getInt("group_id");
                room_data->description = result_set->getString("description");
                room_data->password = result_set->getString("password");
                room_data->users_max = result_set->getInt("users_max");
                room_data->model = Icarus::getGame()->getRoomManager()->getModel(result_set->getString("model"));
                room_data->wallpaper = result_set->getString("wallpaper");
                room_data->floor = result_set->getString("floor");
                room_data->outside = result_set->getString("outside");
                room_data->tags = Utilities::split(result_set->getString("tags"), ',');
                room_data->trade_state = result_set->getInt("trade_state");
                room_data->state = result_set->getInt("state");
                room_data->score = result_set->getInt("score");
                room_data->category = result_set->getInt("category");
                room_data->allow_pets = result_set->getBoolean("allow_pets");
                room_data->allow_pets_eat = result_set->getBoolean("allow_pets_eat");
                room_data->allow_walkthrough = result_set->getBoolean("allow_walkthrough");
                room_data->hide_wall = result_set->getBoolean("hidewall");
                room_data->wall_thickness = result_set->getInt("wall_thickness");
                room_data->floor_thickness = result_set->getInt("floor_thickness");
                room_data->chat_mode = result_set->getInt("chat_mode");
                room_data->chat_size = result_set->getInt("chat_size");
                room_data->chat_speed = result_set->getInt("chat_speed");
                room_data->chat_flood = result_set->getInt("chat_flood");
                room_data->chat_distance = result_set->getInt("chat_distance");
                room_data->who_can_mute = result_set->getInt("who_can_mute");
                room_data->who_can_kick = result_set->getInt("who_can_kick");
                room_data->who_can_ban = result_set->getInt("who_can_ban");
                room_data->user_rights = getRights(room_id);

                rooms.push_back(room);
            }
        }

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return rooms;
}

/*
    Get list of room ids that the player owns

    @param room id
    @return vector of room ids
*/
std::vector<int> RoomDao::getRights(int room_id) {

    std::vector<int> rooms;

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sqlConnection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("SELECT user_id FROM room_rights WHERE room_id = ? "));
        statement->setInt(1, room_id);

        std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

        while (result_set->next()) {
            rooms.push_back(result_set->getInt("user_id"));
        }

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return rooms;
}

/*
    Delete room from database entry

    @param room id
    @return none
*/
void RoomDao::deleteRoom(int room_id) {

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sqlConnection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("DELETE FROM rooms WHERE id = ? "));
        statement->setInt(1, room_id);
        statement->execute();

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);
}
