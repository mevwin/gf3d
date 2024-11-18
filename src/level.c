#include "simple_logger.h"
#include "gf2d_mouse.h"
#include "gfc_config_def.h"
#include "player.h"
#include "enemy.h"
#include "ui.h"
#include "level.h"

static LevelData* level;

void level_init() {
    level = gfc_allocate_array(sizeof(LevelData), 1);
    
    level->assets_made = 0;
    level->game_start = 0;
    level->enemy_start = 0;
    level->_done = 0;
    
    level->last_powerup = 0;
    level->enemy_count = 0;
    level->enemy_killed = 0;
    level->emper_flag = 0;
    level->fencer_flag = 0;
    
    level->wave_count = 1;
    level->in_shop = 0;
    level->paused = 0;
    level->wave_end = 0;

    level->asteroid = gf3d_model_load("models/trench/asteroid.model");
    level->asteroid_list = gfc_list_new_size(ASTEROID_MAX);

    level->continue_from_save = 0;  // initially zero, must check at start_menu_think

    level->player_init = sj_load("def/player_init.json");
    level->player_save = "def/player_save_base.json";

    atexit(level_free);
}

void game_data_init_from_save() {
    PlayerData* p_data;
    UIData* ui;
    SJson* save, *value;

    ui = get_UI_data();
    save = sj_load("def/player_save.json");

    value = sj_object_get_value(save, "level_data");
    sj_object_get_value_as_uint32(value, "wave_count", &level->wave_count);

    value = sj_object_get_value(save, "upgrades");
    sj_object_get_value_as_uint8(value, "shields_check", &ui->shields_check);
    sj_object_get_value_as_uint8(value, "more_scrap_check", &ui->more_scrap_check);
    sj_object_get_value_as_uint8(value, "missiles_check", &ui->missiles_check);
    sj_object_get_value_as_uint8(value, "single_shot_check", &ui->single_shot_check);
    sj_object_get_value_as_uint8(value, "charge_shot_check", &ui->charge_shot_check);
    sj_object_get_value_as_uint8(value, "nuke_check", &ui->nuke_check);
    sj_object_get_value_as_uint8(value, "shields_count", &ui->shields_count);
    sj_object_get_value_as_uint8(value, "more_scrap_count", &ui->more_scrap_count);
    sj_object_get_value_as_uint8(value, "missiles_count", &ui->missiles_count);
    sj_object_get_value_as_uint8(value, "single_shot_count", &ui->single_shot_count);
    sj_object_get_value_as_uint8(value, "harge_shot_count", &ui->missiles_count);

    sj_free(save);
}

void game_save() {
    PlayerData* p_data;
    UIData* ui;
    SJson* save, *value, *data_entry;
    char buffer[4];
    Uint32 wave_check;

    p_data = get_player_data();
    level;
    ui = get_UI_data();

    if (p_data->player_dead)
        return;

    save = sj_load(level->player_save);

    // level save
    value = sj_object_get_value(save, "level_data");

    data_entry = sj_object_get_value(value, "wave_count");
    sj_object_get_value_as_uint32(value, "wave_count", &wave_check);
    if (level->wave_count == wave_check)    // don't save if currently on starting wave from save
        return;

    data_entry->v.string = sj_value_to_json_string(sj_new_uint32(level->wave_count));

    // player save
    value = sj_object_get_value(save, "player_data");
    data_entry = sj_object_get_value(value, "maxHealth");
    data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->maxHealth));

    data_entry = sj_object_get_value(value, "currHealth");
    data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->currHealth));

    data_entry = sj_object_get_value(value, "maxShield");
    data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->maxShield));

    data_entry = sj_object_get_value(value, "currShield");
    data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->currShield));

    data_entry = sj_object_get_value(value, "maxScrap");
    data_entry->v.string = sj_value_to_json_string(sj_new_int(p_data->maxScrap));

    data_entry = sj_object_get_value(value, "currScrap");
    data_entry->v.string = sj_value_to_json_string(sj_new_int(p_data->currScrap));

    data_entry = sj_object_get_value(value, "single_shot_bonus");
    data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->single_shot_bonus));

    data_entry = sj_object_get_value(value, "charge_shot_mult");
    data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->charge_shot_mult));

    data_entry = sj_object_get_value(value, "nuke_cost");
    data_entry->v.string = sj_value_to_json_string(sj_new_int(p_data->nuke_cost));

    // upgrades save
    value = sj_object_get_value(save, "upgrades");
    data_entry = sj_object_get_value(value, "shields_check");
    data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->shields_check));

    data_entry = sj_object_get_value(value, "more_scrap_check");
    data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->more_scrap_check));

    data_entry = sj_object_get_value(value, "missiles_check");
    data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->missiles_check));

    data_entry = sj_object_get_value(value, "single_shot_check");
    data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->single_shot_check));

    data_entry = sj_object_get_value(value, "charge_shot_check");
    data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->charge_shot_check));

    data_entry = sj_object_get_value(value, "nuke_check");
    data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->nuke_check));

    data_entry = sj_object_get_value(value, "shields_count");
    data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->shields_count));

    data_entry = sj_object_get_value(value, "more_scrap_count");
    data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->more_scrap_count));

    data_entry = sj_object_get_value(value, "missiles_count");
    data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->missiles_count));
    
    data_entry = sj_object_get_value(value, "single_shot_count");
    data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->single_shot_count));

    data_entry = sj_object_get_value(value, "charge_shot_count");
    data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->charge_shot_count));

    sj_save(save, "def/player_save.json");
    sj_free(save);
}

void new_level_reset() {
    level->assets_made = 0;
    level->game_start = 0;
    level->enemy_start = 0;
    level->_done = 0;

    level->last_powerup = 0;
    level->enemy_count = 0;
    level->enemy_killed = 0;
    level->emper_flag = 0;
    level->fencer_flag = 0;

    level->wave_count = 1;
    level->in_shop = 0;
    level->paused = 0;
    level->wave_end = 0;

    level->continue_from_save = 0;
}

void asteroid_init() {
    int i, x_start, y_start, z_start;
    float neg;
    Entity* asteroid;

    // asteroid init
    for (i = 0; i < ASTEROID_MAX; i++) {
        asteroid = entity_new();
        asteroid->entity_type = ASTEROID;

        x_start = gfc_random_int(220);
        neg = gfc_crandom();
        if (neg < 0)
            x_start = -x_start;

        y_start = -600 + (gfc_random_int(20) * 20);
        z_start = -55 - gfc_random_int(20);

        asteroid->position = gfc_vector3d(x_start, y_start, z_start);
        asteroid->model = level->asteroid;
        gfc_list_append(level->asteroid_list, asteroid);
    }
    level->asteroids_made = 1;
}

void asteroid_free(){
    gfc_list_foreach(level->asteroid_list, (void (*) (void*)) entity_free);
    level->asteroids_made = 0;
}

void level_free() {
    gf3d_model_free(level->asteroid);
    gfc_list_delete(level->asteroid_list);
    sj_free(level->player_init);
    free(level);
}

void level_visuals() {
    int i;
    Entity* curr;
    
    if (level->in_shop || level->paused || level->wave_end) return;

    for (i = 0; i < ASTEROID_MAX; i++) {
        curr = (Entity*) gfc_list_nth(level->asteroid_list, i);
        
        if (i % 2 == 0) {
            curr->rotation.x += 0.01f;
            curr->position.y += (float) (2 + gfc_random_int(3));
        }
        else {
            curr->rotation.x -= 0.01f;
            curr->position.y += 5.0f;
        }
        curr->rotation.z += 0.01f;

        if (curr->position.y >= 30)
            curr->position.y = -700;
    }
}

void level_update(void* p, void* player_data) {
    Entity* player = (Entity*) p;
    PlayerData* p_data = (PlayerData*) player_data;

    if (!player || !p_data) return;

    entity_think_all();
    entity_update_all();
    entity_draw_all();
    level_visuals();

    if (gfc_input_command_pressed("shop")) {
        if (!level->in_shop) {
            if (level->paused)
                level->paused = 0;

            level->in_shop = 1;
        }
        else
            level->in_shop = 0;
    }

    if (gfc_input_command_pressed("escape")) {
        if (!level->paused) {
            if (level->in_shop)
                level->in_shop = 0;

            level->paused = 1;
        }
        else
            level->paused = 0;
    }

    if (level->in_shop) { // shop menu
        shop_hud_draw(p_data);
        shop_think(p_data);
        gf2d_mouse_draw();
    }
    else if (level->paused) { // pause menu
        pause_menu();
        pause_menu_think(level);
        gf2d_mouse_draw();
    }
    else if (p_data->player_dead) { // death screen
        player_death_screen();
        gf2d_mouse_draw();
        entity_reset();

        // player respawn
        if (gf2d_mouse_button_released(2)) {
            shop_reset(); // reset upgrade checks if player has died
            player_respawn(player);
            level->enemy_count = 0;
            level->enemy_killed = 0;
            level->enemy_start = 0;
            level->fencer_flag = 0;
            level->wave_count = 0;
        }
    }
    else { // game in play
        enemy_hud_all();
        player_hud(player->data);

        if (!level->enemy_start)
            wave_start(level);

        // game condition
        if (level->enemy_count < ENEMY_MIN_LIMIT && level->enemy_killed < ENEMY_GOAL && level->enemy_start)
            enemy_spawn(&(player->position));

        if (level->enemy_killed >= ENEMY_GOAL) {
            level->enemy_killed = ENEMY_GOAL;
            enemy_reset();
            p_data->active_item = 1;
            wave_completed(p_data, level);
        }
    }
}

LevelData* get_level_data() {
    if (!level) return NULL;

    return level;
}