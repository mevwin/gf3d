#ifndef __ITEM_H__
#define __ITEM_H__

#include "entity.h"

typedef enum {
	SCRAP,
	HAPPY_TRIGGER,
	HEALTH_PICKUP,
	INVINCIBILITY
}Item_Type;

typedef struct {
	Item_Type		type;
	void*			enemy_data;

	float			forspeed;
	float           upspeed;
	float           rigspeed;

}ItemData;

Entity* item_spawn(Item_Type type, GFC_Vector3D spawn_pos, void* enemy_data);
void item_think(Entity* self);
void item_update(Entity* self);
void item_activate(Entity* self, void* player_data);
void item_free(Entity* self);

#endif