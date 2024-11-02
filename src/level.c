#include "simple_logger.h"
#include "player.h"
#include "enemy.h"
#include "ui.h"
#include "level.h"

static LevelData* level;

void level_init() {
    int i, x_start, y_start, z_start;
    float neg;
    Entity* asteroid;

    level = gfc_allocate_array(sizeof(LevelData), 1);
    
    level->game_start = 0;
    level->enemy_start = 0;
    level->_done = 0;
    
    level->enemy_count = 0;
    level->enemy_killed = 0;
    level->fencer_flag = 0;
    
    level->wave_count = 0;
    level->in_shop = 0;
    level->paused = 0;
    level->wave_end = 0;

    // asteroid init
    level->asteroid = gf3d_model_load("models/trench/asteroid.model");
    level->asteroid_list = gfc_list_new_size(ASTEROID_MAX);
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

    atexit(level_free);
}

void level_visuals() {
    int i;
    Entity* curr;
    
    if (level->in_shop || level->paused || level->wave_end) return;

    for (i = 0; i < ASTEROID_MAX; i++) {
        curr = (Entity*) gfc_list_nth(level->asteroid_list, i);
        
        if (i % 2 == 0) {
            curr->rotation.x += 0.01f;
            curr->position.y += 3.0f;
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
            wave_completed(p_data, level);
        }
    }
}

void level_free() {
    gf3d_model_free(level->asteroid);
    gfc_list_foreach(level->asteroid_list, (void (*) (void*)) entity_free);
    gfc_list_delete(level->asteroid_list);
    free(level);
}

LevelData* get_level_data() {
    if (!level) return NULL;

    return level;
}