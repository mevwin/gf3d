#ifndef __ITEM_H__
#define __ITEM_H__

#include "entity.h"

typedef enum {
	SCRAP,
	NONE,
	HEALTH_PICKUP,
	HAPPY_TRIGGER,
	INVINCIBILITY
}Item_Type;

typedef struct {
	int				type;			// type of item
	Uint8			active;			

	// movement details
	float			forspeed;		// y-movement
	float           upspeed;		// z-movement
	float           rigspeed;		// x-movement
	float			dist_to_player;

}ItemData;

void item_spawn(int type, GFC_Vector3D spawn_pos, float dist_to_player);
void item_think(Entity* self);
void item_update(Entity* self);
void item_activate(Entity* self, int type);
void item_free(Entity* self);

#endif