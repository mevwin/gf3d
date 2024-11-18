#ifndef __UI_H__
#define __UI_H__

#include "gfc_shape.h"
#include "gf2d_sprite.h"

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

    float           nuke_alpha;
    float           emper_alpha;
    Uint8           audio_flag;

    /*start menu*/
    GFC_Rect        new_start_block;
    GFC_Rect        continue_block;
    GFC_Rect        s_quit_block;

    /*player death screen / pause menu*/
    GFC_Rect        resume_block;
    GFC_Rect        quit_block;
}UIData;

void UI_init();
void UI_free();

void shop_hud_draw(void* d);

/**
* @brief where upgrades are actually given to player
*/
void shop_think(void* d);

/**
* @brief reset shop progress
*/
void shop_reset();

void player_hud(void* d);

void start_menu();
void* start_menu_think();

void pause_menu();
void pause_menu_think(void* l);

void wave_start(void* l);
void wave_completed(void* p, void* l);

void player_death_screen();

void enemy_hud(void* e, GFC_Vector3D position);
void enemy_hud_all();


#endif