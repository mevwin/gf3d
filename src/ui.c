#include <math.h>
#include "simple_logger.h"
#include "gf3d_vgraphics.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gfc_vector.h"
#include "gf2d_mouse.h"
#include "ui.h"
#include "projectile.h"

static UIData* UI_data;

void UI_init() {
    GFC_Vector2D res;
    float x_start, y_start;

    UI_data = gfc_allocate_array(sizeof(UIData), 1);
    if (!UI_data) return;

    res = gf3d_vgraphics_get_resolution();

    /*shop UI*/
    UI_data->shop_color = gfc_color(0, 0, 1, 0.5);
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

    x_start = res.x / 4.0;
    y_start = res.y / 4.0;
    UI_data->shields_block = gfc_rect(x_start, y_start, 200.0, 100.0);

    y_start += 120.0;
    UI_data->scrap_block = gfc_rect(x_start, y_start, 200.0, 100.0);

    y_start += 120.0;
    UI_data->missiles_block = gfc_rect(x_start, y_start, 200.0, 100.0);

    y_start = res.y / 4.0;
    x_start = (res.x - (res.x / 4.0)) - 200.0;
    UI_data->single_shot_block = gfc_rect(x_start, y_start, 200.0, 100.0);

    y_start += 120.0;
    UI_data->charge_shot_block = gfc_rect(x_start, y_start, 200.0, 100.0);

    y_start += 120.0;
    UI_data->nuke_block = gfc_rect(x_start, y_start, 200.0, 100.0);

    /*start menu*/
    x_start = (res.x / 2) - 100.0;
    y_start = (res.y / 2) - 60.0;
    UI_data->start_block = gfc_rect(x_start, y_start, 200.0, 100.0);

    y_start += 130.0;
    UI_data->s_quit_block = gfc_rect(x_start, y_start, 200.0, 100.0);

    /*pause menu*/

    /*wave start*/
    
    /*wave completed*/

    /*player death screen*/
    // gfc_vector2d(res.x / 2 - 90.0, res.y / 2 - 20.0)
    x_start = (res.x / 2) - 100.0;
    y_start = (res.y / 2) - 60.0;
    UI_data->resume_block = gfc_rect(x_start, y_start, 200.0, 100.0);

    y_start += 130.0;
    UI_data->quit_block = gfc_rect(x_start, y_start, 200.0, 100.0);

    atexit(UI_free);
}

void UI_free() {
    free(UI_data);
}

void shop_hud_draw(PlayerData* data) {
    GFC_Rect upgrade_bar;
    GFC_Vector2D res;
    float x_start, y_start, upgrade_bar_length, x1, x2;
    float scrap, maxscrap, currScrap;
    int i, scrap_line_count, upgrade_length;

    res = gf3d_vgraphics_get_resolution();
    gf2d_draw_rect_filled(gfc_rect(0, 0, res.x, res.y), UI_data->shop_color);

    if (UI_data->shop_color_hue + 0.5 > 360.0)
        UI_data->shop_color_hue = 0.0;
    else {
        UI_data->shop_color_hue += 0.5;
        UI_data->shop_color_hue = roundf(10 * UI_data->shop_color_hue) / 10;
    }

    gfc_color_set_hue(UI_data->shop_color_hue, &(UI_data->shop_color));
    gf2d_font_draw_line_tag("SHOP", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(605.0, 100.0));

    // scrap bar draw
    x_start = (res.x / 2) - 200.0;
    y_start = 150.0;
    scrap_line_count = (int) (data->maxScrap / UI_data->upgrade_cost);
    if (data->maxScrap % UI_data->upgrade_cost != 0) scrap_line_count++;
    upgrade_length = (float) (400.0 / (float) scrap_line_count);

    scrap = (float)data->currScrap;
    maxscrap = (float)data->maxScrap;

    currScrap = (float)(400.0 * (scrap / maxscrap));
    currScrap = roundf(10 * currScrap) / 10;

    gf2d_draw_rect_filled(gfc_rect(x_start, y_start, 400.0, 30), GFC_COLOR_BLACK);
    gf2d_draw_rect_filled(gfc_rect(x_start, y_start, currScrap, 30), GFC_COLOR_GREY);

    for (i = scrap_line_count; i > 0; i--) {
        x1 = (int) 400.0 / scrap_line_count;
        gf2d_draw_rect(gfc_rect(x_start, y_start, upgrade_length, 30), GFC_COLOR_WHITE);
        x_start += x1;
    }
    x_start = (res.x / 2) - 200.0;
    gf2d_draw_rect(gfc_rect(x_start, y_start, 400, 30), GFC_COLOR_WHITE);
    //gf2d_font_draw_line_tag("SCRAP", FT_H5, GFC_COLOR_WHITE, gfc_vector2d(x_start, y_start));
    
    // upgrade button draws
        // shields
    gf2d_draw_rect_filled(UI_data->shields_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("SHIELDS", FT_H2, GFC_COLOR_WHITE, UI_data->shields_block);
        // upgrade progress for shields
    x_start = UI_data->shields_block.x;
    y_start = (UI_data->shields_block.y + UI_data->shields_block.h) - 30.0;
    x1 = (float) UI_data->shields_check;
    x2 = (float) UI_data->shields_max;
    upgrade_bar_length = (float) (200.0 * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);

        // more scrap
    gf2d_draw_rect_filled(UI_data->scrap_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("MORE SCRAP", FT_H2, GFC_COLOR_WHITE, UI_data->scrap_block);
        // upgrade progress for scrap up
    x_start = UI_data->scrap_block.x;
    y_start = (UI_data->scrap_block.y + UI_data->scrap_block.h) - 30.0;
    x1 = (float) UI_data->more_scrap_check;
    x2 = (float) UI_data->more_scrap_max;
    upgrade_bar_length = (float) (200.0 * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);

        // missile up
    gf2d_draw_rect_filled(UI_data->missiles_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("MISSILES UP", FT_H3, GFC_COLOR_WHITE, UI_data->missiles_block);
        // upgrade progress for missile up
    x_start = UI_data->missiles_block.x;
    y_start = (UI_data->missiles_block.y + UI_data->missiles_block.h) - 30.0;
    x1 = (float) UI_data->missiles_check;
    x2 = (float) UI_data->missiles_max;
    upgrade_bar_length = (float) (200.0 * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);

        // single shot up
    gf2d_draw_rect_filled(UI_data->single_shot_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("SINGLE_SHOT UP", FT_H3, GFC_COLOR_WHITE, UI_data->single_shot_block);
        // upgrade progress for single shot up
    x_start = UI_data->single_shot_block.x;
    y_start = (UI_data->single_shot_block.y + UI_data->single_shot_block.h) - 30.0;
    x1 = (float)UI_data->single_shot_check;
    x2 = (float)UI_data->single_shot_max;
    upgrade_bar_length = (float) (200.0 * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);

        // charge shot up
    gf2d_draw_rect_filled(UI_data->charge_shot_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("CHARGE UP", FT_H2, GFC_COLOR_WHITE, UI_data->charge_shot_block);
    x_start = UI_data->charge_shot_block.x;
    y_start = (UI_data->charge_shot_block.y + UI_data->charge_shot_block.h) - 30.0;
    x1 = (float)UI_data->charge_shot_check;
    x2 = (float)UI_data->charge_shot_max;
    upgrade_bar_length = (float)(200.0 * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);

    gf2d_draw_rect_filled(UI_data->nuke_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("NUKE COST DOWN", FT_H3, GFC_COLOR_WHITE, UI_data->nuke_block);
    x_start = UI_data->nuke_block.x;
    y_start = (UI_data->nuke_block.y + UI_data->nuke_block.h) - 30.0;
    x1 = (float) UI_data->nuke_check;
    x2 = (float) UI_data->nuke_max;
    upgrade_bar_length = (float)(200.0 * (x1 / x2));
    upgrade_bar = gfc_rect(x_start, y_start, upgrade_bar_length, 30.0);
    gf2d_draw_rect_filled(upgrade_bar, GFC_COLOR_LIGHTCYAN);
}

void shop_think(PlayerData* data) {
    if (!data) return;

    if (!data->wave_end) return;

    if (gf2d_mouse_button_released(2))
        data->in_shop = 0;

    if (gf2d_mouse_button_released(0)) {
        if (gf2d_mouse_in_rect(UI_data->shields_block)) {
            if (UI_data->shields_check >= UI_data->shields_max) {
                //slog("max amount of upgrades for shields");
                return;
            }

            if (data->currScrap >= UI_data->upgrade_cost) {
                data->maxShield += 100.0;
                data->currScrap -= UI_data->upgrade_cost;
                UI_data->shields_check++;
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
                data->missile_mult += 1.0;
                data->currScrap -= UI_data->upgrade_cost;
                UI_data->missiles_check++;
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
                data->single_shot_bonus += 50.0;
                data->currScrap -= UI_data->upgrade_cost;
                UI_data->single_shot_check++;
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
                data->charge_shot_mult += 3.0;
                data->currScrap -= UI_data->upgrade_cost;
                UI_data->charge_shot_check++;
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
                data->currScrap -= UI_data->upgrade_cost;
                UI_data->charge_shot_check++;
                //slog("nuke down");
            }
            //else
                //slog("not enough scrap");
        }
    }
}

void shop_reset() {
    UI_data->shields_check = 0;
    UI_data->more_scrap_check = 0;
    UI_data->missiles_check = 0;
    UI_data->single_shot_check = 0;
    UI_data->charge_shot_check = 0;
    UI_data->upgrade_cost = 5;

    // don't reset nuke upgrade
}

/*
ShopData* get_UI_data() {
    return UI_data;
}
*/

void player_hud(PlayerData* data) {
    GFC_Vector2D res;
    float start, scrap, maxscrap, upgrade_length;
    float currHealth, currShield, currScrap, shieldStart, currVortex;
    float enemy_kill, enemy_goal, currEnem;
    int scrap_line_count, new_x, i;

    if (!data) return;
    if (data->player_dead || data->in_shop) return;
    
    res = gf3d_vgraphics_get_resolution();

    currHealth = (float) (400.0 * (data->currHealth / data->total_health_bar));
    currHealth = roundf(10 * currHealth) / 10;

    currShield = (float) (400.0 * (data->currShield / data->total_health_bar));
    currShield = roundf(10 * currShield) / 10;

    scrap = (float)data->currScrap;
    maxscrap = (float)data->maxScrap;

    currScrap = (float) (400.0 * (scrap / maxscrap));
    currScrap = roundf(10 * currScrap) / 10;

    currVortex = (float) (200.0 * (data->vortex_dur / data->vortex_max));
    currVortex = roundf(10 * currVortex) / 10;

    shieldStart = 10.0 + currHealth;

    enemy_kill = (float) enemy_killed;
    enemy_goal = 50.0;

    currEnem = (float)(500.0 * (enemy_kill / enemy_goal));
    currEnem = roundf(10 * currEnem) / 10;

    // health bar draws
    gf2d_draw_rect_filled(gfc_rect(10, 20, 400.0, 30), GFC_COLOR_BLACK);

        // current health
    gf2d_draw_rect_filled(gfc_rect(10, 20, currHealth, 30), GFC_COLOR_RED);

        // current shield
    gf2d_draw_rect_filled(gfc_rect(shieldStart, 20, currShield, 30), GFC_COLOR_DARKBLUE);

        // bar outline
    gf2d_draw_rect(gfc_rect(10, 20, 400, 30), GFC_COLOR_WHITE);
    gf2d_font_draw_line_tag("HEALTH", FT_H5, GFC_COLOR_WHITE, gfc_vector2d(15, 23));


    // scrap bar draws
        // current scrap
    gf2d_draw_rect_filled(gfc_rect(10, 60, 400.0, 30), GFC_COLOR_BLACK);
    gf2d_draw_rect_filled(gfc_rect(10, 60, currScrap, 30), GFC_COLOR_GREY);

        // bar outline
    start = 10.0;
    scrap_line_count = (int) (data->maxScrap / UI_data->upgrade_cost);
    if (data->maxScrap % UI_data->upgrade_cost != 0) scrap_line_count++;
    upgrade_length = (float) (400.0 / (float) scrap_line_count);

    for (i = scrap_line_count; i > 0; i--) {
        new_x = (int) 400.0 / scrap_line_count;
        gf2d_draw_rect(gfc_rect(start, 60, upgrade_length, 30), GFC_COLOR_WHITE);
        start += new_x;
    }
    gf2d_draw_rect(gfc_rect(10, 60, 400, 30), GFC_COLOR_WHITE);
    gf2d_font_draw_line_tag("SCRAP", FT_H5, GFC_COLOR_WHITE, gfc_vector2d(15, 63));


    // vortex bar draws
    gf2d_draw_rect_filled(gfc_rect(10, 100, 200.0, 30), GFC_COLOR_BLACK);
    gf2d_draw_rect_filled(gfc_rect(10, 100, currVortex, 30), GFC_COLOR_GREEN);
    gf2d_draw_rect(gfc_rect(10, 100, 200, 30), GFC_COLOR_WHITE);
    gf2d_font_draw_line_tag("VORTEX", FT_H5, GFC_COLOR_WHITE, gfc_vector2d(15, 103));

    // wave progress bar draws
    start = res.x - 600.0;
    gf2d_draw_rect_filled(gfc_rect(start, 20, 500.0, 30), GFC_COLOR_BLACK);
    gf2d_draw_rect_filled(gfc_rect(start, 20, currEnem, 30), GFC_COLOR_BLUE);
    gf2d_draw_rect(gfc_rect(start, 20, 500, 30), GFC_COLOR_WHITE);
    gf2d_font_draw_line_tag("WAVE PROGRESS", FT_H5, GFC_COLOR_WHITE, gfc_vector2d(start + 5, 23));

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
    GFC_Vector2D res;

    res = gf3d_vgraphics_get_resolution();
    gf2d_draw_rect_filled(gfc_rect(0, 0, res.x, res.y), gfc_color(65, 65, 65, 0.4));
    gf2d_font_draw_line_tag("GAME START", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(res.x / 2 - 90.0, res.y / 2 - 400.0));

    gf2d_draw_rect_filled(UI_data->start_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("START", FT_H2, GFC_COLOR_WHITE, UI_data->start_block);

    gf2d_draw_rect_filled(UI_data->s_quit_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("QUIT", FT_H2, GFC_COLOR_WHITE, UI_data->s_quit_block);
}

Entity* start_menu_think() {
    if (gf2d_mouse_button_released(0)) {
        if (gf2d_mouse_in_rect(UI_data->start_block)) {
            game_start = 1;
            return player_spawn();
        }
        if (gf2d_mouse_in_rect(UI_data->s_quit_block)) {
            _done = 1;
            return NULL;
        } 
    }
}

void pause_menu() {
    GFC_Vector2D res;

    res = gf3d_vgraphics_get_resolution();
    gf2d_draw_rect_filled(gfc_rect(0, 0, res.x, res.y), gfc_color(65, 65, 65, 0.4));

    gf2d_font_draw_line_tag("GAME PAUSED", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(res.x / 2 - 90.0, res.y / 2 - 100.0));

    gf2d_draw_rect_filled(UI_data->resume_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("RESUME", FT_H2, GFC_COLOR_WHITE, UI_data->resume_block);

    gf2d_draw_rect_filled(UI_data->quit_block, GFC_COLOR_GREY);
    gf2d_font_draw_text_wrap_tag("QUIT", FT_H2, GFC_COLOR_WHITE, UI_data->quit_block);
}

void pause_menu_think(PlayerData* data) {
    if (!data) return;

    if (gf2d_mouse_button_released(0)) {
        if (gf2d_mouse_in_rect(UI_data->resume_block))
            data->paused = 0;
        if (gf2d_mouse_in_rect(UI_data->quit_block)) {
            entity_despawn_all();
            enemy_count = 0;
            enemy_killed = 0;
            game_start = 0;
        }
    }
}

void wave_start(PlayerData* data) {
    GFC_Vector2D res;

    if (!data) return;

    res = gf3d_vgraphics_get_resolution();

    if (gf2d_mouse_button_released(2)) {
        enemy_start = 1;
        data->wave_end = 0;
    }
    else {
        gf2d_font_draw_line_tag("WAVE START", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(res.x / 2 - 90.0, res.y / 2));
        gf2d_font_draw_line_tag("Right Click to Start", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(res.x / 2 - 120.0, res.y / 2 - 60.0));
    }
}

void wave_completed(PlayerData* data) {
    GFC_Vector2D res;

    res = gf3d_vgraphics_get_resolution();
    gf2d_draw_rect_filled(gfc_rect(0, 0, res.x, res.y), gfc_color(65, 65, 65, 0.4));
    gf2d_font_draw_line_tag("WAVE COMPLETE", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(res.x / 2 - 90.0, res.y / 2 - 100.0));
    gf2d_font_draw_line_tag("Press Right Click to Continue", 
        FT_H3, GFC_COLOR_WHITE, gfc_vector2d(res.x / 2 - 90.0, res.y / 2 - 60.0));

    data->wave_end = 1;

    if (gf2d_mouse_button_released(2)) {
        player_upgrade(data);
        shop_reset();
        wave_count++;
        data->in_shop = 1;
        enemy_start = 0;
        enemy_count = 0;
        enemy_killed = 0;
    }
}

void player_death_screen(PlayerData* data) {
    GFC_Vector2D res;

    res = gf3d_vgraphics_get_resolution();
    gf2d_draw_rect_filled(gfc_rect(0, 0, res.x, res.y), gfc_color(65, 65, 65, 0.4));
    
    gf2d_font_draw_line_tag("YOU DIED", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(res.x / 2 - 90.0, res.y / 2 - 100.0));
    
    gf2d_font_draw_line_tag("Press Right Click to Restart",
        FT_H3, GFC_COLOR_WHITE, gfc_vector2d(res.x / 2 - 90.0, res.y / 2 - 60.0));
}

void enemy_hud(EnemyData* data, GFC_Vector3D position) {
    float health, maxhealth, currHealth;
    PlayerData* player_data;
    GFC_Vector2D bar_position;

    if (!data) return;
    
    player_data = get_player_data();
    if (player_data->player_dead || player_data->in_shop || data->currHealth <= 0.0) return;

    health = data->currHealth;
    maxhealth = data->maxHealth;
    currHealth = (float)( 100.0 * (health / maxhealth));
    currHealth = roundf(10 * currHealth) / 10;

    bar_position = gfc_3DPos_to_2DPos(position, data->x_bound, data->z_bound);
    
    // bar offset (maybe testing the scaling for other res???)
    bar_position.x += 100.0;
    bar_position.y += 50.0;
    bar_position.x *= 0.8;
    bar_position.y *= 0.8;

    gf2d_draw_rect_filled(gfc_rect(bar_position.x, bar_position.y, 100.0, 20), GFC_COLOR_BLACK);
    gf2d_draw_rect_filled(gfc_rect(bar_position.x, bar_position.y, currHealth, 20), GFC_COLOR_DARKBLUE);
    gf2d_draw_rect(gfc_rect(bar_position.x, bar_position.y, 100.0, 20), GFC_COLOR_WHITE);
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

