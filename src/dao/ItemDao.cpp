/**
* Icarus - A multi-platform C++ server
*
* Copyright 2016 Alex "Quackster" Miller
*
* Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
* (see https://creativecommons.org/licenses/by-nc-sa/4.0/, or LICENSE.txt for a full license
*/
#include "stdafx.h"

#include "mysql_connection.h"
#include "mysql.h"

#include "boot/Icarus.h"
#include "misc/Utilities.h"

#include "ItemDao.h"

#include "game/item/definitions/ItemDefinition.h"
#include "game/item/Item.h"

/*
    Returns a list of all item definitions, with the database id as they key
    and the item definition pointer as the value

    @return map with integer id key, and ItemDefinition pointer
*/
std::map<int, ItemDefinition*> ItemDao::getItemDefinitions() {

    std::map<int, ItemDefinition*> furnitures;
    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::Statement> statement = std::shared_ptr<sql::Statement>(sql_connection->createStatement());
        std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery("SELECT * FROM furniture"));

        while (result_set->next()) {
            
            ItemDefinition *furni = new ItemDefinition();
            furni->id = result_set->getInt("id");
            furni->public_name = result_set->getString("public_name");
            furni->item_name = result_set->getString("item_name");
            furni->width = result_set->getInt("width");
            furni->length = result_set->getInt("length");
            furni->can_stack = result_set->getInt("can_stack") == 1;
            
            std::string::size_type sz;
            furni->height = std::stod(result_set->getString("stack_height"), &sz);
            furni->type = result_set->getString("type");

            furni->can_stack = true;
            furni->can_sit = result_set->getInt("can_sit") == 1;
            furni->sprite_id = result_set->getInt("sprite_id");

            furni->is_walkable = result_set->getInt("is_walkable") == 1;
            furni->allow_gift = result_set->getInt("allow_gift") == 1;
            furni->allow_trade = result_set->getInt("allow_trade") == 1;
            furni->allow_inventory_stack = result_set->getInt("allow_inventory_stack") == 1;
            furni->allow_marketplace_sell = result_set->getInt("allow_marketplace_sell") == 1;
            furni->allow_recycle = result_set->getInt("allow_recycle") == 1;
            furni->interaction_type = result_set->getString("interaction_type");
            furni->interaction_modes_count = result_set->getInt("interaction_modes_count");
            furni->vending_id = std::stoi(result_set->getString("vending_ids"));
            furni->effect_id = result_set->getInt("effectid");
            //furni->height_adjustible = resultSet->getInt("height_adjustible");
            furni->flat_id = result_set->getInt("flat_id");
            furni->song_id = result_set->getInt("song_id");
            furni->requires_rights = result_set->getInt("requires_rights") == 1;
            furni->is_arrow = result_set->getInt("is_arrow");
            furnitures.insert(std::make_pair(furni->id, furni));

            if (Utilities::isEqual(furni->height, 0) || furni->can_sit || furni->interaction_type == "bed") {
                furni->height = 0.01;
            }

        }
    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return furnitures;
}

/*
    Gets a list of inventory items by the given user id

    @param user id of items to get
    @return vector of items
*/
std::vector<Item*> ItemDao::getInventoryItems(int user_id) {

    std::vector<Item*> items;

    std::map<int, ItemDefinition*> furnitures;
    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("SELECT * FROM items WHERE user_id = ? AND room_id = -1")); {
            statement->setInt(1, user_id);
        }

        std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

        while (result_set->next()) {

            Item *item = new Item(
                result_set->getInt("id"),
                result_set->getInt("user_id"),
                result_set->getInt("owner_id"),
                result_set->getInt("item_id"),
                result_set->getInt("room_id"),
                result_set->getString("x"),
                result_set->getString("y"),
                result_set->getDouble("z"),
                result_set->getInt("rotation"),
                result_set->getString("extra_data")
            );

            items.push_back(item);

        }

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return items;
}

/*
    Gets a list of room items by given room id

    @param room id to get list of items
    @param list of items
*/
std::vector<Item*> ItemDao::getRoomItems(int room_id) {

    std::vector<Item*> items;

    std::map<int, ItemDefinition*> furnitures;
    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("SELECT * FROM items WHERE room_id = ?")); {
            statement->setInt(1, room_id);
        }

        std::shared_ptr<sql::ResultSet> result_set = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

        while (result_set->next()) {

            Item *item = new Item(
                result_set->getInt("id"),
                result_set->getInt("user_id"),
                result_set->getInt("owner_id"),
                result_set->getInt("item_id"),
                result_set->getInt("room_id"),
                result_set->getString("x"),
                result_set->getString("y"),
                result_set->getDouble("z"),
                result_set->getInt("rotation"),
                result_set->getString("extra_data")
            );

            items.push_back(item);

        }

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return items;
}

/*
    Inserts a new item into the database, with the furniture, owner and extra data given
    and will return an item instance with its auto-inserted id

    @param item id
    @param owner id
    @param extra data
    @return Item
*/
Item *ItemDao::newItem(int item_id, int owner_id, std::string extra_data) {

    int id = -1;
    Item *item = nullptr;

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("INSERT INTO items (user_id, owner_id, item_id, extra_data) VALUES(?, ?, ?, ?)")); {
            statement->setInt(1, owner_id);
            statement->setInt(2, owner_id);
            statement->setInt(3, item_id);
            statement->setString(4, extra_data);
        }

        statement->execute();

        // Last inserted id
        std::shared_ptr<sql::Statement> stmnt = std::shared_ptr<sql::Statement>(sql_connection->createStatement()); {
            auto result_set = stmnt->executeQuery("SELECT LAST_INSERT_ID() as id;");
            result_set->next();
            id = result_set->getInt("id");
        }

        item = new Item(id, owner_id, owner_id, item_id, -1, "", "", -1, 0, extra_data);
    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);

    return item;
}

/*
    Saves all item data, such as coordinates, room id, inventory holder, item state/extra data etc

    @param Item
    @return none
*/
void ItemDao::save(Item *item) {

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    std::string x = std::to_string(item->x);
    std::string y = std::to_string(item->y);

    if (item->isWallItem()) {

        std::stringstream stream_x;
        stream_x << item->side << "," << item->width_x << "," << item->width_y;
        x = stream_x.str();
        
        std::stringstream stream_y;
        stream_y << item->length_x << "," << item->length_y;
        y = stream_y.str();

    }

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("UPDATE items SET room_id = ?, x = ?, y = ?, z = ?, rotation = ?, extra_data = ?, user_id = ? WHERE id = ?")); {
            statement->setInt(1, item->room_id);
            statement->setString(2, x);
            statement->setString(3, y);
            statement->setDouble(4, item->z);
            statement->setDouble(5, item->rotation);
            statement->setString(6, item->extra_data);
            statement->setInt(7, item->user_id);
            statement->setInt(8, item->id);
        }

        statement->execute();

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);


}

/*
    Deletes an item entry in the database

    @param Item
    @return none
*/
void ItemDao::remove(Item *item) {

    std::shared_ptr<MySQLConnection> connection = Icarus::getDatabaseManager()->getConnectionPool()->borrow();

    try {

        std::shared_ptr<sql::Connection> sql_connection = connection->sql_connection;
        std::shared_ptr<sql::PreparedStatement> statement = std::shared_ptr<sql::PreparedStatement>(sql_connection->prepareStatement("DELETE FROM items WHERE id = ?")); {
            statement->setInt(1, item->id);
        }

        statement->execute();

    }
    catch (sql::SQLException &e) {
        Icarus::getDatabaseManager()->printException(e, __FILE__, __FUNCTION__, __LINE__);
    }

    Icarus::getDatabaseManager()->getConnectionPool()->unborrow(connection);


}