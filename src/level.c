#include "simple_logger.h"
#include "gf2d_mouse.h"
#include "gfc_input.h"
#include "player.h"
#include "enemy.h"
#include "ui.h"
#include "level.h"

static LevelData* level;

void level_init() {
    level = gfc_allocate_array(sizeof(LevelData), 1);
    
    
    level->last_powerup = 0;
    level->enemy_count = 0;
    level->enemy_killed = 0;
    level->emper_flag = 0;
    level->fencer_flag = 0;
    
    level->wave_count = 1;

    //level->asteroid = gf3d_model_load("models/trench/asteroid.model");
    //level->asteroid_list = gfc_list_new_size(ASTEROID_MAX);


    

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

/*
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
*/

void level_reset() {
    level->last_powerup = 0;
    level->enemy_count = 0;
    level->enemy_killed = 0;
    level->emper_flag = 0;
    level->fencer_flag = 0;

    level->wave_count = 1;
}

/*
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
*/

void level_free() {
    //gf3d_model_free(level->asteroid);
    //gfc_list_delete(level->asteroid_list);

    free(level);
}

/*
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
*/

void level_update() {
    //level_visuals();
}

LevelData* get_level_data() {
    if (!level) return NULL;

    return level;
}