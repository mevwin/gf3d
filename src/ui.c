#include "simple_logger.h"
#include "gfc_audio.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf2d_mouse.h"
#include "gfc_config.h"
#include "ui.h"
#include "player.h"
#include "enemy.h"
#include "world.h"
#include "level.h"
#include "item.h"
#include "perk.h"
#include "notifications.h"

static UIData* UI_data;

void display_new_perk(Perk* perk, Uint8 slot);
void display_player_perks(void* data);
void buy_perk(GFC_List* perk_list, Perk* perk);
void sell_perk(PlayerData* p_data, Uint8 slot);

void UI_init() {
    SJson* position_data, *dimen_data;
    GFC_Vector4D rect;

    UI_data = gfc_allocate_array(sizeof(UIData), 1);
    if (!UI_data) return;

    /*shop UI*/
    UI_data->shop = gf2d_sprite_load_image("images/UI/shop/shop.png");
    UI_data->shop_data = sj_load("menus/shop.menu");
        
        // next wave button
    position_data = sj_object_get_value(UI_data->shop_data, "next_wave_rect");
    sj_value_as_vector4d(position_data, &rect);
    UI_data->next_wave_block = gfc_rect_from_vector4(rect);

        // upgrades section
    position_data = sj_object_get_value(UI_data->shop_data, "upgrades");
    sj_value_as_vector4d(sj_object_get_value(position_data, "shields"), &rect);
    UI_data->shields_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "scrap"), &rect);
    UI_data->scrap_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "missile"), &rect);
    UI_data->missiles_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "single_shot"), &rect);
    UI_data->single_shot_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "charge_shot"), &rect);
    UI_data->charge_shot_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "nuke"), &rect);
    UI_data->nuke_block = gfc_rect_from_vector4(rect);

            // upgrade caps
    dimen_data = sj_object_get_value(position_data, "upgrade_caps");
    sj_object_get_value_as_uint8(dimen_data, "shields", &UI_data->shields_max);
    sj_object_get_value_as_uint8(dimen_data, "scrap", &UI_data->more_scrap_max);
    sj_object_get_value_as_uint8(dimen_data, "missile", &UI_data->missiles_max);
    sj_object_get_value_as_uint8(dimen_data, "single_shot", &UI_data->single_shot_max);
    sj_object_get_value_as_uint8(dimen_data, "charge_shot", &UI_data->charge_shot_max);
    sj_object_get_value_as_uint8(dimen_data, "nuke", &UI_data->nuke_max);
    sj_object_get_value_as_uint8(position_data, "upgrade_cost_default", &UI_data->upgrade_cost);
    sj_object_get_value_as_uint8(position_data, "perk_cost_default", &UI_data->perk_cost);
    sj_object_get_value_as_uint8(position_data, "perk_sell_reduction", &UI_data->perk_sell_reduction);

    UI_data->shields_check = 0;
    UI_data->more_scrap_check = 0;
    UI_data->missiles_check = 0;
    UI_data->single_shot_check = 0;
    UI_data->charge_shot_check = 0;
    UI_data->nuke_check = 0;

    position_data = sj_object_get_value(UI_data->shop_data, "scrap_bar_rect");
    sj_value_as_vector4d(position_data, &rect);
    UI_data->scrap_bar = gfc_rect_from_vector4(rect);

            // current perks
    position_data = sj_object_get_value(UI_data->shop_data, "perks");
    sj_value_as_vector4d(sj_object_get_value(position_data, "curr_perk1"), &rect);
    UI_data->curr_perk1 = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "curr_perk2"), &rect);
    UI_data->curr_perk2 = gfc_rect_from_vector4(rect);

            // new perks
    sj_value_as_vector4d(sj_object_get_value(position_data, "new_perk1"), &rect);
    UI_data->new_perk1 = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "new_perk2"), &rect);
    UI_data->new_perk2 = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "new_perk3"), &rect);
    UI_data->new_perk3 = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "desc_box"), &rect);
    UI_data->s_perk_desc = gfc_rect_from_vector4(rect);

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
    UI_data->game_mode_select = gf2d_sprite_load_image("images/UI/start_menu/game_mode_sel.png");
    UI_data->start_menu_data = sj_load("menus/start_menu.menu");

    position_data = sj_object_get_value(UI_data->start_menu_data, "option_pos");
    sj_value_as_vector4d(sj_object_get_value(position_data, "new_game"), &rect);
    UI_data->new_start_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "continue"), &rect);
    UI_data->continue_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "previous"), &rect);
    UI_data->previous_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "s_quit"), &rect);
    UI_data->s_quit_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "editor"), &rect);
    UI_data->editor_block = gfc_rect_from_vector4(rect);

    position_data = sj_object_get_value(UI_data->start_menu_data, "game_mode_sel");
    sj_value_as_vector4d(sj_object_get_value(position_data, "regular"), &rect);
    UI_data->regular_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "endless"), &rect);
    UI_data->endless_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "exit"), &rect);
    UI_data->s_exit_block = gfc_rect_from_vector4(rect);

    /*pause menu*/
    UI_data->pause_menu = gf2d_sprite_load_image("images/UI/pause_menu/pause_menu.png");
    UI_data->pause_menu_data = sj_load("menus/pause_menu.menu");
    UI_data->item_progress_bar = gf2d_sprite_load_image("images/UI/pause_menu/item_progress_bar.png");
    UI_data->p_progress_bar = gf2d_sprite_load_image("images/UI/pause_menu/game_progress_bar.png");

        // menu blocks
    position_data = sj_object_get_value(UI_data->pause_menu_data, "option_pos");
    sj_value_as_vector4d(sj_object_get_value(position_data, "resume"), &rect);
    UI_data->resume_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "quit"), &rect);
    UI_data->quit_block = gfc_rect_from_vector4(rect);

        // perks and item progress are calculated in pause_menu()

    position_data = sj_object_get_value(UI_data->pause_menu_data, "perks_blocks");
    sj_value_as_vector4d(sj_object_get_value(position_data, "perk1"), &rect);
    UI_data->p_perk1 = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "perk2"), &rect);
    UI_data->p_perk2 = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "desc_box"), &rect);
    UI_data->p_perk_desc = gfc_rect_from_vector4(rect);

    /*wave start*/
    UI_data->wave_start = gf2d_sprite_load_image("images/UI/wave_start/wave_start.png");
    UI_data->wave_start_data = sj_load("menus/wave_start.menu");

    position_data = sj_object_get_value(UI_data->wave_start_data, "wave_text_offset");
    sj_value_as_vector2d(position_data, &UI_data->curr_wave_loc);


    /*wave completed*/
    UI_data->wave_completed = gf2d_sprite_load_image("images/UI/wave_completed/wave_completed.png");
    UI_data->wave_completed_data = sj_load("menus/wave_completed.menu");

    position_data = sj_object_get_value(UI_data->wave_completed_data, "stage_blocks");
    sj_value_as_vector4d(sj_object_get_value(position_data, "stage1_block"), &rect);
    UI_data->stage_block1 = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "stage2_block"), &rect);
    UI_data->stage_block2 = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "nextwave_block"), &rect);
    UI_data->nextwave_block = gfc_rect_from_vector4(rect);

    /*game over (very similar to pause menu)*/
    UI_data->game_over = gf2d_sprite_load_image("images/UI/game_over/game_over.png");
    UI_data->game_over_data = sj_load("menus/game_over.menu");

        // menu blocks
    position_data = sj_object_get_value(UI_data->game_over_data, "option_pos");
    sj_value_as_vector4d(sj_object_get_value(position_data, "respawn"), &rect);
    UI_data->respawn_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(position_data, "g_quit"), &rect);
    UI_data->g_quit_block = gfc_rect_from_vector4(rect);

    UI_data->nuke_alpha = 0.0f;
    UI_data->emper_alpha = 0.0f;

    /*previous runs*/
    UI_data->prev_menu_data = sj_load("menus/previous_run.menu");
    UI_data->preview_menu = gf2d_sprite_load_image("images/UI/previous_runs/runs_preview.png");
    UI_data->previous_run = gf2d_sprite_load_image("images/UI/previous_runs/previous_run.png");

    sj_value_as_vector4d(sj_object_get_value(UI_data->prev_menu_data, "return_rect"), &rect);
    UI_data->return_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(UI_data->prev_menu_data, "previews_menu_exit"), &rect);
    UI_data->exit_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(UI_data->prev_menu_data, "previews_next_page"), &rect);
    UI_data->next_block = gfc_rect_from_vector4(rect);

    sj_value_as_vector4d(sj_object_get_value(UI_data->prev_menu_data, "previews_prev_page"), &rect);
    UI_data->prev_block = gfc_rect_from_vector4(rect);

    UI_data->preview_page_offset = 0;

    sj_value_as_vector2d(sj_object_get_value(UI_data->prev_menu_data, "run_num_offset"), &UI_data->run_num_offset);

    /*game complete*/
    UI_data->game_complete = gf2d_sprite_load_image("images/UI/game_complete/game_complete.png");

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
    sj_free(UI_data->prev_menu_data);
    gf2d_sprite_free(UI_data->player_hud);
    gf2d_sprite_free(UI_data->start_menu);
    gf2d_sprite_free(UI_data->game_mode_select);
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
    gf2d_sprite_free(UI_data->p_progress_bar);
    gf2d_sprite_free(UI_data->item_progress_bar);
    gf2d_sprite_free(UI_data->preview_menu);
    gf2d_sprite_free(UI_data->previous_run);
    gf2d_sprite_free(UI_data->game_complete);

    free(UI_data);
}

void shop_hud_draw() {
    SJson* data_entry;
    float height, progress, max, current;
    LevelData* level;
    PlayerData* p_data;
    WorldData* world;
    GFC_Rect dummy, dummy2;
    Perk* perk;
    GFC_List* perk_list;
    int i, new_slot;

    level = get_level_data();
    p_data = get_player_data();
    world = get_world_data();

    perk_list = get_perk_list();

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

            if (progress != 0) {
                max = (float)p_data->maxScrap;
                current = (float)(progress / max);
                dummy2.x += (dummy.w - (dummy2.w * current));
                dummy2.w *= current;
                gf2d_draw_rect_filled(dummy2, GFC_COLOR_LIGHTRED);
            }
        }
    }

        // show perk cost on scrap bar
    if (p_data->currScrap >= UI_data->perk_cost) {
        if (gf2d_mouse_in_rect(UI_data->new_perk1)) {
            gfc_rect_copy(dummy2, UI_data->scrap_bar);

            perk = gfc_list_nth(perk_list, 0);

            if (!perk->bought) {
                progress = (float)UI_data->perk_cost;
                max = (float)p_data->maxScrap;
                current = (float)(progress / max);
                dummy2.x += (dummy.w - (dummy2.w * current));
                dummy2.w *= current;
                gf2d_draw_rect_filled(dummy2, GFC_COLOR_LIGHTRED);
            }
        }
        else if (gf2d_mouse_in_rect(UI_data->new_perk2)) {
            gfc_rect_copy(dummy2, UI_data->scrap_bar);

            perk = gfc_list_nth(perk_list, 1);

            if (!perk->bought) {
                progress = (float)UI_data->perk_cost;
                max = (float)p_data->maxScrap;
                current = (float)(progress / max);
                dummy2.x += (dummy.w - (dummy2.w * current));
                dummy2.w *= current;
                gf2d_draw_rect_filled(dummy2, GFC_COLOR_LIGHTRED);
            }
        }
        else if (gf2d_mouse_in_rect(UI_data->new_perk3)) {
            gfc_rect_copy(dummy2, UI_data->scrap_bar);

            perk = gfc_list_nth(perk_list, 2);

            if (!perk->bought) {
                progress = (float)UI_data->perk_cost;
                max = (float)p_data->maxScrap;
                current = (float)(progress / max);
                dummy2.x += (dummy.w - (dummy2.w * current));
                dummy2.w *= current;
                gf2d_draw_rect_filled(dummy2, GFC_COLOR_LIGHTRED);
            }
        }
    }

        // show scrap return when selling a perk on the scrap bar
        // TODO: fix bar clipping outside if new scrap value exceeds maxScarp
    if (gf2d_mouse_in_rect(UI_data->curr_perk1)) {
        perk = p_data->perk1;

        if (perk->type != NO_PERK) {
            gfc_rect_copy(dummy2, UI_data->scrap_bar);

            if (p_data->currScrap + UI_data->perk_cost - UI_data->perk_sell_reduction <= p_data->maxScrap)
                progress = (float) (UI_data->perk_cost - UI_data->perk_sell_reduction);
            else // if new scrap exceeds maxScrap
                progress = (float) (p_data->maxScrap - p_data->currScrap);

            max = (float) p_data->maxScrap;
            current = (float)(progress / max);
            dummy2.x += dummy.w;
            dummy2.w *= current;
            gf2d_draw_rect_filled(dummy2, GFC_COLOR_DARKBLUE);
        }
    }
    else if (gf2d_mouse_in_rect(UI_data->curr_perk2)) {
        perk = p_data->perk2;

        if (perk->type != NO_PERK) {
            gfc_rect_copy(dummy2, UI_data->scrap_bar);

            if (p_data->currScrap + UI_data->perk_cost - UI_data->perk_sell_reduction <= p_data->maxScrap)
                progress = (float)(UI_data->perk_cost - UI_data->perk_sell_reduction);
            else // if new scrap exceeds maxScrap
                progress = (float)(p_data->maxScrap - p_data->currScrap);

            progress = (float)(UI_data->perk_cost - UI_data->perk_sell_reduction);
            max = (float) p_data->maxScrap;
            current = (float)(progress / max);
            dummy2.x += dummy.w;
            dummy2.w *= current;
            gf2d_draw_rect_filled(dummy2, GFC_COLOR_DARKBLUE);
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

    // perks section
        // display new perks for player to buy
    for (i = 0, new_slot = 1; i < perk_list->count; i++) {
        perk = gfc_list_nth(perk_list, i);

        if (!perk) continue;

        if (perk->bought) { //player just bought perk or perk is null do not display
            new_slot++;
            continue;
        }
        else { // new perk that can be bought 
            display_new_perk(perk, new_slot);
            new_slot++;
        }
    }

        // display player perks
    display_player_perks(p_data);

    // notifications

    if (world->notif_flag)
        notif_window(world->notif_type);

}

void display_new_perk(Perk* perk, Uint8 slot) {
    GFC_Rect desc_block;

    switch (slot) {
        case 1:
            gf2d_draw_rect_filled(UI_data->new_perk1, perk->color);
            gf2d_font_draw_text_wrap_tag(perk->name, FT_Large, GFC_COLOR_WHITE, UI_data->new_perk1);
            
            gfc_rect_copy(desc_block, UI_data->new_perk1);
            desc_block.x += 4;
            desc_block.y += 50;
            desc_block.w -= 50;

            gf2d_font_draw_text_wrap_tag(perk->desc, FT_H5, GFC_COLOR_WHITE, desc_block);

            break;
        case 2:
            gf2d_draw_rect_filled(UI_data->new_perk2, perk->color);
            gf2d_font_draw_text_wrap_tag(perk->name, FT_Large, GFC_COLOR_WHITE, UI_data->new_perk2);
            
            gfc_rect_copy(desc_block, UI_data->new_perk2);
            desc_block.x += 4;
            desc_block.y += 50;
            desc_block.w -= 50;

            gf2d_font_draw_text_wrap_tag(perk->desc, FT_H5, GFC_COLOR_WHITE, desc_block);

            break;
        case 3:
            gf2d_draw_rect_filled(UI_data->new_perk3, perk->color);
            gf2d_font_draw_text_wrap_tag(perk->name, FT_Large, GFC_COLOR_WHITE, UI_data->new_perk3);

            gfc_rect_copy(desc_block, UI_data->new_perk3);
            desc_block.x += 4;
            desc_block.y += 50;
            desc_block.w -= 50;

            gf2d_font_draw_text_wrap_tag(perk->desc, FT_H5, GFC_COLOR_WHITE, desc_block);

            break;
        default:
            slog("slot not found: %i", slot);
            return;
    }
}

void display_player_perks(void* data){
    Perk* perk;
    WorldData* world;
    GFC_Rect desc_block;
    PlayerData* p_data;
    
    world = get_world_data();

    // if null, dislaying previous run
    if (!data) {
        perk = (Perk*) world->perk1;
    }
    else {
        p_data = (PlayerData*) data;
        perk = p_data->perk1;
    }

    if (perk->type != NO_PERK) {
        if (world->current_state == SHOP) {
            gf2d_draw_rect_filled(UI_data->curr_perk1, perk->color);

            if (gf2d_mouse_in_rect(UI_data->curr_perk1)) {
                gf2d_draw_rect_filled(UI_data->s_perk_desc, perk->color);
                gf2d_font_draw_text_wrap_tag(perk->name, FT_Large, GFC_COLOR_WHITE, UI_data->s_perk_desc);
                gfc_rect_copy(desc_block, UI_data->s_perk_desc);
                desc_block.x += 4;
                desc_block.y += 50;

                gf2d_font_draw_text_wrap_tag(perk->desc, FT_H5, GFC_COLOR_WHITE, desc_block);
            }
        }
        else if (world->current_state == PAUSE_MENU || world->current_state == GAME_OVER || 
                 world->current_state == PREVIOUS_RUN || world->current_state == GAME_COMPLETED) 
        {
            gf2d_draw_rect_filled(UI_data->p_perk1, perk->color);

            if (gf2d_mouse_in_rect(UI_data->p_perk1)) {
                gf2d_draw_rect_filled(UI_data->p_perk_desc, perk->color);
                gf2d_font_draw_text_wrap_tag(perk->name, FT_Large, GFC_COLOR_WHITE, UI_data->p_perk_desc);

                gfc_rect_copy(desc_block, UI_data->p_perk_desc);
                desc_block.x += 4;
                desc_block.y += 50;

                gf2d_font_draw_text_wrap_tag(perk->desc, FT_H5, GFC_COLOR_WHITE, desc_block);
            }
        }
    }

    if (!data) {
        perk = (Perk*) world->perk2;
    }
    else {
        p_data = (PlayerData*) data;
        perk = p_data->perk2;
    }

    if (perk->type != NO_PERK) {
        if (world->current_state == SHOP) {
            gf2d_draw_rect_filled(UI_data->curr_perk2, perk->color);

            if (gf2d_mouse_in_rect(UI_data->curr_perk2)) {
                gf2d_draw_rect_filled(UI_data->s_perk_desc, perk->color);
                gf2d_font_draw_text_wrap_tag(perk->name, FT_Large, GFC_COLOR_WHITE, UI_data->s_perk_desc);
                gfc_rect_copy(desc_block, UI_data->s_perk_desc);
                desc_block.x += 4;
                desc_block.y += 50;

                gf2d_font_draw_text_wrap_tag(perk->desc, FT_H5, GFC_COLOR_WHITE, desc_block);
            }
        }
        else if (world->current_state == PAUSE_MENU || world->current_state == GAME_OVER ||
            world->current_state == PREVIOUS_RUN || world->current_state == GAME_COMPLETED)
        {
            gf2d_draw_rect_filled(UI_data->p_perk2, perk->color);

            if (gf2d_mouse_in_rect(UI_data->p_perk2)) {
                gf2d_draw_rect_filled(UI_data->p_perk_desc, perk->color);
                gf2d_font_draw_text_wrap_tag(perk->name, FT_Large, GFC_COLOR_WHITE, UI_data->p_perk_desc);

                gfc_rect_copy(desc_block, UI_data->p_perk_desc);
                desc_block.x += 4;
                desc_block.y += 50;

                gf2d_font_draw_text_wrap_tag(perk->desc, FT_H5, GFC_COLOR_WHITE, desc_block);
            }
        }
    }
}

void buy_perk(GFC_List* perk_list, Perk* perk) {
    PlayerData* p_data;
    WorldData* world;
    Perk* player_perk;

    p_data = get_player_data();
    world = get_world_data();

    if (!p_data || !perk_list || !perk) {
        slog("failed to buy perk");
    }

    // remove from perk list and give it to player data struct
    //gfc_list_delete_data(perk_list, perk);

    // check if player has empty slots
    if (p_data->perk1->type == NO_PERK) {

        // check if player has duplicate perk
        if (p_data->perk2->type != perk->type) {
            p_data->perk1 = copy_perk_data(perk);
            p_data->currScrap -= UI_data->perk_cost;
        }
        else {
            world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
            world->notif_flag = 1;
            world->notif_type = DUPE_PERKS;
        }
        return;
    }

    if (p_data->perk2->type == NO_PERK) {

        // check if player has duplicate perk
        if (p_data->perk1->type != perk->type) {
            p_data->perk2 = copy_perk_data(perk);
            p_data->currScrap -= UI_data->perk_cost;
        }
        else {
            world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
            world->notif_flag = 1;
            world->notif_type = DUPE_PERKS;
        }
        return;
    }

    // at this point, player has no open slots and must sell perks
}

void sell_perk(PlayerData* p_data, Uint8 slot) {
    Perk* perk;
    
    if (!p_data) return;

    switch (slot) {
        case 1:
            perk = p_data->perk1;    
            break;

        case 2:
            perk = p_data->perk2;
            break;

        default:
            slog("invalid slot");
            return;
    }

    if (perk->type == NO_PERK)
        return;

    perk->type = NO_PERK;
    perk->type = 0;
    perk->num_effect = 0;
    sprintf(perk->name, "0");
    sprintf(perk->desc, "0");
    perk->color = gfc_color(0, 0, 0, 0);

    p_data->currScrap += UI_data->perk_cost - UI_data->perk_sell_reduction;
    
    if (p_data->currScrap > p_data->maxScrap)
        p_data->currScrap = p_data->maxScrap;

}

void shop_think() {
    LevelData* level;
    PlayerData* data;
    WorldData* world;
    Perk* perk_list;

    world = get_world_data();
    data = get_player_data();
    if (!data) return;

    level = get_level_data();
    if (!level->wave_end) return; // only allow buying during the end of the wave

    perk_list = get_perk_list();
    

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
            else {
                world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
                world->notif_flag = 1;
                world->notif_type = NO_SCRAP;
            }
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
            else {
                world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
                world->notif_flag = 1;
                world->notif_type = NO_SCRAP;
            }
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
            else {
                world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
                world->notif_flag = 1;
                world->notif_type = NO_SCRAP;
            }
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
            else {
                world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
                world->notif_flag = 1;
                world->notif_type = NO_SCRAP;
            }
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
            else {
                world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
                world->notif_flag = 1;
                world->notif_type = NO_SCRAP;
            }
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
            else {
                world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
                world->notif_flag = 1;
                world->notif_type = NO_SCRAP;
            }
        }
        else if (gf2d_mouse_in_rect(UI_data->new_perk1)) {
            if (data->currScrap >= UI_data->perk_cost) {
                buy_perk(perk_list, gfc_list_nth(perk_list, 0));
            }
            else {
                world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
                world->notif_flag = 1;
                world->notif_type = NO_SCRAP;
            }
        }
        else if (gf2d_mouse_in_rect(UI_data->new_perk2)) {
            if (data->currScrap >= UI_data->perk_cost) {
                buy_perk(perk_list, gfc_list_nth(perk_list, 1));
            }
            else {
                world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
                world->notif_flag = 1;
                world->notif_type = NO_SCRAP;
            }
        }
        else if (gf2d_mouse_in_rect(UI_data->new_perk3)) {
            if (data->currScrap >= UI_data->perk_cost) {
                buy_perk(perk_list, gfc_list_nth(perk_list, 2));
            }
            else {
                world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
                world->notif_flag = 1;
                world->notif_type = NO_SCRAP;
            }
        }
        else if (gf2d_mouse_in_rect(UI_data->curr_perk1)) {
            sell_perk(data, 1);
        }
        else if (gf2d_mouse_in_rect(UI_data->curr_perk2)) {
            sell_perk(data, 2);
        }

        gfc_sound_play(get_sound_data()->confirm, 0, 0.5, -1, -1);
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
    GFC_Vector2D bar_position, scale, offset;
    SJson* position_data;
    float scrap, maxscrap, nuke_cost, bar_length, currTime, currSeconds;
    float currHealth, currShield, currScrap, currVortex, currNuke;
    char buffer[100], time[5];
    int progress, goal, i, k;
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
    position_data = sj_object_get_value(UI_data->player_hud_data, "player_bars");

    // health bar draws
        // current health
    sj_value_as_vector2d(sj_object_get_value(position_data, "health_offset"), &bar_position);
    scale = gfc_vector2d(currHealth, 1.0f);
    gf2d_sprite_draw(UI_data->player_health, bar_position, &scale, 
                    NULL, NULL, NULL, NULL, NULL, NULL);

        // current shield
    sj_object_get_value_as_float(position_data, "bar_width", &bar_length);
    if (currShield > 0) {
        bar_position.x += (bar_length * currHealth);
        scale = gfc_vector2d(currShield, 1.0f);
        gf2d_sprite_draw(UI_data->player_shield, bar_position, &scale, 
                    NULL, NULL, NULL, NULL, NULL, NULL);
    }

    // scrap bar draws
        // current scrap
    sj_value_as_vector2d(sj_object_get_value(position_data, "scrap_offset"), &bar_position);
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
    sj_value_as_vector2d(sj_object_get_value(position_data, "vortex_offset"), &bar_position);
    scale = gfc_vector2d(currVortex, 1.0f);
    gf2d_sprite_draw(UI_data->player_vortex, bar_position, &scale, 
                    NULL, NULL, NULL, NULL, NULL, NULL);
    
    // power up notifs
    if (data->active_item == HAPPY_TRIGGER) {
        get_world_data()->notif_flag = 1;
        notif_window(HAPPYTRIG_POWERUP);
    }
    else if (data->active_item == INVINCIBILITY) {
        get_world_data()->notif_flag = 1;
        notif_window(INVINCE_POWERUP);
    }

    // display level_obj
    switch (level->obj_type) {
        case KILL_ENEMY:
            progress = level->enemy_killed;
            goal = level->enemy_goal;
            sprintf(buffer, "%s (%i/%i)", level->level_obj, progress, goal);

            break;
        case SURVIVE:
            currTime = (level->survival_time - (level->goal_timestamp - CURRENT_TIME)) / 60.0f;
            progress = (int) currTime;  //minutes
            k = (int)level->survival_time / 60.0f;
            currSeconds = (currTime * (k * 60.0f) / (level->survival_time / 60.0f)) - (progress * 60); //seconds
           
            i = (int) currSeconds;
            if (i < 10)
                sprintf(time, "%i:0%i", progress, i);
            else
                sprintf(time, "%i:%i", progress, i);

            sprintf(buffer, "%s (%s)", level->level_obj, time);

            break;
    }

    if (level->obj_type == KILL_ENEMY || level->obj_type == SURVIVE) {
        sj_value_as_vector2d(sj_object_get_value(UI_data->player_hud_data, "level_obj_offset"), &offset);
        gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, offset);
    }

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

void start_menu(Uint8 state) {
    WorldData* world;

    world = get_world_data();

    if (state == START_MENU)
        gf2d_sprite_draw_image(UI_data->start_menu, gfc_vector2d(0, 0));
    else if (state == GAME_MODE_SEL)
        gf2d_sprite_draw_image(UI_data->game_mode_select, gfc_vector2d(0, 0));

    if (world->notif_flag) 
        notif_window(world->notif_type);
}

void pause_menu(Sprite* menu, SJson* data) {
    SJson* data_entry;
    float x, y, width;
    float progress, goal;
    LevelData* level;
    PlayerData* p_data;
    GFC_Vector2D scale, offset;
    WorldData* world;

    level = get_level_data();
    world = get_world_data();

    if (world->current_state == PREVIOUS_RUN)
        p_data = NULL;
    else
        p_data = get_player_data();

    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(65, 65, 65, 0.4f));
    gf2d_sprite_draw_image(menu, gfc_vector2d(0, 0));

    // progress bar draw
    data_entry = sj_object_get_value(data, "progress_bar");
    sj_value_as_vector2d(sj_object_get_value(data_entry, "bar_offset"), &offset);

        // check objective type (TODO)
    switch (level->obj_type){   
        case KILL_ENEMY:
            progress = (float) level->enemy_killed;
            goal = (float) level->enemy_goal;
            break;
        case SURVIVE:
            progress = world->pause_time;
            goal = level->goal_timestamp;
            break;
    }
    if (progress && goal) {
        scale = gfc_vector2d(progress / goal, 1.0f);
        gf2d_sprite_draw(UI_data->p_progress_bar, offset, &scale,
            NULL, NULL, NULL, NULL, NULL, NULL);
    }

        // lvl objective text
    sj_value_as_vector2d(sj_object_get_value(data_entry, "text_offset"), &offset);
    gf2d_font_draw_line_tag(level->level_obj, FT_Large, GFC_COLOR_WHITE, offset);

    // upgrade progress draws
    data_entry = sj_object_get_value(data, "item_progress_bar");

        // shields
    sj_value_as_vector2d(sj_object_get_value(data_entry, "shields_offset"), &offset);
    progress = (float) UI_data->shields_check;
    goal = (float) UI_data->shields_max;

    width = (progress / goal);
    scale = gfc_vector2d(width, 1.0f);

    gf2d_sprite_draw(UI_data->item_progress_bar, offset, &scale,
        NULL, NULL, NULL, NULL, NULL, NULL);

        // scrap up
    sj_value_as_vector2d(sj_object_get_value(data_entry, "scrapup_offset"), &offset);
    progress = (float)UI_data->more_scrap_check;
    goal = (float)UI_data->more_scrap_max;

    width = (progress / goal);
    scale = gfc_vector2d(width, 1.0f);

    gf2d_sprite_draw(UI_data->item_progress_bar, offset, &scale,
        NULL, NULL, NULL, NULL, NULL, NULL);

        // missiles up
    sj_value_as_vector2d(sj_object_get_value(data_entry, "missiles_offset"), &offset);
    progress = (float)UI_data->missiles_check;
    goal = (float)UI_data->missiles_max;

    width = (progress / goal);
    scale = gfc_vector2d(width, 1.0f);

    gf2d_sprite_draw(UI_data->item_progress_bar, offset, &scale,
        NULL, NULL, NULL, NULL, NULL, NULL);

        // single shot dmg up
    sj_value_as_vector2d(sj_object_get_value(data_entry, "singleshot_offset"), &offset);
    progress = (float)UI_data->single_shot_check;
    goal = (float)UI_data->single_shot_max;

    width = (progress / goal);
    scale = gfc_vector2d(width, 1.0f);

    gf2d_sprite_draw(UI_data->item_progress_bar, offset, &scale,
        NULL, NULL, NULL, NULL, NULL, NULL);

        // charge shot dmg up
    sj_value_as_vector2d(sj_object_get_value(data_entry, "chargeshot_offset"), &offset);
    progress = (float)UI_data->charge_shot_check;
    goal = (float)UI_data->charge_shot_max;

    width = (progress / goal);
    scale = gfc_vector2d(width, 1.0f);

    gf2d_sprite_draw(UI_data->item_progress_bar, offset, &scale,
        NULL, NULL, NULL, NULL, NULL, NULL);

        // nuke cost down
    sj_value_as_vector2d(sj_object_get_value(data_entry, "nukecost_offset"), &offset);
    progress = (float)UI_data->nuke_check;
    goal = (float)UI_data->nuke_max;

    width = (progress / goal);
    scale = gfc_vector2d(width, 1.0f);

    gf2d_sprite_draw(UI_data->item_progress_bar, offset, &scale,
        NULL, NULL, NULL, NULL, NULL, NULL);

    if (world->current_state == PREVIOUS_RUN)
        display_player_perks(NULL);
    else
        display_player_perks(p_data);
}

void wave_start() {
    SJson* data_entry;
    LevelData* level;
    GFC_Vector2D obj_loc;
    char buffer[9];

    level = get_level_data();

    if (!level) return;

    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(65, 65, 65, 0.4f));
    gf2d_sprite_draw_image(UI_data->wave_start, gfc_vector2d(0, 0));

    sprintf(buffer, "WAVE #%d", level->wave_count);
    gf2d_font_draw_line_tag(buffer, FT_H1, GFC_COLOR_WHITE, UI_data->curr_wave_loc);

    switch (level->obj_type) {
        case KILL_ENEMY:
            data_entry = sj_object_get_value(UI_data->wave_start_data, "kill_enemy_text_offset");
            break;
        
        case SURVIVE:
            data_entry = sj_object_get_value(UI_data->wave_start_data, "survive_text_offset");
            break;
        
        default:
            data_entry = NULL;
    }

    if (!data_entry)
        return;

    sj_value_as_vector2d(data_entry, &obj_loc);
    gf2d_font_draw_line_tag(level->level_obj, FT_H2, GFC_COLOR_WHITE, obj_loc);
}

// TODO: make two cases: stage select (ENDLESS) and show next level (REGULAR)
void wave_completed(Uint8 game_mode) {
    SJson* stage_desc;
    LevelData* level;
    LevelType level_type;
    ObjType obj_type;
    GFC_Vector2D offset;
    int i, j, k;
    char name[20];
    char obj[20];

    level = get_level_data();

    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(65, 65, 65, 0.4f));
    gf2d_sprite_draw_image(UI_data->wave_completed, gfc_vector2d(0, 0));

    // TODO: implement images and text for next stages
    if (game_mode == REGULAR) {
        stage_desc = sj_array_get_nth(sj_object_get_value(UI_data->wave_completed_data, "stage_desc"), 0);
        gf2d_draw_rect_filled(UI_data->nextwave_block, gfc_color(65, 65, 65, 0.4f));

        if (!level->curr_level) 
            get_current_level();
        
        sj_object_get_value_as_int(sj_object_get_value(level->curr_level, "level"), "level_type", &i);
        level_type = (LevelType) i;

        strcpy(name, sj_object_get_value_as_string(sj_array_get_nth(sj_object_get_value(level->level_base, "level_type"), i), "name"));
        
        sj_object_get_value_as_int(sj_object_get_value(level->curr_level, "level"), "obj_type", &i);
        obj_type = (ObjType) i;

        switch (obj_type) {
            case KILL_ENEMY:
                strcpy(obj, "KILL ENEMY");

                break;
            case SURVIVE:
                strcpy(obj, "SURVIVE");

                break;

            case BOSS:
                strcpy(obj, "MINI-BOSS");

                break;
        }
        sj_value_as_vector2d(sj_object_get_value(stage_desc, "level_text"), &offset);
        gf2d_font_draw_line_tag(name, FT_Large, GFC_COLOR_WHITE, offset);

        sj_value_as_vector2d(sj_object_get_value(stage_desc, "level_obj"), &offset);
        gf2d_font_draw_line_tag(obj, FT_Large, GFC_COLOR_WHITE, offset);

        /*
        * level preview neds:
        *   - level_type
        *   - level_obj
        *   - level_goal
        * 
        */

    }
    else if (game_mode == ENDLESS) {
        // two levels are created through json and displayed here
        for (k = 0; k < 2; k++) {
            stage_desc = sj_array_get_nth(sj_object_get_value(UI_data->wave_completed_data, "stage_desc"), k + 1);
            if (k == 0){
                gf2d_draw_rect_filled(UI_data->stage_block1, gfc_color(65, 65, 65, 0.4f));
                sj_object_get_value_as_int(level->endless_prev1, "level_type", &i);
                sj_object_get_value_as_int(level->endless_prev1, "obj_type", &j);
            }
            else { 
                gf2d_draw_rect_filled(UI_data->stage_block2, gfc_color(65, 65, 65, 0.4f));
                sj_object_get_value_as_int(level->endless_prev2, "level_type", &i);
                sj_object_get_value_as_int(level->endless_prev2, "obj_type", &j);
            }
            
            level_type = (LevelType)i;
            strcpy(name, sj_object_get_value_as_string(sj_array_get_nth(sj_object_get_value(level->level_base, "level_type"), i), "name"));
            obj_type = (ObjType)j;

            switch (obj_type) {
                case KILL_ENEMY:
                    sprintf(obj, "KILL ENEMY");

                    break;
                case SURVIVE:
                    strcpy(obj, "SURVIVE");

                    break;

                case BOSS:
                    strcpy(obj, "MINI-BOSS");

                break;
            }
            sj_value_as_vector2d(sj_object_get_value(stage_desc, "level_text"), &offset);
            gf2d_font_draw_line_tag(name, FT_Large, GFC_COLOR_WHITE, offset);

            sj_value_as_vector2d(sj_object_get_value(stage_desc, "level_obj"), &offset);
            gf2d_font_draw_line_tag(obj, FT_Large, GFC_COLOR_WHITE, offset);

        }
    }
}

void player_death_screen(Sprite* menu, SJson* menu_data) {
    SJson* data_entry;
    LevelData* level;
    GFC_Vector2D offset;
    char buffer[35];
    float x, y;

    level = get_level_data();

    gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), gfc_color(255, 0, 0, 0.4f));
    pause_menu(menu, menu_data);

    // stats display
    data_entry = sj_object_get_value(menu_data, "stats");

    sj_value_as_vector2d(sj_object_get_value(data_entry, "enemy_offset"), &offset);
    sprintf(buffer, "Enemies Killed: %d", level->enemy_killed_total);
    gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, offset);

    sj_value_as_vector2d(sj_object_get_value(data_entry, "waves_offset"), &offset);
    sprintf(buffer, "Waves Cleared: %d", level->wave_count - 1);
    gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, offset);

    sj_value_as_vector2d(sj_object_get_value(data_entry, "scrap_offset"), &offset);
    sprintf(buffer, "Scrap Collected: %d", level->total_scrap);
    gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, offset);

    sj_value_as_vector2d(sj_object_get_value(data_entry, "time_offset"), &offset);
    sprintf(buffer, "Time Played: %f", level->total_game_time);
    gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, offset);
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

    currHealth = data->currHealth / data->maxHealth;

    bar_position = gfc_3DPos_to_2DPos(position, data->x_bound, data->z_bound);
    scale = gfc_vector2d(currHealth, 1.0f);

    // bar offset (maybe testing the scaling for other res???)
    bar_position.x -= 50.0f;
    bar_position.y -= 50.0f;
    //bar_position.x *= 0.8f;
    //bar_position.y *= 0.8f;

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

void preview_runs() {
    WorldData* world;
    GFC_List* run_list;
    SJson* run, *data, *run_preview;
    GFC_Vector4D rect_vec;
    GFC_Vector2D offset;
    GFC_Rect preview_rect;
    GFC_Color color;
    char buffer[10];
    int i, j;

    gf2d_sprite_draw_image(UI_data->preview_menu, gfc_vector2d(0, 0));

    world = get_world_data();
    run_list = get_previous_runs();
    data = sj_object_get_value(UI_data->prev_menu_data, "previews");

    for (i = 0, j = UI_data->preview_page_offset; i < 3; i++, j++) {
        run = gfc_list_nth(run_list, j);
        if (run) {
            run_preview = sj_array_get_nth(data, i);

            sj_value_as_vector4d(sj_object_get_value(run_preview, "rect"), &rect_vec);
            preview_rect = gfc_rect_from_vector4(rect_vec);
            color = sj_object_get_color(UI_data->prev_menu_data, "previews_menu_color");

            sprintf(buffer, "RUN #%d", j+1);
            sj_value_as_vector2d(sj_object_get_value(run_preview, "text_offset"), &offset);

            gf2d_draw_rect_filled(preview_rect, color);
            gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, offset);

            if (gf2d_mouse_button_released(0) && gf2d_mouse_in_rect(preview_rect)) {
                gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);

                world->last_state = PREV_PREVIEW;

                UI_data->run_num = j + 1;

                level_begin(PREV_DISPLAY);
                game_data_init_from_save(RUNSAVE, run);
            }
        }
    }
}

void display_previous_run(Uint8 game_mode) {
    char buffer[10];

    player_death_screen(UI_data->previous_run, UI_data->prev_menu_data);

    sprintf(buffer, "#%d", UI_data->run_num);
    gf2d_font_draw_line_tag(buffer, FT_H1, GFC_COLOR_WHITE, UI_data->run_num_offset);

    if (game_mode == REGULAR)
        gf2d_font_draw_line_tag("REGULAR", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(40.0f, 40.0f));
    else if (game_mode == ENDLESS)
        gf2d_font_draw_line_tag("ENDLESS", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(40.0f, 40.0f));
}

void game_complete() {
    player_death_screen(UI_data->game_complete, UI_data->prev_menu_data);
}

UIData* get_UI_data() {
    if (!UI_data) return NULL;

    return UI_data;
}