#ifndef __UI_H__
#define __UI_H__

#include "gfc_shape.h"
#include "gf2d_sprite.h"
#include "gf3d_vgraphics.h"

#define RES (gf3d_vgraphics_get_resolution())

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

typedef struct UIData_S{
    /*shop UI*/
    SJson*          shop_data;
    Sprite*         shop;
    GFC_Rect        next_wave_block;

        // upgrades section
    GFC_Rect        shields_block;
    GFC_Rect        scrap_block;
    GFC_Rect        missiles_block;
    GFC_Rect        single_shot_block;
    GFC_Rect        charge_shot_block;
    GFC_Rect        nuke_block;
    GFC_Rect        scrap_bar;

        // shop data
            // player upgrade max
    Uint8           shields_max;
    Uint8           more_scrap_max;
    Uint8           missiles_max;
    Uint8           single_shot_max;
    Uint8           charge_shot_max;
    Uint8           nuke_max;

            // player upgrade checks
    Uint8           shields_check;
    Uint8           more_scrap_check;
    Uint8           missiles_check;
    Uint8           single_shot_check;
    Uint8           charge_shot_check;
    Uint8           nuke_check;
    Uint8           upgrade_cost;
    Uint8           perk_cost;
    Uint8           perk_sell_reduction;

            // player upgrade count (for "previous runs")
    Uint8           shields_count;
    Uint8           more_scrap_count;
    Uint8           missiles_count;
    Uint8           single_shot_count;
    Uint8           charge_shot_count;

        // perks section
    GFC_Rect        curr_perk1;
    GFC_Rect        curr_perk2;
    GFC_Rect        new_perk1;
    GFC_Rect        new_perk2;
    GFC_Rect        new_perk3;
    GFC_Rect        s_perk_desc;

    /*player UI*/
    SJson*          player_hud_data;
    Sprite*         player_hud;
    Sprite*         player_health;
    Sprite*         player_shield;
    Sprite*         player_scrap;
    Sprite*         player_vortex;
    Sprite*         progress_bar;

    Sprite*         enemy_health;
    Sprite*         enemy_health_back;

    float           nuke_alpha;
    float           emper_alpha;

    /*start menu*/
    SJson*          start_menu_data;
    Sprite*         start_menu;
    Sprite*         game_mode_select;
    GFC_Rect        new_start_block;
    GFC_Rect        continue_block;
    GFC_Rect        previous_block;
    GFC_Rect        s_quit_block;
    GFC_Rect        editor_block;

        // game_mode_select
    GFC_Rect        regular_block;
    GFC_Rect        endless_block;
    GFC_Rect        s_exit_block;
    
    /*pause menu*/
    SJson*          pause_menu_data;
    Sprite*         pause_menu;
    GFC_Rect        resume_block;
    GFC_Rect        quit_block;
    Sprite*         item_progress_bar;
    Sprite*         p_progress_bar;

        // same thing for game over screen
    GFC_Rect        p_perk1;    
    GFC_Rect        p_perk2;
    GFC_Rect        p_perk_desc;

    /*wave start*/
    SJson*          wave_start_data;
    Sprite*         wave_start;
    GFC_Vector2D    curr_wave_loc;

    /*wave completed*/
    SJson*          wave_completed_data;
    Sprite*         wave_completed;
    GFC_Rect        stage_block1;
    GFC_Rect        stage_block2;
    GFC_Rect        nextwave_block;

    /*game over*/
    SJson*          game_over_data;
    Sprite*         game_over;
    GFC_Rect        respawn_block;
    GFC_Rect        g_quit_block;
    // TODO: add way to check individual wave progress (maybe)

    /*previous runs*/
    SJson*          prev_menu_data;
    Sprite*         preview_menu;
    Sprite*         previous_run;
    GFC_Rect        return_block;
    GFC_Rect        exit_block;
    GFC_Rect        next_block;
    GFC_Rect        prev_block;
    int             preview_page_offset;

    /*game complete*/
    Sprite*         game_complete;
    // same data and rects as previous runs
}UIData;

void UI_init();

void UI_free();

void shop_hud_draw();

/**
* @brief where upgrades are actually given to player
*/
void shop_think();

/**
* @brief reset shop progress
*/
void shop_reset();

void player_hud(void* d);

void start_menu(Uint8 state);

void wave_start();

void wave_completed(Uint8 game_mode);

void player_death_screen(Sprite* menu, SJson* menu_data);

void enemy_hud_all();

void preview_runs();

void display_previous_run();

void game_complete();

UIData* get_UI_data();

#endif