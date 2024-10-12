#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

typedef struct {
    Uint8           freelook;

    float           upspeed;
    float           rigspeed;
    float           forspeed;

    GFC_Vector3D    og_pos;
    int             mid_roll;
    int             roll;

    int             x_bound;
    int             z_bound;

    int             curr_mode;
            
}PlayerData;

Entity* player_spawn(GFC_Vector3D position);
void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity*  self);
void player_attack(Entity* self, PlayerData* data);

#endif