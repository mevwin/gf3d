#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

#define MAX_PROJ 20
#define MAX_WAVE 1
#define MAX_MISSILE 7

typedef enum{
	SINGLE_SHOT,
	CHARGE_SHOT,
	WAVE_SHOT,
	MISSILE,
	SUPER_NUKE
}ProjType;

typedef struct {
	ProjType		type;			// projectile_type
	int				y_bound;		// limit to how far a projectile travels
	GFC_Vector3D    reticle_pos;	// reticle position
	GFC_Vector3D	spawn_pos;		// projectile's spawn position
	Entity*			owner;			// who shot the projectile

	// projectile movement details
	float           forspeed;		// y-movement
	float			upspeed;		// z-movement
	float			rigspeed;		// x-movement
}ProjData;


void player_proj_spawn(GFC_Vector3D position, GFC_Vector3D reticle_pos, Entity* player, float curr_time);
void proj_update(Entity* self);
void proj_free(Entity* self);

/**
* @brief checks to see if projectile is within its y-bound;
*/
Uint8 proj_exist(Entity* self, ProjData* data);

/**
* @brief thinking for SINGLE_SHOT and CHARGE_SHOT
*/
void proj_think_basic(Entity* self);

void proj_think_missile(Entity* self);
void proj_think_wave_shot(Entity* self);
void proj_think_super_nuke(Entity* self);
Uint8 proj_hit(Entity* self, ProjData* data);



#endif