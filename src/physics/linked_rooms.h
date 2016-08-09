#ifndef LINKED_ROOMS_H_INCLUDED
#define LINKED_ROOMS_H_INCLUDED

#include "util/vector.h"

typedef struct Room
{
   struct Room *neighbors[4];
   int x, y;
   Vector bodies;
} Room;

typedef struct RoomWeb
{
    /// Vector<Room>
    Vector rooms;
    float room_width, room_height;
} RoomWeb;

#endif
