#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"
#include "player_move.h"

typedef struct {
    Uint8           freelook;
    //        cameraPitch; //raise and lower target instead

    float           upspeed;
    float           rigspeed;
    float           forspeed;

    GFC_Vector3D    og_pos;
    int             mid_roll;
    int             roll;

    int             x_bound;
    int             z_bound;
            
}PlayerData;

Entity* player_spawn(GFC_Vector3D postion);
void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity*  self);

#endif