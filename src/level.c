#include "simple_logger.h"
#include "gf2d_mouse.h"
#include "gfc_config.h"
#include "gfc_input.h"
#include "gfc_audio.h"
#include "world.h"
#include "player.h"
#include "enemy.h"
#include "ui.h"
#include "level.h"
#include "item.h"
#include "level_generator.h"

/*
typedef struct Level_S {
    Model*      terrain;
    GFC_List*   hazard_list;
    Uint8       hazard_count;
}Level;
*/

//static Level* level;
static LevelData* level_data;

void level_init() {
    level_data = gfc_allocate_array(sizeof(LevelData), 1);

    //level->asteroid = gf3d_model_load("models/trench/asteroid.model");
    //level->asteroid_list = gfc_list_new_size(ASTEROID_MAX);

    //TODO: create level from config
    
    //get_world_data()->level_assets_made = 1;
    atexit(level_free);
}

void level_begin(Uint8 game_mode) {
    level_data->last_powerup = 0;
    level_data->enemy_count = 0;
    level_data->enemy_killed = 0;
    level_data->enemy_killed_total = 0;
    level_data->fencer_flag = 0;
    level_data->wave_end_time = 0;
    level_data->total_game_time = 0;
    level_data->wave_end = 0;
    level_data->obj_complete = 0;
    level_data->wave_count = 1;
    level_data->total_scrap = 0;
    level_data->emper_flag = 0;
    level_data->flock_num = 0;
    level_data->level_base = sj_load("levels/level.def");

    switch (game_mode) {
        case REGULAR:
            level_data->level_def = sj_load("levels/regular_levels.def");
            level_load(REGULAR);

            break;

        case ENDLESS:
            level_gen_init(sj_load("levels/endless.def"));
            level_load(ENDLESS);

            break;
        case EDITOR:

            break;
    }
}

SJson* get_current_level() {
    SJson *data, *curr_level;
    char level_path[30];

    data = sj_array_get_nth(sj_object_get_value(level_data->level_def, "level_list"), level_data->wave_count - 1);
    strcpy(level_path, sj_object_get_value_as_string(data, "level"));
    curr_level = sj_load(level_path);

    if (!curr_level)
        return NULL;

    if (level_data->curr_level) 
        sj_free(level_data->curr_level);

    level_data->curr_level = curr_level;

    return curr_level;
}

// load a level based on wave_count-1
void level_load(Uint8 game_mode) {
    SJson* data, *curr_level, *def_entry;
    char buffer[30];
    int i, j;
    float k;

    if (game_mode == REGULAR) {
        curr_level = get_current_level();

        sj_object_get_value_as_int(sj_object_get_value(curr_level, "level"), "level_type", &i);
        level_data->level_type = (LevelType) i;

        def_entry = sj_object_get_value(level_data->level_base, "level_type");
        strcpy(level_data->name, sj_object_get_value_as_string(sj_array_get_nth(def_entry, i), "name"));

        sj_object_get_value_as_int(sj_object_get_value(curr_level, "level"), "obj_type", &j);
        level_data->obj_type = (ObjType) j;

        def_entry = sj_object_get_value(level_data->level_base, "level_obj");
        switch (level_data->obj_type) {
            case KILL_ENEMY:
                sj_object_get_value_as_int(sj_object_get_value(curr_level, "level"), "level_goal", &i);
                break;
            case SURVIVE:
                sj_object_get_value_as_float(sj_object_get_value(curr_level, "level"), "level_goal", &k);
                i = (int) k/60.0f;
                break;
        }

        if (i == 0) // boss stage does not need sprintf
            strcpy(level_data->level_obj, sj_object_get_value_as_string(sj_array_get_nth(def_entry, j), "obj_desc"));
        else {
            sprintf(buffer, sj_object_get_value_as_string(sj_array_get_nth(def_entry, j), "obj_desc"), i);
            strcpy(level_data->level_obj, buffer);
        }

        switch (level_data->obj_type) {
            case KILL_ENEMY:
                level_data->enemy_goal = i;
                break;
            case SURVIVE:
                level_data->survival_time = k;
                break;
        }
        data = sj_object_get_value(level_data->level_def, "level_list");
        level_data->wave_goal = data->v.array->count;
    }
    else if (game_mode == ENDLESS) {
        // create a level instead of loading one
        generate_level();
    }
    else if (game_mode == PREV_DISPLAY) {
        // all done in game_data_init_from_save
    }
}

void level_load_enemy_flock(SJson* curr_level, void* p_data) {
    SJson* flock, *enemy, *level_spawns;
    PlayerData* player;
    char buffer[9];
    int i;

    if (!curr_level)
        return;

    player = (PlayerData*) p_data;

    level_spawns = sj_object_get_value(sj_object_get_value(curr_level, "level"), "level_spawns");
    if (!level_spawns)
        return;

    sprintf(buffer, "flock%d", level_data->flock_num);
    flock = sj_object_get_value(level_spawns, buffer);
    if (!flock)
        return;

    for (i = 0; i < flock->v.array->count; i++) {
        enemy = sj_array_get_nth(flock, i);
        enemy_spawn(player->player_pos, 0, enemy, 0);
    }

    level_data->curr_level = NULL;
}

void new_wave_level_reset(){
    PlayerData* p_data;

    p_data = get_player_data();

    level_data->last_powerup = 0;
    level_data->enemy_count = 0;
    level_data->enemy_killed_total += level_data->enemy_killed;
    level_data->enemy_killed = 0;
    level_data->emper_flag = 0;
    level_data->fencer_flag = 0;
    level_data->flock_num = 0;

    level_data->wave_count++;
    level_data->wave_end = 1;

    // update player data to reset
    p_data->item_duration = 0;
    p_data->active_item = 0;

    if (p_data->nuke_flag)
        p_data->nuke_flag = 0;
}

void full_level_reset() {
    WorldData* world;

    world = get_world_data();

    level_data->last_powerup = 0;
    level_data->enemy_count = 0;
    level_data->enemy_killed = 0;
    level_data->enemy_killed_total = 0;
    level_data->emper_flag = 0;
    level_data->fencer_flag = 0;
    level_data->flock_num = 0;

    level_data->wave_end_time = 0;
    level_data->total_game_time = 0;

    level_data->total_scrap = 0;

    level_data->wave_count = 1;
    level_data->wave_end = 0;

    sj_free(level_data->level_base);

    if (world->game_mode == EDITOR || world->game_mode == PREV_DISPLAY)
        return;

    if (world->game_mode == ENDLESS) {
        level_gen_close();
        return;
    }

    if (level_data->level_def)
        sj_free(level_data->level_def);

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

    free(level_data);
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
    Entity* item, *entityList;
    ItemData* i_data;
    PlayerData* p_data;
    WorldData* world;
    int i;

    //level_visuals();

    world = get_world_data();
    entityList = get_entityList();
    p_data = get_player_data();

    if (p_data->player_dead) { // game is over
        level_data->total_game_time += CURRENT_TIME - level_data->game_start;
        game_save(RUNSAVE);

        if (world->game_mode == REGULAR) 
            world->current_state = GAME_OVER;
        else if (world->game_mode == ENDLESS) 
            world->current_state = GAME_COMPLETED;

        return;
    }

    // check if level objective has been accomplished
    switch (level_data->obj_type) {
        case KILL_ENEMY:
            if (level_data->enemy_killed >= level_data->enemy_goal)
                level_data->obj_complete = 1;

            break;
        case SURVIVE:
            if (CURRENT_TIME >= level_data->goal_timestamp)
                level_data->obj_complete = 1;

            break;
    }

    if (level_data->obj_complete) {
        entity_reset();
        new_wave_level_reset();

        if (world->game_mode == REGULAR) {
            if (level_data->wave_count - 1 == level_data->wave_goal || !get_current_level()) {
                level_data->total_game_time += CURRENT_TIME - level_data->game_start;

                game_save(RUNSAVE);
                world->current_state = GAME_COMPLETED;
                return;
            }
        }
        else if (world->game_mode == ENDLESS) {
            // create two levels
            level_data->endless_prev1 = generate_level_preview();
            level_data->endless_prev2 = generate_level_preview();
        }

        player_upgrade();
        shop_reset();

        // clear all items
        for (i = 0; i < MAX_ENTITY; i++) {
            item = &entityList[i];
            if (item->entity_type != ITEM) continue;

            i_data = item->data;

            if (i_data->type == SCRAP || i_data->type == HEALTH_PICKUP)
                item_activate(item, i_data->type);
            else
                entity_free(item);
        }

        level_data->wave_end_time = CURRENT_TIME;
        level_data->total_game_time += (level_data->wave_end_time - level_data->game_start);

        level_data->obj_complete = 0;
        world->current_state = WAVE_COMPLETED;
        update_player_perks();
        level_data->curr_level = NULL;
        return;
    }

    if (level_data->flock_num == -1)
        return;

    if (world->game_mode == REGULAR && world->current_state == IN_GAME) {
        if (level_data->enemy_count == 0) {
            // load next flock
            level_data->flock_num++;
            level_data->curr_level = get_current_level();
        }
        level_load_enemy_flock(level_data->curr_level, p_data);
    }
    else if (world->game_mode == ENDLESS && world->current_state == IN_GAME) {
        if (level_data->enemy_count == 0) {
            level_data->flock_num++;
            entity_reset();

            if (level_data->flock_num >= get_flock_count()) {
                level_data->flock_num = -1;
                return;
            }
            level_generate_enemy_flock(level_data);
        }
    }

    // enemy spawning (TODO: remove later)
//if (level_data->enemy_count < 5 && world->enemy_start)
   // level_load_enemy_random(p_data);
}

LevelData* get_level_data() {
    if (!level_data) return NULL;

    return level_data;
}

/**
* procedural content generator
*	- provide a palette/framework for the content
*	- at what time frame should you spawn things?
*	- provide different cases/scenarios for certain things
*	- weigh the choices based on the world state
*/