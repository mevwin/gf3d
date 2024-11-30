#include "simple_logger.h"
#include "gf2d_mouse.h"
#include "gfc_input.h"
#include "gfc_audio.h"
#include "world.h"
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
    level->enemy_goal = 5;
    level->enemy_killed_total = 0;
    level->emper_flag = 0;
    level->fencer_flag = 0;

    level->wave_end = 0;
    level->wave_count = 1;

    level->obj_type = KILL_ENEMY;

    //level->asteroid = gf3d_model_load("models/trench/asteroid.model");
    //level->asteroid_list = gfc_list_new_size(ASTEROID_MAX);

    //TODO: create level from config
    
    //get_world_data()->level_assets_made = 1;
    atexit(level_free);
}

void level_load() {

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
}

void full_level_reset() {
    level->last_powerup = 0;
    level->enemy_count = 0;
    level->enemy_killed = 0;
    level->enemy_killed_total = 0;
    level->emper_flag = 0;
    level->fencer_flag = 0;

    level->wave_count = 1;
    level->wave_end = 0;
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
    WorldData* world;

    //level_visuals();

    world = get_world_data();
    
    switch (level->obj_type) {
        case KILL_ENEMY:
            if (level->enemy_killed == level->enemy_goal) {
                enemy_reset();
                new_wave_level_reset();
                player_upgrade();
                shop_reset();

                world->current_state = WAVE_COMPLETED;
            }
            break;
        case SURVIVE:

            break;
    }
}

LevelData* get_level_data() {
    if (!level) return NULL;

    return level;
}