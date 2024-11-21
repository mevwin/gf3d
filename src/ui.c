#include "simple_logger.h"
#include "gf3d_vgraphics.h"
#include "gfc_audio.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf2d_mouse.h"
#include "ui.h"
#include "player.h"
#include "enemy.h"
#include "world.h"
#include "level.h"
#include "item.h"

#define RES (gf3d_vgraphics_get_resolution())
#define UPGRADE_BLOCK_WIDTH 200.0f
#define UPGRADE_BLOCK_HEIGHT 100.0f
#define UPGRADE_BLOCK(x, y) (gfc_rect(x, y, UPGRADE_BLOCK_WIDTH, UPGRADE_BLOCK_HEIGHT))
//#define MENU_BLOCK(x, y) (gfc_rect(x, y, UPGRADE_BLOCK_WIDTH, UPGRADE_BLOCK_HEIGHT))
#define TEXT_LOCATION (gfc_vector2d((RES.x / 2.0f) - 90.0f, (RES.y / 2.0) - 200.0f))

static UIData* UI_data;

void UI_init() {
    float x_start, y_start;

    UI_data = gfc_allocate_array(sizeof(UIData), 1);
    if (!UI_data) return;

    /*shop UI*/
    UI_data->shop_color = gfc_color(0, 0, 1, 0.5f);
    UI_data->shop_color_hue = 0.0;
    
    UI_data->shields_check = 0;
    UI_data->more_scrap_check = 0;
    UI_data->missiles_check = 0;
    UI_data->single_shot_check = 0;
    UI_data->charge_shot_check = 0;
    UI_data->nuke_check = 0;
    UI_data->upgrade_cost = 5;

    UI_data->shields_max = 5;
    UI_data->more_scrap_max = 3;
    UI_data->missiles_max = 3;
    UI_data->single_shot_max = 3;
    UI_data->charge_shot_max = 2;
    UI_data->nuke_max = 1;

    x_start = RES.x / 4.0f;
    y_start = RES.y / 4.0f;
    UI_data->shields_block = UPGRADE_BLOCK(x_start, y_start);

    y_start += 120.0f;
    UI_data->scrap_block = UPGRADE_BLOCK(x_start, y_start);

    y_start += 120.0f;
    UI_data->missiles_block = UPGRADE_BLOCK(x_start, y_start);

    y_start = RES.y / 4.0f;
    x_start = (RES.x - (RES.x / 4.0f)) - UPGRADE_BLOCK_WIDTH;
    UI_data->single_shot_block = UPGRADE_BLOCK(x_start, y_start);

    y_start += 120.0f;
    UI_data->charge_shot_block = UPGRADE_BLOCK(x_start, y_start);

    y_start += 120.0f;
    UI_data->nuke_block = UPGRADE_BLOCK(x_start, y_start);

    /*player UI*/
    UI_data->player_health = gf2d_sprite_load_image("images/UI/health.png");
    UI_data->player_shield = gf2d_sprite_load_image("images/UI/shield.png");
    UI_data->player_scrap = gf2d_sprite_load_image("images/UI/scrap.png");
    UI_data->player_vortex = gf2d_sprite_load_image("images/UI/vortex.png");
    UI_data->progress_bar = gf2d_sprite_load_image("images/UI/progress.png");
    UI_data->enemy_health = gf2d_sprite_load_image("images/UI/enemy_health.png");

    UI_data->player_health_back = gf2d_sprite_load_image("images/UI/health_back.png");
    UI_data->vortex_back = gf2d_sprite_load_image("images/UI/vortex_back.png");
    UI_data->progress_back = gf2d_sprite_load_image("images/UI/progress_back.png");
    UI_data->enemy_health_back = gf2d_sprite_load_image("images/UI/enemy_health_back.png");

    /*start menu*/
    x_start = (RES.x / 2.0f) - 100.0f;
    y_start = (RES.y / 2.0f) - 60.0f;
    UI_data->new_start_block = UPGRADE_BLOCK(x_start, y_start);

    y_start += 130.0;
    UI_data->continue_block = UPGRADE_BLOCK(x_start, y_start);

    y_start += 130.0;
    UI_data->s_quit_block = UPGRADE_BLOCK(x_start, y_start);

    /*player death screen / pause menu*/
    x_start = (RES.x / 2.0f) - 100.0f;
    y_start = (RES.y / 2.0f) - 60.0f;
    UI_data->resume_block = UPGRADE_BLOCK(x_start, y_start);

    y_start += 130.0f;
    UI_data->quit_block = UPGRADE_BLOCK(x_start, y_start);

    UI_data->nuke_alpha = 0.0f;
    UI_data->emper_alpha = 0.0f;

    UI_data->audio_flag = 1;

    atexit(UI_free);
}

void UI_free() {
    gf2d_sprite_free(UI_data->player_health);
    gf2d_sprite_free(UI_data->player_shield);
    gf2d_sprite_free(UI_data->player_scrap);
    gf2d_sprite_free(UI_data->player_vortex);
    gf2d_sprite_free(UI_data->progress_bar);
    gf2d_sprite_free(UI_data->enemy_health);

    gf2d_sprite_free(UI_data->player_health_back);
    gf2d_sprite_free(UI_data->vortex_back);
    gf2d_sprite_free(UI_data->progress_back);
    gf2d_sprite_free(UI_data->enemy_health_back);

    free(UI_data);
}

void shop_hud_draw() {
    GFC_Rect upgrade_bar;
    GFC_Vector2D bar_position, scale;
    float x_start, y_start, upgrade_bar_length, x1, x2, upgrade_cost, upgrade_length;
    float scrap, maxscrap, currScrap;
    PlayerData* data;

    data = get_player_data();

    if (!data) return;

    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), UI_data->shop_color);

    if (UI_data->shop_color_hue + 0.5f > 360)
        UI_data->shop_color_hue = 0;
    else {
        UI_data->shop_color_hue += 0.5f;
        UI_data->shop_color_hue = roundf(10 * UI_data->shop_color_hue) / 10;
    }

    gfc_color_set_hue(UI_data->shop_color_hue, &(UI_data->shop_color));
    gf2d_font_draw_line_tag("SHOP", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(605, 100));

    // scrap bar draw
    scrap = (float) data->currScrap;
    maxscrap = (float) data->maxScrap;
    currScrap = (float)(scrap / maxscrap);

    x_start = (RES.x / 2) - 200.0f;
    y_start = 150.0f;
    bar_position = gfc_vector2d(x_start, y_start);
    scale = gfc_vector2d(currScrap, 1);

    gf2d_sprite_draw_image(UI_data->player_health_back, bar_position);
    gf2d_sprite_draw(UI_data->player_scrap, bar_position, &scale, NULL, NULL, NULL, NULL, NULL, NULL);
    
    // next upgrade cost indicator
    x1 = x_start + (UI_data->player_scrap->frameWidth * currScrap);
    if (gf2d_mouse_in_rect(UI_data->nuke_block)) { // super_nuke cost
        upgrade_cost = (float)(maxscrap / (float)(UI_data->upgrade_cost * 5));
        upgrade_length = (float)(UI_data->player_health_back->frameWidth / upgrade_cost);
    }
    else { // regular nuke cost
        upgrade_cost = (float)(maxscrap / (float)UI_data->upgrade_cost);
        upgrade_length = (float)(UI_data->player_health_back->frameWidth / upgrade_cost);
    }
    x1 -= upgrade_length;
    if (x1 >= x_start) 
        gf2d_draw_rect_filled(gfc_rect(x1, y_start, upgrade_length, UI_data->player_health_back->frameHeight), GFC_COLOR_RED);
    
        // bar outline
    x_start = (RES.x / 2.0f) - 200.0f;
    bar_position = gfc_vector2d(x_start, y_start);
    gf2d_draw_rect(gfc_rect(bar_position.x, bar_position.y, UI_data->player_scrap->frameWidth, UI_data->player_scrap->frameHeight), GFC_COLOR_WHITE);
    gf2d_font_draw_line_tag("SCRAP", FT_H5, GFC_COLOR_WHITE, bar_position);

    
    // upgrade button draws
        // shields
    gf2d_draw_rect_filled(UI_data->shields_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("SHIELDS", FT_H2, GFC_COLOR_WHITE, UI_data->shields_block);
        // upgrade progress for shields
    x_start = UI_data->shields_block.x;
    y_start = (UI_data->shields_block.y + UI_data->shields_block.h) - 30.0f;
    x1 = (float) UI_data->shields_check;
    x2 = (float) UI_data->shields_max;
    upgrade_bar_length = (float) (200.0f * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0f);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);

        // more scrap
    gf2d_draw_rect_filled(UI_data->scrap_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("MORE SCRAP", FT_H2, GFC_COLOR_WHITE, UI_data->scrap_block);
        // upgrade progress for scrap up
    x_start = UI_data->scrap_block.x;
    y_start = (UI_data->scrap_block.y + UI_data->scrap_block.h) - 30.0f;
    x1 = (float) UI_data->more_scrap_check;
    x2 = (float) UI_data->more_scrap_max;
    upgrade_bar_length = (float) (200.0f * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0f);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);

        // missile up
    gf2d_draw_rect_filled(UI_data->missiles_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("MISSILES UP", FT_H3, GFC_COLOR_WHITE, UI_data->missiles_block);
        // upgrade progress for missile up
    x_start = UI_data->missiles_block.x;
    y_start = (UI_data->missiles_block.y + UI_data->missiles_block.h) - 30.0f;
    x1 = (float) UI_data->missiles_check;
    x2 = (float) UI_data->missiles_max;
    upgrade_bar_length = (float) (200.0f * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0f);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);

        // single shot up
    gf2d_draw_rect_filled(UI_data->single_shot_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("SINGLE_SHOT UP", FT_H3, GFC_COLOR_WHITE, UI_data->single_shot_block);
        // upgrade progress for single shot up
    x_start = UI_data->single_shot_block.x;
    y_start = (UI_data->single_shot_block.y + UI_data->single_shot_block.h) - 30.0f;
    x1 = (float)UI_data->single_shot_check;
    x2 = (float)UI_data->single_shot_max;
    upgrade_bar_length = (float) (200.0f * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0f);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);

        // charge shot up
    gf2d_draw_rect_filled(UI_data->charge_shot_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("CHARGE UP", FT_H2, GFC_COLOR_WHITE, UI_data->charge_shot_block);
    x_start = UI_data->charge_shot_block.x;
    y_start = (UI_data->charge_shot_block.y + UI_data->charge_shot_block.h) - 30.0f;
    x1 = (float)UI_data->charge_shot_check;
    x2 = (float)UI_data->charge_shot_max;
    upgrade_bar_length = (float) (200.0f * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0f);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);

        // super nuke up
    gf2d_draw_rect_filled(UI_data->nuke_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("NUKE COST DOWN", FT_H3, GFC_COLOR_WHITE, UI_data->nuke_block);
    x_start = UI_data->nuke_block.x;
    y_start = (UI_data->nuke_block.y + UI_data->nuke_block.h) - 30.0f;
    x1 = (float) UI_data->nuke_check;
    x2 = (float) UI_data->nuke_max;
    upgrade_bar_length = (float)(200.0f * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0f);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);
}

void shop_think() {
    LevelData* level;
    PlayerData* data;

    data = get_player_data();
    if (!data) return;

    level = get_level_data();
    if (!level->wave_end) return;

    if (gf2d_mouse_button_released(0)) {
        if (gf2d_mouse_in_rect(UI_data->shields_block)) {
            if (UI_data->shields_check >= UI_data->shields_max) {
                //slog("max amount of upgrades for shields");
                return;
            }

            if (data->currScrap >= UI_data->upgrade_cost) {
                data->maxShield += 100.0f;
                data->currShield = data->maxShield;
                data->currScrap -= UI_data->upgrade_cost;
                UI_data->shields_check++;
                UI_data->shields_count++;
                //slog("more shields");
            }
            //else
                //slog("not enough scrap");
        }
        else if (gf2d_mouse_in_rect(UI_data->scrap_block)) {
            if (UI_data->more_scrap_check >= UI_data->more_scrap_max) {
                //slog("max amount of upgrades for more scrap");
                return;
            }

            if (data->currScrap >= UI_data->upgrade_cost) {
                data->maxScrap += 5;
                data->currScrap -= UI_data->upgrade_cost;
                UI_data->more_scrap_check++;
                UI_data->more_scrap_count++;
                //slog("more scrap");
            }
            //else
                //slog("not enough scrap");
        }
        else if (gf2d_mouse_in_rect(UI_data->missiles_block)) {
            if (UI_data->missiles_check >= UI_data->missiles_max) {
                //slog("max amount of upgrades for missiles up");
                return;
            }
            else if (data->currScrap >= UI_data->upgrade_cost) {
                data->missile_bonus += 150.0f;
                data->currScrap -= UI_data->upgrade_cost;
                UI_data->missiles_check++;
                UI_data->missiles_count++;
                //slog("missile up");
            }
            //else
                //slog("not enough scrap");
        }
        else if (gf2d_mouse_in_rect(UI_data->single_shot_block)) {
            if (UI_data->single_shot_check >= UI_data->single_shot_max) {
                //slog("max amount of upgrades for more single shot up");
                return;
            }
            else if (data->currScrap >= UI_data->upgrade_cost) {
                data->single_shot_bonus += 50.0f;
                data->currScrap -= UI_data->upgrade_cost;
                UI_data->single_shot_check++;
                UI_data->single_shot_count++;
                //slog("more single");
            }
            //else
                //slog("not enough scrap");
        }
        else if (gf2d_mouse_in_rect(UI_data->charge_shot_block)) {
            if (UI_data->charge_shot_check >= UI_data->charge_shot_max) {
                //slog("max amount of upgrades for more single shot up");
                return;
            }
            else if (data->currScrap >= UI_data->upgrade_cost) {
                data->charge_shot_mult += 0.5f;
                data->currScrap -= UI_data->upgrade_cost;
                UI_data->charge_shot_check++;
                UI_data->charge_shot_count++;
                //slog("more charge");
            }
            //else
                //slog("not enough scrap");
        }
        else if (gf2d_mouse_in_rect(UI_data->nuke_block)) {
            if (UI_data->nuke_check >= UI_data->nuke_max) {
                //slog("max amount of upgrades for more single shot up");
                return;
            }
            else if (data->currScrap >= (UI_data->upgrade_cost * 5)) {
                data->nuke_cost -= 25;
                data->currScrap -= (UI_data->upgrade_cost * 5);
                UI_data->nuke_check++;
                //slog("nuke down");
            }
            //else
                //slog("not enough scrap");
        }
        gfc_sound_play(
            get_sound_data()->confirm,
            0,
            0.5,
            -1,
            -1);
    }
}

void shop_reset() {
    UI_data->shields_check = 0;
    UI_data->more_scrap_check = 0;
    UI_data->missiles_check = 0;
    UI_data->single_shot_check = 0;
    UI_data->charge_shot_check = 0;
    UI_data->shields_count = 0;
    UI_data->more_scrap_count = 0;
    UI_data->missiles_count = 0;
    UI_data->single_shot_count = 0;
    UI_data->charge_shot_count = 0;


    // don't reset nuke upgrade or upgrade cost
}

void player_hud(void* d) {
    GFC_Vector2D bar_position, scale;
    float start, scrap, maxscrap, nuke_cost;
    float currHealth, currShield, currScrap, currVortex, currNuke;
    float enemy_kill, currEnem;
    LevelData* level;
    PlayerData* data;

    data = (PlayerData*) d;
    if (!data) return;

    level = get_level_data();

    // progress calculations
    currHealth = (float) (data->currHealth / data->total_health_bar);
    currShield = (float) (data->currShield / data->total_health_bar);

    scrap = (float) data->currScrap;
    maxscrap = (float) data->maxScrap;

    currScrap = (float) (scrap / maxscrap);

    currVortex = (float) (data->vortex_dur / data->vortex_max);

    enemy_kill = (float) level->enemy_killed;
    currEnem = (float) (enemy_kill / ENEMY_GOAL);
    if (currEnem < 0)
        currEnem = 0;

    // health bar draws
    bar_position = gfc_vector2d(10, 20);
    gf2d_sprite_draw_image(UI_data->player_health_back, bar_position);

        // current health
    scale = gfc_vector2d(currHealth, 1);
    gf2d_sprite_draw(UI_data->player_health, bar_position, &scale, NULL, NULL, NULL, NULL, NULL, NULL);

        // current shield
    if (currShield > 0) {
        bar_position.x += (UI_data->player_health->frameWidth * currHealth);
        scale = gfc_vector2d(currShield, 1);
        gf2d_sprite_draw(UI_data->player_shield, bar_position, &scale, NULL, NULL, NULL, NULL, NULL, NULL);
    }
        // bar outline
    gf2d_draw_rect(gfc_rect(10, bar_position.y, UI_data->player_health_back->frameWidth, UI_data->player_health_back->frameHeight), GFC_COLOR_WHITE);
    gf2d_font_draw_line_tag("HEALTH", FT_H5, GFC_COLOR_WHITE, gfc_vector2d(15, 24));


    // scrap bar draws
        // current scrap
    bar_position = gfc_vector2d(10, 60);
    scale = gfc_vector2d(currScrap, 1);
    gf2d_sprite_draw_image(UI_data->player_health_back, bar_position);
    gf2d_sprite_draw(UI_data->player_scrap, bar_position, &scale, NULL, NULL, NULL, NULL, NULL, NULL);

        // super nuke cost draw
    if (data->currScrap >= data->maxScrap) {
        nuke_cost = (float) data->nuke_cost;
        currNuke = (float) (UI_data->player_scrap->frameWidth * (data->nuke_cost / maxscrap));
        gf2d_draw_rect_filled(
            gfc_rect(bar_position.x, bar_position.y, currNuke, UI_data->player_scrap->frameHeight),
            gfc_color(255, 0, 0, 0.3f));
    }
        // bar outline
    gf2d_draw_rect(gfc_rect(bar_position.x, bar_position.y, UI_data->player_scrap->frameWidth, UI_data->player_scrap->frameHeight), GFC_COLOR_WHITE);
    gf2d_font_draw_line_tag("SCRAP", FT_H5, GFC_COLOR_WHITE, gfc_vector2d(15, 64));

    // vortex bar draws
    bar_position = gfc_vector2d(10, 100);
    scale = gfc_vector2d(currVortex, 1);
    gf2d_sprite_draw_image(UI_data->vortex_back, bar_position);

    gf2d_sprite_draw(UI_data->player_vortex, bar_position, &scale, NULL, NULL, NULL, NULL, NULL, NULL);
    gf2d_draw_rect(gfc_rect(bar_position.x, bar_position.y, 200, 30), GFC_COLOR_WHITE);
    gf2d_font_draw_line_tag("VORTEX", FT_H5, GFC_COLOR_WHITE, gfc_vector2d(15, 104));

    // wave progress bar draws
    start = RES.x - 600.0f;
    bar_position = gfc_vector2d(start, 20);
    scale = gfc_vector2d(currEnem, 1);
    gf2d_sprite_draw_image(UI_data->progress_back, bar_position);
    
    gf2d_sprite_draw(UI_data->progress_bar, bar_position, &scale, NULL, NULL, NULL, NULL, NULL, NULL);
    gf2d_draw_rect(gfc_rect(bar_position.x, bar_position.y, UI_data->progress_bar->frameWidth, UI_data->progress_bar->frameHeight), GFC_COLOR_WHITE);
    gf2d_font_draw_line_tag("WAVE PROGRESS", FT_H5, GFC_COLOR_WHITE, gfc_vector2d(start + 5, 23));

    // power up notifs
    if (data->active_item == HAPPY_TRIGGER) 
        gf2d_font_draw_line_tag("HAPPY TRIGGER", FT_H4, GFC_COLOR_WHITE, gfc_vector2d(520, 300));
    else if (data->active_item == INVINCIBILITY)
        gf2d_font_draw_line_tag("INVICIBILITY", FT_H4, GFC_COLOR_WHITE, gfc_vector2d(520, 300));
    
    // visual for super nuke
    if (data->nuke_flag) {
        UI_data->nuke_alpha += 0.004f;
        if (UI_data->nuke_alpha > 1)
            UI_data->nuke_alpha = 1;

        gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(255, 0, 0, UI_data->nuke_alpha));
    }
    else {
        UI_data->nuke_alpha -= 0.05f;
        if (UI_data->nuke_alpha > 0)
            gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(255, 0, 0, UI_data->nuke_alpha));
        else
            UI_data->nuke_alpha = 0;
    }

    /*
    if (data->currMode == MISSILE) {
        entityList = get_entityList();
        for (i = 0; i < MAX_ENTITY; i++) {
            target = &entityList[i];

            if (target->entity_type != ENEMY)
                continue;

            enemy_data = target->data;
            if (enemy_data->missile_targeted) {
                target_pos = gfc_3DPos_to_2DPos(target->position, enemy_data->x_bound, enemy_data->z_bound);
                target_pos.x *= 0.9;
                target_pos.y *= 0.9;
                gf2d_draw_circle(target_pos, 20, GFC_COLOR_WHITE);
            }
        }
    }
    */
}

void start_menu() {
    GFC_Vector2D game_start_tag;

    game_start_tag = TEXT_LOCATION;
    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(65, 65, 65, 0.4f));
    gf2d_font_draw_line_tag("GAME START", FT_H1, GFC_COLOR_WHITE, game_start_tag);

    gf2d_draw_rect_filled(UI_data->new_start_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("NEW START", FT_H2, GFC_COLOR_WHITE, UI_data->new_start_block);

    gf2d_draw_rect_filled(UI_data->continue_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("CONTINUE", FT_H2, GFC_COLOR_WHITE, UI_data->continue_block);

    gf2d_draw_rect_filled(UI_data->s_quit_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("QUIT", FT_H2, GFC_COLOR_WHITE, UI_data->s_quit_block);
}

void start_menu_think() {
    WorldData* world;

    world = get_world_data();
    if (gf2d_mouse_button_released(0)) {
        if (gf2d_mouse_in_rect(UI_data->new_start_block)) {
            gfc_sound_play( get_sound_data()->confirm, 0, 1, -1, -1 );
            if (!world->entity_assets_made)
                entity_assets_init();
            //if (!level->asteroids_made)
                //asteroid_init();
            
            if (world->entity_assets_made) {
                gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), GFC_COLOR_BLACK);
                world->enemy_start = 0;
                world->player_spawned = 1;
            }
        }
        else if (gf2d_mouse_in_rect(UI_data->continue_block)) {

        }
        else if (gf2d_mouse_in_rect(UI_data->s_quit_block)) {
            world->_done = 1;
        } 
    }
}

void pause_menu() {
    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(65, 65, 65, 0.4f));

    gf2d_font_draw_line_tag("GAME PAUSED", FT_H1, GFC_COLOR_WHITE, TEXT_LOCATION);

    gf2d_draw_rect_filled(UI_data->resume_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("RESUME", FT_H2, GFC_COLOR_WHITE, UI_data->resume_block);

    gf2d_draw_rect_filled(UI_data->quit_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("QUIT", FT_H2, GFC_COLOR_WHITE, UI_data->quit_block);
}

void pause_menu_think(void* w) {
    WorldData* world;

    world = (WorldData*) w;
    if (!world) return;

    if (gf2d_mouse_button_released(0)) {
        if (gf2d_mouse_in_rect(UI_data->resume_block)) {
            world->current_state = IN_GAME;
            gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
        }
        else if (gf2d_mouse_in_rect(UI_data->quit_block)) {
            //game_save();
            level_reset();
            entity_despawn_all();
            entity_assets_close();
            world->current_state = START_MENU;
            world->player_spawned = 0;
            gfc_sound_play(get_sound_data()->cancel, 0, 1, -1, -1);
        }
    }
}

void wave_start() {
    GFC_Vector2D text_loc;
    WorldData* world;
    char buffer[8];

    if (gf2d_mouse_button_released(2)) {
        world = get_world_data();
        //world->enemy_start = 1;
        world->current_state = IN_GAME;

        gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
    }
    else {
        sprintf(buffer, "WAVE #%d", get_level_data()->wave_count);
        gf2d_font_draw_line_tag(buffer, FT_H1, GFC_COLOR_WHITE, TEXT_LOCATION);

        text_loc = TEXT_LOCATION;
        text_loc.y += 100.0f;
        gf2d_font_draw_line_tag("Right Click to Start", FT_H1, GFC_COLOR_WHITE, text_loc);
    }
}

void wave_completed() {
    GFC_Vector2D text_loc;
    PlayerData* player; 
    LevelData* level;

    player = get_player_data();
    level = get_level_data();

    if (!player || !level) return;

    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(65, 65, 65, 0.4f));
    gf2d_font_draw_line_tag("WAVE COMPLETE", FT_H1, GFC_COLOR_WHITE, TEXT_LOCATION);

    text_loc = TEXT_LOCATION;
    text_loc.y += 100.0f;
    gf2d_font_draw_line_tag("Press Right Click to Continue", FT_H2, GFC_COLOR_WHITE, text_loc);

    if (player->nuke_flag)
        player->nuke_flag = 0;

    // play victory theme
    if (!level->wave_end) {
        gfc_sound_play(get_sound_data()->victory, 0, 0.3f, -1, -1);
    }
    level->wave_end = 1;
}

void player_death_screen() {
    GFC_Vector2D text_loc;

    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(65, 65, 65, 0.4f));
    
    gf2d_font_draw_line_tag("YOU DIED", FT_H1, GFC_COLOR_WHITE, TEXT_LOCATION);
    
    text_loc = TEXT_LOCATION;
    text_loc.y += 100.0f;
    gf2d_font_draw_line_tag("Press Right Click to Restart", FT_H3, GFC_COLOR_WHITE, text_loc);
}

void enemy_hud(void* e, GFC_Vector3D position) {
    float health, maxhealth, currHealth;
    PlayerData* player_data;
    EnemyData* data;
    LevelData* level;
    GFC_Vector2D bar_position, scale, fencer_start;
    GFC_Rect fencer_attack;

    data = (EnemyData*) e;
    if (!data) return;
    
    player_data = get_player_data();
    level = get_level_data();

    health = data->currHealth;
    maxhealth = data->maxHealth;
    currHealth = (float)(health / maxhealth);

    bar_position = gfc_3DPos_to_2DPos(position, data->x_bound, data->z_bound);
    scale = gfc_vector2d(currHealth, 1);

    // bar offset (maybe testing the scaling for other res???)
    bar_position.x += 100.0f;
    bar_position.y += 50.0f;
    bar_position.x *= 0.8f;
    bar_position.y *= 0.8f;

    gf2d_sprite_draw_image(UI_data->enemy_health_back, bar_position);
    gf2d_sprite_draw(UI_data->enemy_health, bar_position, &scale, NULL, NULL, NULL, NULL, NULL, NULL);
    
    gf2d_draw_rect(gfc_rect(bar_position.x, bar_position.y, 100, 20), GFC_COLOR_WHITE);

    // draw fencer attack region
    if (level->fencer_flag) {
        fencer_start = gfc_3DPos_to_2DPos(level->fencer_spawn, player_data->x_bound, player_data->z_bound);
        fencer_start.x -= 360.0f;
        fencer_start.y -= 240.0f;
        fencer_attack = gfc_rect(fencer_start.x, fencer_start.y, 720.0f, 480.0f);
        gf2d_draw_rect_filled(fencer_attack, gfc_color(65, 65, 65, 0.1f));
    }
    
    // draw emper effect
    if (player_data->emp_time >= CURRENT_TIME && player_data->active_item != INVINCIBILITY) {
        UI_data->emper_alpha += 0.001f;
        if (UI_data->emper_alpha > 0.1f)
            UI_data->emper_alpha = 0.1f;

        gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(0, 0, 255, UI_data->emper_alpha));
    }
    else {
        UI_data->emper_alpha -= 0.02;
        if (UI_data->emper_alpha > 0.0)
            gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(0, 0, 255, UI_data->emper_alpha));
        else
            UI_data->emper_alpha = 0;
    }
}

void enemy_hud_all() {
    Entity* entityList, *enemy;
    int i;

    entityList = get_entityList();
    for (i = 0; i < MAX_ENTITY; i++) {
        enemy = &entityList[i];

        if (enemy->entity_type != ENEMY)
            continue;

        enemy_hud(enemy->data, enemy->position);
    }
}

UIData* get_UI_data() {
    return UI_data;
}