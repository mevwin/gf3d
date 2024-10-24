#ifndef __UI_H__
#define __UI_H__

#include "player.h"
#include "enemy.h"

typedef struct {
    GFC_Rect        shields_block;
    GFC_Rect        scrap_block;
    GFC_Rect        single_shot_block;
    GFC_Rect        charge_shot_block;
    GFC_Rect        missiles_block;
    GFC_Rect        nuke_block;
    GFC_Color       shop_color;         // color for shop UI
    float           shop_color_hue;     // shop 
}ShopData;

void shop_init();
void shop_free();
void shop_hud_draw(PlayerData* data);
void shop_think(PlayerData* data);
void player_hud(PlayerData* data);
void enemy_hud(EnemyData* data, GFC_Vector3D position);
void enemy_hud_all();
void pause_menu(PlayerData* data);
void player_death_screen(PlayerData* data);

#endif