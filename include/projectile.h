#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

#define MAX_PROJ 20

typedef enum{
	SINGLE_SHOT,
	CHARGE_SHOT,
	MISSILE,
	SPREAD_SHOT,
	SUPER_NUKE
}ProjType;

typedef struct {
	ProjType		type;			// projectile_type
	int				y_bound;		// limit to how far a projectile travels
	GFC_Vector3D    reticle_pos;	// reticle position

	// projectile movement details
	float           forspeed;		// y-movement
	float			upspeed;
	float			rigspeed;
}ProjData;


void proj_spawn(GFC_Vector3D position, GFC_Vector3D reticle_pos, ProjType curr_mode, float curr_time);
void proj_update(Entity* self);
void proj_free(Entity* self);

/**
* @brief checks to see if projectile is within its y-bound;
*/
int	proj_exist(Entity* self, ProjData* data);

/**
* @brief thinking for SINGLE_SHOT and CHARGE_SHOT
*/
void proj_think_basic(Entity* self);

void proj_think_missile(Entity* self);
void proj_think_spread_shot(Entity* self);
void proj_think_super_nuke(Entity* self);

#endif