#ifndef __UI_H__
#define __UI_H__

#include "player.h"
#include "enemy.h"

typedef enum {
	HEALTH_UP,
	SCRAP_HELD,
	SINGLE_SHOT_UP,
	CHARGE_SHOT_UP,
	MORE_MISSILES,
	NUKE_COST_DOWN
}Upgrade_Type;

void shop_hud(PlayerData* data);
void player_hud(PlayerData* data);
void enemy_hud(EnemyData* data, GFC_Vector3D position);
void enemy_hud_all();

#endif