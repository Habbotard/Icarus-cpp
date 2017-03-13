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
#include "game/pathfinder/Pathfinder.h"

#include "plugin/events/player/PlayerWalkEvent.h"

class WalkMessageEvent : public MessageEvent {

public:
    WalkMessageEvent() { }

    void handle(Player *player, Request &request) {

        Room *room = player->getRoomUser()->getRoom();

        if (room == nullptr) {
            return;
        }

        int goal_x = request.readInt();
        int goal_y = request.readInt();


        RoomUser *room_user = player->getRoomUser();
        room_user->awake();

        room_user->goal.x = goal_x;
        room_user->goal.y = goal_y;

        Position goal = room_user->goal;
        Position current = room_user->position;

        int map_size_x = room->getModel()->getMapSizeX();
        int map_size_y = room->getModel()->getMapSizeY();

		PlayerWalkEvent *player_walk_event = static_cast<PlayerWalkEvent*>(Icarus::getGame()->getPluginManager()->callEvent(std::make_shared<PlayerWalkEvent>(player)));

		if (player_walk_event->isCancelled()) {
			return;
		}

        if (goal.x >= map_size_x || goal.x >= map_size_y) {
            return;
        }

        if (goal.x == current.x && goal.x == current.y) {
            return;
        }

        if (!Pathfinder::isValidStep(room, current, goal, false)) {
            return;
        }

        auto path = Pathfinder::makePath(room_user->position, room_user->goal, room);

        if (path.size() == 0) {
            return;
        }

        room_user->setPath(path);
        room_user->is_walking = true;
    }
};