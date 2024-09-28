#ifndef __DINO_H__
#define __DINO_H__

#include "entity.h"

Entity* player_spawn(GFC_Vector3D postion);
void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity*  self);

#endif