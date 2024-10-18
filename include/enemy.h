#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "entity.h"

typedef struct {
	float           upspeed;
	float           rigspeed;

	GFC_Vector3D    og_pos;

	int             x_bound;
	int             z_bound;

	float           maxHealth;          // enemy's maximum health
	float           currHealth;         // enemy's current health

	Uint8			took_damage;
	float			damage_taken;
}EnemyData;

int enemy_count;

Entity* enemy_spawn();
void enemy_think(Entity* self);
void enemy_update(Entity* self);
void enemy_free(Entity* self);
void enemy_take_damage(Entity* self, EnemyData* data);
void enemy_die(Entity* self);




#endif