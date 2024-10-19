#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "entity.h"

typedef enum {
	PEAS,		// SINGLE_SHOT
	FENCERS,	// restrict player space
	CHARGERS,	// CHARGE_SHOT
	BOMBERS,	// trynamite-type enemy
	EMPERS		// don't kill it in time, EMP that freezes ship
}Enemy_Type;

typedef struct {
	// enemy stats
	Enemy_Type		enemy_type;
	float           maxHealth;          // enemy's maximum health
	float           currHealth;         // enemy's current health
	float			base_damage;

	// enemy attack values
	float			pea_speed;

	// enemy movement
	float			forspeed;
	float           upspeed;
	float           rigspeed;
	
	// enemy bounds
	int             x_bound;
	int             z_bound;
	int				dist_to_player;
	
	// enemy flags
	int				proj_count;
	Uint8			took_damage;
	Uint8			has_shot;
	float			damage_taken;

	// other
	GFC_Vector3D*	player_pos;
	GFC_Vector3D    spawn_pos;
}EnemyData;

int enemy_count;

Entity* enemy_spawn(GFC_Vector3D* player_pos);
void enemy_think(Entity* self);
void enemy_update(Entity* self);
void enemy_free(Entity* self);
void enemy_take_damage(Entity* self, EnemyData* data);
void enemy_die(Entity* self);




#endif