#include "stdafx.h"

#include "boot/Icarus.h"
#include "game/room/tasks/RoomRunnable.h"

/*
    Constructor for room runnable
*/
RoomRunnable::RoomRunnable(Room *room) : room(room) { }

/*
    Tick handler for RoomRunnable

    @return none
*/
void RoomRunnable::run() {

    printf("ticktock\n");

    this->room->scheduleRunnable(); // reschedule again!!
}