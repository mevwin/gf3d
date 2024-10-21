#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

#define MAX_PROJ 20
#define MAX_WAVE 1
#define MAX_MISSILE 1

typedef enum{
	SINGLE_SHOT,	// player or enemy
	CHARGE_SHOT,	// player or enemy
	WAVE_SHOT,			// replace with EMP
	MISSILE,		// player only
	SUPER_NUKE		// player only
}ProjType;

typedef struct {
	ProjType		type;			// projectile_type
	int				y_bound;		// limit to how far a projectile travels
	GFC_Vector3D	spawn_pos;		// projectile's spawn position
	Entity*			owner;			// who shot the projectile
	Entity_Type		owner_type;		// enemy or player
	float			damage;			// damage it will deal 
	Uint8			missile_active;

	// projectile movement details
	float           forspeed;		// y-movement
	float			upspeed;		// z-movement
	float			rigspeed;		// x-movement
}ProjData;


void player_proj_spawn(GFC_Vector3D position, GFC_Vector3D reticle_pos, Entity* owner, float curr_time);
void enemy_proj_spawn(GFC_Vector3D position, GFC_Vector3D player_pos, Entity* owner, float curr_time);
void proj_update(Entity* self);
void proj_free(Entity* self);

/**
* @brief checks to see if projectile is within its y-bound;
*/
Uint8 proj_exist(Entity* self, ProjData* data);

/**
* @brief thinking for SINGLE_SHOT/PEAS and CHARGE_SHOT/CHARGERS
*/
void proj_think_basic(Entity* self);

/**
* @brief missiles only spawn when reticle meets enemy
*/
void proj_think_missile(Entity* self);
void proj_think_wave_shot(Entity* self);
void proj_think_super_nuke(Entity* self);
Uint8 shot_delay(Entity* self, float curr_time);


#endif