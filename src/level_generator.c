#include "simple_logger.h"
#include "gfc_config.h"
#include "gfc_audio.h"
#include "gfc_list.h"
#include "world.h"
#include "level.h"
#include "player.h"
#include "item.h"
#include "enemy.h"
#include "level_generator.h"

typedef struct LevelGenManager_S {
    SJson*      level_palette;
    SJson*      level_param;
    Uint32      flock_count;
    Uint32      enemy_per_flock;
}LevelGenManager;

static LevelGenManager gen_manager = { 0 };

void level_new_enemy_per_flock();

void level_gen_init(SJson* level_def) {
    gen_manager.level_palette = level_def;
    gen_manager.flock_count = 0;
    gen_manager.enemy_per_flock = 0;

    atexit(level_gen_close);
}

void level_gen_close() {
    sj_free(gen_manager.level_palette);
    gen_manager.flock_count = 0;
    gen_manager.enemy_per_flock = 0;

    if (get_world_data()->_done)
        memset(&gen_manager, 0, sizeof(LevelGenManager));
}

void generate_level() {
    SJson* data, *array;
    WorldData* world;
    LevelData* level;
    GFC_Vector2D rand_bounds;
    int i, j, k, rand_low, rand_high;
    char buffer[30];

    world = get_world_data();
    level = get_level_data();

    if (world->current_state == WAVE_COMPLETED) {
        switch (level->endless_chosen_level) {
            case 1:
                data = level->endless_prev1;
                sj_free(level->endless_prev2);
                break;

            default:
                data = level->endless_prev2;
                sj_free(level->endless_prev1);
        }
        sj_object_get_value_as_int(data, "obj_type", &level->obj_type);
        sj_object_get_value_as_int(data, "level_type", &level->level_type);
        strcpy(level->level_obj, sj_object_get_value_as_string(data, "obj_desc"));
        sj_object_get_value_as_int(data, "flock_count", &gen_manager.flock_count);

        switch (level->obj_type) {
            case KILL_ENEMY:
                sj_object_get_value_as_int(data, "level_goal", &level->enemy_goal);
                break;
            case SURVIVE:
                sj_object_get_value_as_int(data, "level_goal", &i);
                level->survival_time = i * 60.0f;
                break;
        }
        array = sj_object_get_value(gen_manager.level_palette, "level_palette");
        i = (int) level->obj_type;
        gen_manager.level_param = sj_array_get_nth(array, i);

        sj_free(data);
    }
    else { // create one default level;
        // get obj type
        array = sj_object_get_value(gen_manager.level_palette, "level_palette");
        i = gfc_random_int(array->v.array->count); // index
        level->obj_type = (ObjType)i;
        gen_manager.level_param = sj_array_get_nth(array, i);

        // get level type
        data = sj_object_get_value(gen_manager.level_param, "level_types");
        i = gfc_random_int(data->v.array->count); // index
        sj_get_integer_value(sj_array_get_nth(data, i), &j);
        level->level_type = (LevelType)j;

        // initialize level obj desc and goal
        switch (level->obj_type) {
            case BOSS:
                full_level_reset();
                level_gen_close();
                world->current_state = START_MENU;

                return;

            default:
                sj_value_as_vector2d(sj_object_get_value(gen_manager.level_param, "level_goal"), &rand_bounds);
        }
        rand_low = (int)rand_bounds.x;
        rand_high = (int)rand_bounds.y;
        j = rand_low + gfc_random_int((rand_high - rand_low) + 1);

        data = sj_object_get_value(level->level_base, "level_obj");
        k = (int)level->obj_type;
        if (level->obj_type == BOSS) // boss stage does not need sprintf
            strcpy(level->level_obj, sj_object_get_value_as_string(sj_array_get_nth(data, k), "obj_desc"));
        else {
            sprintf(buffer, sj_object_get_value_as_string(sj_array_get_nth(data, k), "obj_desc"), j);
            strcpy(level->level_obj, buffer);
        }

        switch (level->obj_type) {
            case KILL_ENEMY:
                level->enemy_goal = j;
                break;
            case SURVIVE:
                level->survival_time = j * 60.0f;
                break;
        }

        // initialize flock count for editor 
        sj_value_as_vector2d(sj_object_get_value(gen_manager.level_param, "flock_count"), &rand_bounds);
        rand_low = (int)rand_bounds.x;
        rand_high = (int)rand_bounds.y;

        gen_manager.flock_count = rand_low + gfc_random_int(rand_high - rand_low);
    }

    if (level->wave_count > 1 && world->current_state == SHOP) {
        slog("true");
        level_generate_enemy_flock(level);
    }
}

SJson* generate_level_preview() {
    WorldData* world;
    LevelData* level;
    SJson* new_level, *data, *array, *param;
    GFC_Vector2D rand_bounds;
    int i, j, k, rand_low, rand_high, obj_type;
    char buffer[30];

    new_level = sj_object_new();

    world = get_world_data();
    level = get_level_data();

    // get obj type
    array = sj_object_get_value(gen_manager.level_palette, "level_palette");
    i = gfc_random_int(array->v.array->count); // index
    sj_object_insert(new_level, "obj_type", sj_new_int(i));
    param = sj_array_get_nth(array, i);
    obj_type = i;

    // get level type
    data = sj_object_get_value(param, "level_types");
    i = gfc_random_int(data->v.array->count); // index
    sj_get_integer_value(sj_array_get_nth(data, i), &k);
    sj_object_insert(new_level, "level_type", sj_new_int(k));

    // initialize level obj desc and goal
    switch (obj_type) {
        case BOSS:
            full_level_reset();
            level_gen_close();
            world->current_state = START_MENU;

            return;

        default:
            sj_value_as_vector2d(sj_object_get_value(param, "level_goal"), &rand_bounds);
    }
    rand_low = (int)rand_bounds.x;
    rand_high = (int)rand_bounds.y;
    j = rand_low + gfc_random_int((rand_high - rand_low) + 1);

    data = sj_object_get_value(level->level_base, "level_obj");
    if (obj_type == BOSS) { // boss stage does not need sprintf
        strcpy(buffer, sj_object_get_value_as_string(sj_array_get_nth(data, obj_type), "obj_desc"));
    }
    else {
        sprintf(buffer, sj_object_get_value_as_string(sj_array_get_nth(data, obj_type), "obj_desc"), j);
    }
    sj_object_insert(new_level, "obj_desc", sj_new_str(buffer));
    sj_object_insert(new_level, "level_goal", sj_new_int(j));

    // initialize flock count for editor 
    sj_value_as_vector2d(sj_object_get_value(gen_manager.level_param, "flock_count"), &rand_bounds);
    rand_low = (int)rand_bounds.x;
    rand_high = (int)rand_bounds.y;
    j = rand_low + gfc_random_int(rand_high - rand_low);
    sj_object_insert(new_level, "flock_count", sj_new_int(j));

    return new_level;
}

void level_generate_enemy_flock(void* level_data) {
    SJson* array, *data;
    PlayerData* p_data;
    LevelData* level;
    int i, j, k, l;
    EnemyType enemy_type;

    level = (LevelData*) level_data;
    p_data = get_player_data();

    // initialize enemy per flock for editor (will be different per flock)
    level_new_enemy_per_flock();
    data = sj_object_get_value(gen_manager.level_param, "enemy_spawns");

    /*
    array = sj_object_get_value(data, "enemy_types");
    for (i = 0; i < *(&gen_manager.enemy_type_buffer + 1) - gen_manager.enemy_type_buffer; i++) {
        sj_get_integer_value(sj_array_get_nth(array, i), &k);
        gen_manager.enemy_type_buffer[i] = k;
    }
    */

    for (i = 0; i < gen_manager.enemy_per_flock; i++) {
        // get allowable enemy types
        array = sj_object_get_value(data, "enemy_types");
        j = gfc_random_int(array->v.array->count); // index
        sj_get_integer_value(sj_array_get_nth(array, j), &k);
        enemy_type = (EnemyType) k;

        /*
        if (enemy_type == FENCERS) {
            if (level->fencer_flag) {
                //reroll, exclude fencers
                j = gfc_random_int(array->v.array->count);

            }
            else 
                level->fencer_flag = 1;
        }
        */
        

        // get allowable item types
        array = sj_object_get_value(data, "held_items");
        j = gfc_random_int(array->v.array->count); // index
        sj_get_integer_value(sj_array_get_nth(array, j), &l);

        enemy_spawn(p_data->player_pos, enemy_type, NULL, l);
    }

    /*
    type = (EnemyType)gfc_random_int(5);

    // only one fencer on-screen
    if (type == FENCERS && !level->fencer_flag)
        level->fencer_flag = 1;
    else if (type == FENCERS && level->fencer_flag)
        type = (EnemyType)gfc_random_int(4);

    // only one emper on-screen
    if (type == EMPERS && !level->emper_flag)
        level->emper_flag = 1;
    else if (type == EMPERS && level->emper_flag)
        type = (EnemyType)gfc_random_int(3);


    //enemy_spawn(p_data->player_pos, type, NULL);
    */
}

void level_new_enemy_per_flock() {
    GFC_Vector2D rand_bounds;
    int rand_low, rand_high;

    sj_value_as_vector2d(sj_object_get_value(gen_manager.level_param, "enemy_per_flock"), &rand_bounds);
    rand_low = (int)rand_bounds.x;
    rand_high = (int)rand_bounds.y;

    gen_manager.enemy_per_flock = rand_low + gfc_random_int(rand_high - rand_low);
}

Uint32 get_flock_count() {
    return gen_manager.flock_count;
}