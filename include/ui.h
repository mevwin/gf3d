#ifndef __UI_H__
#define __UI_H__

#include "gfc_shape.h"
#include "gf2d_sprite.h"
#include "player.h"
#include "enemy.h"

/*
typedef enum {
    SHIELDS,            // increase maxHealth
    SCRAP_HELD,         // increase maxScrap
    SINGLE_SHOT_UP,     // increase single_shot bonus damage
    CHARGE_SHOT_UP,     // increase charge_shot multipler
    MISSILES_UP,        // increase max_missile
    NUKE_COST_DOWN      // decrease nuke_cost
}Upgrade_Type;
*/

typedef struct {
    /*shop UI*/
        // upgrade button locations
    GFC_Rect        shields_block;
    GFC_Rect        scrap_block;
    GFC_Rect        missiles_block;
    GFC_Rect        single_shot_block;
    GFC_Rect        charge_shot_block;
    GFC_Rect        nuke_block;

        // shop background color
    GFC_Color       shop_color;         // color for shop UI
    float           shop_color_hue;     // shop 

        // player upgrade checks
    Uint8           shields_check;
    Uint8           more_scrap_check;
    Uint8           missiles_check;
    Uint8           single_shot_check;
    Uint8           charge_shot_check;
    Uint8           nuke_check;
    Uint8           upgrade_cost;

        // player upgrade max
    Uint8           shields_max;
    Uint8           more_scrap_max;
    Uint8           missiles_max;
    Uint8           single_shot_max;
    Uint8           charge_shot_max;
    Uint8           nuke_max; 

    /* player UI */
    Sprite*         player_health;
    Sprite*         player_shield;
    Sprite*         player_scrap;
    Sprite*         player_vortex;
    Sprite*         progress_bar;
    Sprite*         enemy_health;

    Sprite*         player_health_back;
    Sprite*         vortex_back;
    Sprite*         progress_back;
    Sprite*         enemy_health_back;

    /*start menu*/
    GFC_Rect        start_block;
    GFC_Rect        s_quit_block;

    /*player death screen / pause menu*/
    GFC_Rect        resume_block;
    GFC_Rect        quit_block;
}UIData;

Uint8 game_start, enemy_start;
float enemy_goal;
int _done;

void UI_init();
void UI_free();

void shop_hud_draw(PlayerData* data);
void shop_think(PlayerData* data);
void shop_reset();
//ShopData* get_shop_data();

void player_hud(PlayerData* data);

void start_menu();
Entity* start_menu_think();

void pause_menu();
void pause_menu_think(PlayerData* data);

void wave_start(PlayerData* data);
void wave_completed(PlayerData* data);

void player_death_screen(PlayerData* data);

void enemy_hud(EnemyData* data, GFC_Vector3D position);
void enemy_hud_all();


#endif