#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "entity.h"

#define MAX_ENEMY 10

typedef struct {
	float           upspeed;
	float           rigspeed;

	GFC_Vector3D    og_pos;

	int             x_bound;
	int             z_bound;
}EnemyData;

Entity* enemy_spawn(GFC_Vector3D position);
void enemy_think(Entity* self);
void enemy_update(Entity* self);
void enemy_free(Entity* self);




#endif