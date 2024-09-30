#ifndef __DINO_H__
#define __DINO_H__

#include "entity.h"

typedef struct {
    Uint8   freelook;
    //        cameraPitch; //raise and lower target instead

    float   upspeed;
    float  rigspeed;
    float  forspeed;

}PlayerData;

Entity* player_spawn(GFC_Vector3D postion);
void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity*  self);

#endif