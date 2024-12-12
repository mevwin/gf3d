#include "simple_logger.h"
#include "gf2d_mouse.h"
#include "gfc_input.h"
#include "gfc_audio.h"
#include "world.h"
#include "player.h"
#include "enemy.h"
#include "ui.h"
#include "level.h"
#include "item.h"

typedef struct Level_S {
    Model*      terrain;
    GFC_List*   hazard_list;
    Uint8       hazard_count;
}Level;

static LevelData* level;

void level_init() {
    level = gfc_allocate_array(sizeof(LevelData), 1);

    //level->asteroid = gf3d_model_load("models/trench/asteroid.model");
    //level->asteroid_list = gfc_list_new_size(ASTEROID_MAX);

    //TODO: create level from config
    
    //get_world_data()->level_assets_made = 1;
    atexit(level_free);
}

void level_begin(Uint8 game_mode) {
    level->last_powerup = 0;
    level->enemy_count = 0;
    level->enemy_killed = 0;
    level->enemy_killed_total = 0;
    level->fencer_flag = 0;
    level->wave_end_time = 0;
    level->total_game_time = 0;
    level->wave_end = 0;
    level->obj_complete = 0;
    level->wave_count = 1;
    level->total_scrap = 0;
    level->emper_flag = 0;
    level->spawn_counter = 0;

    switch (game_mode) {
        case REGULAR:
            level->level_def = sj_load("levels/regular_levels.def");
            level_load(REGULAR);

            break;

        case ENDLESS:
            level->level_def = sj_load("levels/endless.def");
            level_load(ENDLESS);

            break;
    }
}

// load a level based on wave_count-1
void level_load(Uint8 game_mode) {
    SJson* data, *curr_level;
    char buffer[30];
    int i;

    if (game_mode == REGULAR) {
        data = sj_object_get_value(level->level_def, "level_list");
        curr_level = sj_array_get_nth(data, level->wave_count - 1);
        
        sj_object_get_value_as_int(curr_level, "level_type", &i);
        level->level_type = (LevelType) i;

        sj_object_get_value_as_int(curr_level, "obj_type", &i);
        level->obj_type = (ObjType) i;

        sj_object_get_value_as_int(curr_level, "level_goal", &i);
        if (i == 0) // boss stage does not need sprintf
            strcpy(level->level_obj, sj_object_get_value_as_string(curr_level, "level_obj"));
        else {
            sprintf(buffer, sj_object_get_value_as_string(curr_level, "level_obj"), i);
            strcpy(level->level_obj, buffer);
        }

        switch (level->obj_type) {
            case KILL_ENEMY:
                level->enemy_goal = i;
                break;
            case SURVIVE:
                level->survival_time = i;
                break;
        }

        level->wave_goal = data->v.array->count;
        //level->wave_goal = 1;

        // load level assets

    }
    else if (game_mode == ENDLESS) {

    }
    else if (game_mode == PREV_DISPLAY) {
        // all done in game_data_init_from_save
    }

}

void new_wave_level_reset(){
    level->last_powerup = 0;
    level->enemy_count = 0;
    level->enemy_killed_total += level->enemy_killed;
    level->enemy_killed = 0;
    level->emper_flag = 0;
    level->fencer_flag = 0;

    level->wave_count++;
    level->wave_end = 1;

    if (get_player_data()->nuke_flag)
        get_player_data()->nuke_flag = 0;
}

void full_level_reset() {
    level->last_powerup = 0;
    level->enemy_count = 0;
    level->enemy_killed = 0;
    level->enemy_killed_total = 0;
    level->emper_flag = 0;
    level->fencer_flag = 0;

    level->wave_end_time = 0;
    level->total_game_time = 0;

    level->total_scrap = 0;

    level->wave_count = 1;
    level->wave_end = 0;

    if (level->level_def && get_world_data()->current_state != PREVIOUS_RUN)
        sj_free(level->level_def);
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
    Entity* item, *entityList;
    ItemData* i_data;
    PlayerData* p_data;
    WorldData* world;
    int i;

    //level_visuals();

    world = get_world_data();
    entityList = get_entityList();
    p_data = get_player_data();

    // player death check
    if (p_data->player_dead) {
        level->total_game_time += CURRENT_TIME - level->game_start;
        level->enemy_killed_total += level->enemy_killed;

        game_save(RUNSAVE);
        world->current_state = GAME_OVER;
    }

    // enemy spawning (TODO: remove later)
    if (level->enemy_count < 5 && world->enemy_start) {
        enemy_spawn(p_data->player_pos);
    }

    // check if level objective has been accomplished
    switch (level->obj_type) {
        case KILL_ENEMY:
            if (level->enemy_killed >= level->enemy_goal)
                level->obj_complete = 1;
            
            break;
        case SURVIVE:
            // if (CURRENT_TIME >= level->goal_timestamo)
            //  level->obj_complete = 1;
      
            break;
    }

    if (level->obj_complete) {
        enemy_reset();
        new_wave_level_reset();

        if (level->wave_count - 1 == level->wave_goal) {
            level->total_game_time += CURRENT_TIME - level->game_start;
            level->enemy_killed_total += level->enemy_killed;

            game_save(RUNSAVE);
            world->current_state = GAME_COMPLETED;
            return;
        }

        player_upgrade();
        shop_reset();

        for (i = 0; i < MAX_ENTITY; i++) {
            item = &entityList[i];
            if (item->entity_type != ITEM) continue;

            i_data = item->data;

            if (i_data->type == SCRAP || i_data->type == HEALTH_PICKUP)
                item_activate(item, i_data->type);
            else
                entity_free(item);
        }

        level->wave_end_time = CURRENT_TIME;
        level->total_game_time += (level->wave_end_time - level->game_start);

        level->obj_complete = 0;
        world->current_state = WAVE_COMPLETED;
    }

}

LevelData* get_level_data() {
    if (!level) return NULL;

    return level;
}

/**
* procedural content generator
*	- provide a palette/framework for the content
*	- at what time frame should you spawn things?
*	- provide different cases/scenarios for certain things
*	- weigh the choices based on the world state
*/