#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

#define MAX_PROJ 20

typedef struct {
	Uint8			type;			// projectile_type
	int				y_bound;		// limit to how far a projectile travels
	Entity*			owner;			// who shot the projectile
	Entity_Type		owner_type;		// enemy or player
	float			damage;			// damage it will deal

	// projectile flags
	Uint8			vortexed;		// was the projectile from player vortex?
	
		// missile data
	Uint8			missile_active;	// is missile in motion
	GFC_Vector3D*	missile_target;	// pointer to the missile target
		
		// nuke data
	Uint8			nuke_active;	// is nuke attack active?
	GFC_Vector3D    nuke_deton_pos;	// nuke's detonation position
	float			nuke_dur;		// nuke duraton

	// projectile movement details
	float           forspeed;		// y-movement
	float			upspeed;		// z-movement
	float			rigspeed;		// x-movement
}ProjData;

/**
* @brief spawn a player attack/projectile
* @param position: spawn position, relative to player position
* @param reticle_pos: reticie position
* @param curr_time: current time from attack call
* @param vortexed: was the projectile formed from vortex attack?
*/
void player_proj_spawn(GFC_Vector3D position, GFC_Vector3D reticle_pos, float curr_time, Uint8 vortexed);

/**
* @brief spawn an enemy attack/projectile
* @param position: spawn position, relative to enemy position
* @param player_pos: last recorded position of player
* @param owner: pointer to enemy entity
* @param curr_time: curren time from attack call
*/
void enemy_proj_spawn(GFC_Vector3D position, GFC_Vector3D player_pos, Entity* owner, float curr_time);

void proj_update_player(Entity* self);
void proj_update_enemy(Entity* self);
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

/**
* @brief vortex weapon sucks in enemy projectiles and sends them to reticle position
*/
void proj_think_vortex(Entity* self);

/**
* @brief super nuke travels to center of screen and detonates, attacking all enemies
*/
void proj_think_super_nuke(Entity* self);


/**
* @brief enemy attack that restricts player movement to a certain region on-screen
*/
void fencer_think(Entity* self);

void bomber_think(Entity* self);

void emper_think(Entity* self);

#endif