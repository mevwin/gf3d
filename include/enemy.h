#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "entity.h"

typedef enum {
	PEAS,		// SINGLE_SHOT
	CHARGERS,	// CHARGE_SHOT
	FENCERS,	// restrict player space
	BOMBERS,	// trynamite-type enemy
	EMPERS		// don't kill it in time, EMP that freezes ship
}Enemy_Type;

typedef struct {
	// enemy stats
	int				enemy_type;			// type of enemy
	float           maxHealth;          // enemy's maximum health
	float           currHealth;         // enemy's current health
	float			base_damage;		// starting damage
	float			pea_speed;			// SINGLE_SHOT projectile speed

	// enemy movement
	float			forspeed;			// y movement
	float           upspeed;			// z movement
	float           rigspeed;			// x movement
	
	// enemy bounds
	int             x_bound;			// horizontal bounds
	int             z_bound;			// vertical bounds
	int				dist_to_player;		// distance to player
	
	// enemy flags
	int				proj_count;			// amount of active projectiles
	Uint8			took_damage;		// have they taken damage?
	float			damage_taken;		// amount of damage received
	Uint8           damaged_type;		// type of damage received
	float			next_single_shot;	// next shot time
	Uint8			enemy_dead;			// is enemy health less than zero

	// other
	GFC_Vector3D*	player_pos;			// pointer to player's current position
	GFC_Vector3D    spawn_pos;			// initial spawn position
	Uint8			missile_targeted;	// flag for player missile attack
}EnemyData;

Uint32 enemy_count;
Uint32 enemy_killed;

Entity* enemy_spawn(GFC_Vector3D* player_pos);
void enemy_think(Entity* self);
void enemy_update(Entity* self);
void enemy_free(Entity* self);
void enemy_take_damage(Entity* self, EnemyData* data);
void enemy_die(Entity* self, EnemyData* data, int item_type);
//void check_rand_position(Entity* self);



#endif