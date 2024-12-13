#ifndef __ITEM_H__
#define __ITEM_H__

#include "entity.h"

typedef enum ItemType_E{
	NONE,
	SCRAP,
	HEALTH_PICKUP,

	// powerups
	HAPPY_TRIGGER,
	INVINCIBILITY
}ItemType;

typedef struct ItemData_S{
	ItemType		type;			// type of item
	Uint8			active;			// is currently moving to player
	int				upper_bound;

	// movement details
	float			forspeed;		// y-movement
	float           upspeed;		// z-movement
	float           rigspeed;		// x-movement
	float			dist_to_player;	// y-distance to player

}ItemData;

void item_spawn(int type, GFC_Vector3D spawn_pos, float dist_to_player);
void item_activate(Entity* self, int type);

#endif