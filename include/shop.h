#ifndef __SHOP_H__
#define __SHOP_H__

#include "player.h"

typedef enum {
	HEALTH_UP,
	SCRAP_HELD,
	SINGLE_SHOT_UP,
	CHARGE_SHOT_UP,
	MORE_MISSILES,
	NUKE_COST_DOWN
}Upgrade_Type;

void shop_hud(PlayerData* data);

#endif