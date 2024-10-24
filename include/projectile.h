#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

#define MAX_PROJ 20

typedef enum{
	SINGLE_SHOT,	// player or enemy
	CHARGE_SHOT,	// player or enemy
	VORTEX,			// player only
	MISSILE,		// player only
	SUPER_NUKE		// player only
}ProjType;

typedef struct {
	ProjType		type;			// projectile_type
	int				y_bound;		// limit to how far a projectile travels
	Entity*			owner;			// who shot the projectile
	Entity_Type		owner_type;		// enemy or player
	float			damage;			// damage it will deal

	// player flags
	Uint8			player_in_shop;	// is player in shop
	Uint8			player_paused;	// is game paused

	// missile data
	Uint8			missile_active;	// is missile in motion
	GFC_Vector3D*	missile_target;	// pointer to the missile target

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
void proj_think_vortex(Entity* self);
void proj_think_super_nuke(Entity* self);


#endif