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
	void*			player_data;

	// movement details
	float			forspeed;		// y-movement
	float           upspeed;		// z-movement
	float           rigspeed;		// x-movement
	float			dist_to_player;

}ItemData;

Entity* item_spawn(int type, GFC_Vector3D spawn_pos, float dist_to_player, void* player_data);
void item_think(Entity* self);
void item_update(Entity* self);
void item_activate(Entity* self, int type, void* player_data);
void item_free(Entity* self);

#endif