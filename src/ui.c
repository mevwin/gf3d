#include "simple_logger.h"
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

static UIData* UI_data;

void UI_init() {
    SJson* position_data, *dimen_data;
    float x_start, y_start, width, height;

    UI_data = gfc_allocate_array(sizeof(UIData), 1);
    if (!UI_data) return;

    /*shop UI*/
    UI_data->shop = gf2d_sprite_load_image("images/UI/shop/shop.png");
    UI_data->shop_data = sj_load("menus/shop.menu");
        
        // next wave button
    position_data = sj_object_get_value(UI_data->shop_data, "next_wave");
    sj_object_get_value_as_float(position_data, "x_offset", &x_start);
    sj_object_get_value_as_float(position_data, "y_offset", &y_start);
    sj_object_get_value_as_float(position_data, "width", &width);
    sj_object_get_value_as_float(position_data, "height", &height);
    UI_data->next_wave_block = gfc_rect(x_start, y_start, width, height);

        // upgrades section
    position_data = sj_object_get_value(UI_data->shop_data, "upgrades");
    dimen_data = sj_object_get_value(position_data, "offsets");

    sj_object_get_value_as_float(position_data, "width", &width);
    sj_object_get_value_as_float(position_data, "height", &height);

    sj_object_get_value_as_float(dimen_data, "column1", &x_start);
    sj_object_get_value_as_float(dimen_data, "row1", &y_start);
    UI_data->shields_block = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(dimen_data, "row2", &y_start);
    UI_data->scrap_block = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(dimen_data, "row3", &y_start);
    UI_data->missiles_block = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(dimen_data, "column2", &x_start);
    sj_object_get_value_as_float(dimen_data, "row1", &y_start);
    UI_data->single_shot_block = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(dimen_data, "row2", &y_start);
    UI_data->charge_shot_block = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(dimen_data, "row3", &y_start);
    UI_data->nuke_block = gfc_rect(x_start, y_start, width, height);

            // upgrade caps
    dimen_data = sj_object_get_value(position_data, "upgrade_caps");
    sj_object_get_value_as_uint8(dimen_data, "shields", &UI_data->shields_max);
    sj_object_get_value_as_uint8(dimen_data, "scrap", &UI_data->more_scrap_max);
    sj_object_get_value_as_uint8(dimen_data, "missile", &UI_data->missiles_max);
    sj_object_get_value_as_uint8(dimen_data, "single_shot", &UI_data->single_shot_max);
    sj_object_get_value_as_uint8(dimen_data, "charge_shot", &UI_data->charge_shot_max);
    sj_object_get_value_as_uint8(dimen_data, "nuke", &UI_data->nuke_max);
    sj_object_get_value_as_uint8(position_data, "upgrade_cost_default", &UI_data->upgrade_cost);

    UI_data->shields_check = 0;
    UI_data->more_scrap_check = 0;
    UI_data->missiles_check = 0;
    UI_data->single_shot_check = 0;
    UI_data->charge_shot_check = 0;
    UI_data->nuke_check = 0;

    position_data = sj_object_get_value(UI_data->shop_data, "scrap_bar");
    sj_object_get_value_as_float(position_data, "x_offset", &x_start);
    sj_object_get_value_as_float(position_data, "y_offset", &y_start);
    sj_object_get_value_as_float(position_data, "width", &width);
    sj_object_get_value_as_float(position_data, "height", &height);
    UI_data->scrap_bar = gfc_rect(x_start, y_start, width, height);

        // perks section
            // current perks
    position_data = sj_object_get_value(UI_data->shop_data, "curr_perks");
    sj_object_get_value_as_float(position_data, "y_offset", &y_start);
    sj_object_get_value_as_float(position_data, "width", &width);
    sj_object_get_value_as_float(position_data, "height", &height);

    sj_object_get_value_as_float(position_data, "p1x_offset", &x_start);
    UI_data->curr_perk1 = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(position_data, "p2x_offset", &x_start);
    UI_data->curr_perk1 = gfc_rect(x_start, y_start, width, height);

            // new perks
    position_data = sj_object_get_value(UI_data->shop_data, "new_perks");
    sj_object_get_value_as_float(position_data, "x_offset", &x_start);
    sj_object_get_value_as_float(position_data, "width", &width);
    sj_object_get_value_as_float(position_data, "height", &height);

    sj_object_get_value_as_float(position_data, "p1y_offset", &y_start);
    UI_data->curr_perk1 = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(position_data, "p2y_offset", &y_start);
    UI_data->curr_perk1 = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(position_data, "p3y_offset", &y_start);
    UI_data->curr_perk1 = gfc_rect(x_start, y_start, width, height);


    /*player UI*/
    UI_data->player_hud_data = sj_load("menus/player_hud.menu");
    UI_data->player_hud = gf2d_sprite_load_image("images/UI/player_hud/player_hud.png");
    
    UI_data->player_health = gf2d_sprite_load_image("images/UI/player_hud/player_health.png");
    UI_data->player_shield = gf2d_sprite_load_image("images/UI/player_hud/player_shield.png");
    UI_data->player_scrap = gf2d_sprite_load_image("images/UI/player_hud/player_scrap.png");
    UI_data->player_vortex = gf2d_sprite_load_image("images/UI/player_hud/player_vortex.png");

    UI_data->progress_bar = gf2d_sprite_load_image("images/UI/progress.png");
    UI_data->enemy_health = gf2d_sprite_load_image("images/UI/enemy_health.png");
    UI_data->enemy_health_back = gf2d_sprite_load_image("images/UI/enemy_health_back.png");


    /*start menu*/
    UI_data->start_menu = gf2d_sprite_load_image("images/UI/start_menu/start_menu.png");
    UI_data->start_menu_data = sj_load("menus/start_menu.menu");

    position_data = sj_object_get_value(UI_data->start_menu_data, "option_pos");
    sj_object_get_value_as_float(position_data, "block_x", &x_start);

    dimen_data = sj_object_get_value(UI_data->start_menu_data, "menu_block");
    sj_object_get_value_as_float(dimen_data, "width", &width);
    sj_object_get_value_as_float(dimen_data, "height", &height);

    sj_object_get_value_as_float(position_data, "new_game_y", &y_start);
    UI_data->new_start_block = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(position_data, "continue_y", &y_start);
    UI_data->continue_block = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(position_data, "previous_y", &y_start);
    UI_data->previous_block = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(position_data, "s_quit_y", &y_start);
    UI_data->s_quit_block = gfc_rect(x_start, y_start, width, height);


    /*pause menu*/
    UI_data->pause_menu = gf2d_sprite_load_image("images/UI/pause_menu/pause_menu.png");
    UI_data->pause_menu_data = sj_load("menus/pause_menu.menu");

        // menu blocks
    position_data = sj_object_get_value(UI_data->pause_menu_data, "option_pos");
    sj_object_get_value_as_float(position_data, "block_x", &x_start);

    dimen_data = sj_object_get_value(UI_data->pause_menu_data, "menu_block");
    sj_object_get_value_as_float(dimen_data, "width", &width);
    sj_object_get_value_as_float(dimen_data, "height", &height);

    sj_object_get_value_as_float(position_data, "resume_y", &y_start);
    UI_data->resume_block = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(position_data, "quit_y", &y_start);
    UI_data->quit_block = gfc_rect(x_start, y_start, width, height);

        // perks and item progress are calculated in pause_menu()


    /*wave start*/
    UI_data->wave_start = gf2d_sprite_load_image("images/UI/wave_start/wave_start.png");
    UI_data->wave_start_data = sj_load("menus/wave_start.menu");

    position_data = sj_object_get_value(UI_data->wave_start_data, "wave_text");
    sj_object_get_value_as_float(position_data, "x_offset", &x_start);
    sj_object_get_value_as_float(position_data, "y_offset", &y_start);
    UI_data->curr_wave_loc = gfc_vector2d(x_start, y_start);


    /*wave completed*/
    UI_data->wave_completed = gf2d_sprite_load_image("images/UI/wave_completed/wave_completed.png");
    UI_data->wave_completed_data = sj_load("menus/wave_completed.menu");

    position_data = sj_object_get_value(UI_data->wave_completed_data, "stage_blocks");
    sj_object_get_value_as_float(position_data, "width", &width);
    sj_object_get_value_as_float(position_data, "height", &height);
    
    sj_object_get_value_as_float(position_data, "y_offset", &y_start);

    sj_object_get_value_as_float(position_data, "x_offset1", &x_start);
    UI_data->stage_block1 = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(position_data, "x_offset2", &x_start);
    UI_data->stage_block2 = gfc_rect(x_start, y_start, width, height);


    /*game over (very similar to pause menu)*/
    UI_data->game_over = gf2d_sprite_load_image("images/UI/game_over/game_over.png");
    UI_data->game_over_data = sj_load("menus/game_over.menu");

        // menu blocks
    position_data = sj_object_get_value(UI_data->game_over_data, "option_pos");
    sj_object_get_value_as_float(position_data, "block_x", &x_start);

    dimen_data = sj_object_get_value(UI_data->game_over_data, "menu_block");
    sj_object_get_value_as_float(dimen_data, "width", &width);
    sj_object_get_value_as_float(dimen_data, "height", &height);

    sj_object_get_value_as_float(position_data, "respawn_y", &y_start);
    UI_data->respawn_block = gfc_rect(x_start, y_start, width, height);

    sj_object_get_value_as_float(position_data, "g_quit_y", &y_start);
    UI_data->g_quit_block = gfc_rect(x_start, y_start, width, height);

    UI_data->nuke_alpha = 0.0f;
    UI_data->emper_alpha = 0.0f;

    atexit(UI_free);
}

void UI_free() {
    sj_free(UI_data->player_hud_data);
    sj_free(UI_data->start_menu_data);
    sj_free(UI_data->pause_menu_data);
    sj_free(UI_data->wave_start_data);
    sj_free(UI_data->wave_completed_data);
    sj_free(UI_data->game_over_data);
    sj_free(UI_data->shop_data);
    gf2d_sprite_free(UI_data->player_hud);
    gf2d_sprite_free(UI_data->start_menu);
    gf2d_sprite_free(UI_data->pause_menu);
    gf2d_sprite_free(UI_data->wave_start);
    gf2d_sprite_free(UI_data->wave_completed);
    gf2d_sprite_free(UI_data->game_over);
    gf2d_sprite_free(UI_data->shop);
    gf2d_sprite_free(UI_data->player_health);
    gf2d_sprite_free(UI_data->player_shield);
    gf2d_sprite_free(UI_data->player_scrap);
    gf2d_sprite_free(UI_data->player_vortex);
    gf2d_sprite_free(UI_data->progress_bar);
    gf2d_sprite_free(UI_data->enemy_health);
    gf2d_sprite_free(UI_data->enemy_health_back);

    free(UI_data);
}

void shop_hud_draw() {
    SJson* data_entry;
    float height, progress, max, current;
    LevelData* level;
    PlayerData* p_data;
    GFC_Rect dummy, dummy2;

    level = get_level_data();
    p_data = get_player_data();

    gf2d_sprite_draw_image(UI_data->shop, gfc_vector2d(0, 0));

    // upgrades section
    data_entry = sj_object_get_value(UI_data->shop_data, "upgrades");
    sj_object_get_value_as_float(data_entry, "upgrade_progress_height", &height);

        // scrap bar
    progress = (float) p_data->currScrap;
    max = (float) p_data->maxScrap;
    current = (float) (progress / max);
    gfc_rect_copy(dummy, UI_data->scrap_bar);
    dummy.w = UI_data->scrap_bar.w * current;

    gf2d_draw_rect_filled(dummy, GFC_COLOR_LIGHTBLUE);

        // show upgrade cost on scrap bar
    if (p_data->currScrap >= UI_data->upgrade_cost) {
        if (gf2d_mouse_in_rect(UI_data->shields_block) ||
            gf2d_mouse_in_rect(UI_data->scrap_block) ||
            gf2d_mouse_in_rect(UI_data->missiles_block) ||
            gf2d_mouse_in_rect(UI_data->single_shot_block) ||
            gf2d_mouse_in_rect(UI_data->charge_shot_block) ||
            gf2d_mouse_in_rect(UI_data->nuke_block)
            ) {
            gfc_rect_copy(dummy2, UI_data->scrap_bar);

            progress = (float) UI_data->upgrade_cost;
            if (gf2d_mouse_in_rect(UI_data->nuke_block)) {
                if (p_data->currScrap >= (UI_data->upgrade_cost * 5))
                    progress = (float)(UI_data->upgrade_cost * 5);
                else
                    progress = 0;
            }

            max = (float)p_data->maxScrap;
            current = (float)(progress / max);
            dummy2.x += (dummy.w - (dummy2.w * current));
            dummy2.w *= current;
            gf2d_draw_rect_filled(dummy2, GFC_COLOR_LIGHTRED);
        }
    }

        // upgrade progress (only draw if at least one upgrade active)
    if (UI_data->shields_check > 0) {
        gfc_rect_copy(dummy, UI_data->shields_block);
        dummy.y += (dummy.h - height);
        dummy.h = height;

        progress = (float) UI_data->shields_check;
        max = (float) UI_data->shields_max;
        current = (float) (progress / max);

        dummy.w = UI_data->shields_block.w * current;
        
        gf2d_draw_rect_filled(dummy, GFC_COLOR_LIGHTBLUE);
    }
    if (UI_data->more_scrap_check > 0) {
        gfc_rect_copy(dummy, UI_data->scrap_block);
        dummy.y += (dummy.h - height);
        dummy.h = height;

        progress = (float) UI_data->more_scrap_check;
        max = (float) UI_data->more_scrap_max;
        current = (float) (progress / max);
        dummy.w = UI_data->scrap_block.w * current;
        gf2d_draw_rect_filled(dummy, GFC_COLOR_LIGHTBLUE);
    }
    if (UI_data->missiles_check > 0) {
        gfc_rect_copy(dummy, UI_data->missiles_block);
        dummy.y += (dummy.h - height);
        dummy.h = height;

        progress = (float)UI_data->missiles_check;
        max = (float)UI_data->missiles_max;
        current = (float)(progress / max);
        dummy.w = UI_data->missiles_block.w * current;
        gf2d_draw_rect_filled(dummy, GFC_COLOR_LIGHTBLUE);
    }
    if (UI_data->single_shot_check > 0) {
        gfc_rect_copy(dummy, UI_data->single_shot_block);
        dummy.y += (dummy.h - height);
        dummy.h = height;

        progress = (float)UI_data->single_shot_check;
        max = (float)UI_data->single_shot_max;
        current = (float)(progress / max);
        dummy.w = UI_data->single_shot_block.w * current;
        gf2d_draw_rect_filled(dummy, GFC_COLOR_LIGHTBLUE);
    }
    if (UI_data->charge_shot_check > 0) {
        gfc_rect_copy(dummy, UI_data->charge_shot_block);
        dummy.y += (dummy.h - height);
        dummy.h = height;

        progress = (float)UI_data->charge_shot_check;
        max = (float)UI_data->charge_shot_max;
        current = (float)(progress / max);
        dummy.w = UI_data->charge_shot_block.w * current;
        gf2d_draw_rect_filled(dummy, GFC_COLOR_LIGHTBLUE);
    }
    if (UI_data->nuke_check > 0) {
        gfc_rect_copy(dummy, UI_data->nuke_block);
        dummy.y += (dummy.h - height);
        dummy.h = height;

        progress = (float)UI_data->nuke_check;
        max = (float)UI_data->nuke_max;
        current = (float)(progress / max);
        dummy.w = UI_data->nuke_block.w * current;
        gf2d_draw_rect_filled(dummy, GFC_COLOR_LIGHTBLUE);
    }

    // perks (TODO)

}

void shop_think() {
    LevelData* level;
    PlayerData* data;

    data = get_player_data();
    if (!data) return;

    level = get_level_data();
    if (!level->wave_end) return; // only allow buying during the end of the wave

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
    SJson* position_data, *dimen_data;
    float scrap, maxscrap, nuke_cost, x, y, bar_length;
    float currHealth, currShield, currScrap, currVortex, currNuke;
    //float enemy_kill, currEnem;
    LevelData* level;
    PlayerData* data;

    data = (PlayerData*) d;
    if (!data) return;

    level = get_level_data();

    // progress calculations (calculates percentages of each bar)
    currHealth = (float) (data->currHealth / data->total_health_bar);
    currShield = (float) (data->currShield / data->total_health_bar);

    scrap = (float) data->currScrap;
    maxscrap = (float) data->maxScrap;
    currScrap = (float) (scrap / maxscrap);

    currVortex = (float) (data->vortex_dur / data->vortex_max);

    //enemy_kill = (float) level->enemy_killed;
    //currEnem = (float) (enemy_kill / ENEMY_GOAL);
    //if (currEnem < 0)
       // currEnem = 0;

    gf2d_sprite_draw_image(UI_data->player_hud, gfc_vector2d(0, 0));
    position_data = sj_object_get_value(UI_data->player_hud_data, "bar_start_positions");
    dimen_data = sj_object_get_value(UI_data->player_hud_data, "bar_dimensions");

    sj_object_get_value_as_float(position_data, "bar_x", &x);

    // health bar draws
        // current health
    sj_object_get_value_as_float(position_data, "health_y", &y);
    bar_position = gfc_vector2d(x, y);


    scale = gfc_vector2d(currHealth, 1.0f);
    gf2d_sprite_draw(UI_data->player_health, bar_position, &scale, 
                    NULL, NULL, NULL, NULL, NULL, NULL);

        // current shield
    sj_object_get_value_as_float(dimen_data, "player_bars_w", &bar_length);

    if (currShield > 0) {
        bar_position.x += (bar_length * currHealth);
        scale = gfc_vector2d(currShield, 1.0f);
        gf2d_sprite_draw(UI_data->player_shield, bar_position, &scale, 
                    NULL, NULL, NULL, NULL, NULL, NULL);
    }

    // scrap bar draws
        // current scrap
    sj_object_get_value_as_float(position_data, "scrap_y", &y);
    bar_position = gfc_vector2d(x, y);
    scale = gfc_vector2d(currScrap, 1.0f);
    gf2d_sprite_draw(UI_data->player_scrap, bar_position, &scale, 
                    NULL, NULL, NULL, NULL, NULL, NULL);

        // super nuke cost draw
    if (data->currScrap >= data->maxScrap) {
        nuke_cost = (float) data->nuke_cost;
        currNuke = (float) (bar_length * (data->nuke_cost / maxscrap));
        gf2d_draw_rect_filled(
            gfc_rect(bar_position.x, bar_position.y, currNuke, UI_data->player_scrap->frameHeight),
            gfc_color(255, 0, 0, 0.3f));
    }

    // vortex bar draws
    sj_object_get_value_as_float(position_data, "vortex_y", &y);
    bar_position = gfc_vector2d(x, y);
    scale = gfc_vector2d(currVortex, 1.0f);

    gf2d_sprite_draw(UI_data->player_vortex, bar_position, &scale, 
                    NULL, NULL, NULL, NULL, NULL, NULL);

    // wave progress bar draws
    /*
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
    */

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
    gf2d_sprite_draw_image(UI_data->start_menu, gfc_vector2d(0, 0));
}

void pause_menu(Sprite* menu, SJson* data) {
    SJson* data_entry;
    float x, y, width, height;
    float progress, goal;
    LevelData* level;
    PlayerData* p_data;

    level = get_level_data();
    p_data = get_player_data();

    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(65, 65, 65, 0.4f));
    gf2d_sprite_draw_image(menu, gfc_vector2d(0, 0));

    // progress bar draw
    data_entry = sj_object_get_value(data, "progress_bar");
    sj_object_get_value_as_float(data_entry, "x_offset", &x);
    sj_object_get_value_as_float(data_entry, "y_offset", &y);
    sj_object_get_value_as_float(data_entry, "width", &width);
    sj_object_get_value_as_float(data_entry, "height", &height);

        // check objective type
    switch (level->obj_type){   
        case KILL_ENEMY:
            progress = (float) level->enemy_killed;
            goal = (float) level->enemy_goal;
            break;
        case SURVIVE:

            break;
    }

    width *= (progress / goal);
    gf2d_draw_rect_filled(gfc_rect(x, y, width, height), GFC_COLOR_LIGHTBLUE);

    sj_object_get_value_as_float(data_entry, "text_x_offset", &x);
    sj_object_get_value_as_float(data_entry, "text_y_offset", &y);
    gf2d_font_draw_line_tag(level->level_obj, FT_Large, GFC_COLOR_WHITE, gfc_vector2d(x, y));

    // upgrade progress draws
    data_entry = sj_object_get_value(data, "item_progress_bar");
    sj_object_get_value_as_float(data_entry, "width", &width);
    sj_object_get_value_as_float(data_entry, "height", &height);

        // shields
    sj_object_get_value_as_float(data_entry, "column_1", &x);
    sj_object_get_value_as_float(data_entry, "row_1", &y);
    progress = (float) UI_data->shields_check;
    goal = (float) UI_data->shields_max;

    gf2d_draw_rect_filled(gfc_rect(x, y, width * (progress / goal), height), GFC_COLOR_LIGHTBLUE);

        // scrap up
    sj_object_get_value_as_float(data_entry, "column_2", &x);
    sj_object_get_value_as_float(data_entry, "row_1_b", &y);
    progress = (float) UI_data->more_scrap_check;
    goal = (float) UI_data->more_scrap_max;

    gf2d_draw_rect_filled(gfc_rect(x, y, width * (progress / goal), height), GFC_COLOR_LIGHTBLUE);

        // missiles up
    sj_object_get_value_as_float(data_entry, "column_3", &x);
    sj_object_get_value_as_float(data_entry, "row_1_b", &y);
    progress = (float) UI_data->missiles_check;
    goal = (float) UI_data->missiles_max;

    gf2d_draw_rect_filled(gfc_rect(x, y, width * (progress / goal), height), GFC_COLOR_LIGHTBLUE);

        // single shot dmg up
    sj_object_get_value_as_float(data_entry, "column_1", &x);
    sj_object_get_value_as_float(data_entry, "row_2", &y);
    progress = (float) UI_data->single_shot_check;
    goal = (float) UI_data->single_shot_max;

    gf2d_draw_rect_filled(gfc_rect(x, y, width * (progress / goal), height), GFC_COLOR_LIGHTBLUE);

        // charge shot dmg up
    sj_object_get_value_as_float(data_entry, "column_2", &x);
    sj_object_get_value_as_float(data_entry, "row_2", &y);
    progress = (float) UI_data->charge_shot_check;
    goal = (float) UI_data->charge_shot_max;

    gf2d_draw_rect_filled(gfc_rect(x, y, width * (progress / goal), height), GFC_COLOR_LIGHTBLUE);

        // nuke cost down
    sj_object_get_value_as_float(data_entry, "column_3", &x);
    sj_object_get_value_as_float(data_entry, "row_2", &y);
    progress = (float) UI_data->nuke_check;
    goal = (float) UI_data->nuke_max;

    gf2d_draw_rect_filled(gfc_rect(x, y, width * (progress / goal), height), GFC_COLOR_LIGHTBLUE);

    // perks draws (TODO)
}

void wave_start() {
    SJson* data_entry;
    LevelData* level;
    float x, y;
    char buffer[9];

    level = get_level_data();

    if (!level) return;

    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(65, 65, 65, 0.4f));
    gf2d_sprite_draw_image(UI_data->wave_start, gfc_vector2d(0, 0));

    sprintf(buffer, "WAVE #%d", level->wave_count);
    gf2d_font_draw_line_tag(buffer, FT_H1, GFC_COLOR_WHITE, UI_data->curr_wave_loc);

    switch (level->obj_type) {
        case KILL_ENEMY:
            data_entry = sj_object_get_value(UI_data->wave_start_data, "kill_enemy_text");
            sj_object_get_value_as_float(data_entry, "x_offset", &x);
            sj_object_get_value_as_float(data_entry, "y_offset", &y);

            break;
    }

    gf2d_font_draw_line_tag(level->level_obj, FT_H2, GFC_COLOR_WHITE, gfc_vector2d(x,y));
}

void wave_completed() { 
    LevelData* level;

    level = get_level_data();

    if (!level) return;

    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(65, 65, 65, 0.4f));
    gf2d_sprite_draw_image(UI_data->wave_completed, gfc_vector2d(0, 0));

    // TODO: implement images and text for next stages
}

void player_death_screen() {
    SJson* data_entry;
    LevelData* level;
    char buffer[35];
    float x, y;

    level = get_level_data();

    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(255, 0, 0, 0.4f));
    pause_menu(UI_data->game_over, UI_data->game_over_data);

    // stats display
    data_entry = sj_object_get_value(UI_data->game_over_data, "stats");
    sj_object_get_value_as_float(data_entry, "text_x", &x);

    sj_object_get_value_as_float(data_entry, "enemy_y", &y);
    sprintf(buffer, "Enemies Killed: %d", level->enemy_killed_total);
    gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, gfc_vector2d(x, y));

    sj_object_get_value_as_float(data_entry, "waves_y", &y);
    sprintf(buffer, "Waves Cleared: %d", level->wave_count);
    gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, gfc_vector2d(x, y));

    sj_object_get_value_as_float(data_entry, "scrap_y", &y);
    sprintf(buffer, "Scrap Collected: %d", level->total_scrap);
    gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, gfc_vector2d(x, y));

    sj_object_get_value_as_float(data_entry, "time_y", &y);
    sprintf(buffer, "Time Played: %f", level->total_game_time);
    gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, gfc_vector2d(x, y));
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
    if (!UI_data) return NULL;

    return UI_data;
}