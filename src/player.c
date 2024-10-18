#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_vector.h"
#include "player.h"
#include "player_move.h"
#include "projectile.h"
#include "reticle.h"


Entity* player_spawn() {
    Entity* self;
    PlayerData* data;
    GFC_Vector3D position;

    // sanity check
    self = entity_new();
    if (!self) return NULL;

    self->model = gf3d_model_load("models/player_ship/player_ship_single.model");
    self->think = player_think;
    self->update = player_update;
    self->free = player_free;
    self->entity_type = PLAYER;

    data = gfc_allocate_array(sizeof(PlayerData), 1);
    if (data) self->data = data;

    data->upspeed = 1.2;
    data->rigspeed = 1.2;

    data->x_bound = 49; // left is positive, right is negative
    data->z_bound = 35; // 98 x 70

    position = gfc_vector3d_random_pos(data->x_bound, 0, data->z_bound);
    self->position = position;

    data->og_pos = self->position;
    data->mid_roll = 0;
    data->wave_flag = 0;
    data->nuke_flag = 0;
    data->change_flag = 1;
    data->proj_count = 0;

    data->curr_mode = SINGLE_SHOT; //default attack
    data->base_damage = 0.3;
    data->proj_speed = 8.0;

    data->next_charged_shot = (SDL_GetTicks() / 1000.0) + 2.0;
    data->charge_shot_delay = 0;

    data->reticle = reticle_spawn(gfc_vector3d(position.x, -50, position.z));

    return self;
}

void player_think(Entity* self) {
    PlayerData* data;
    float time;

    if (!self) return;

    data = self->data;
    if (!data) return;
    
    if (!data->mid_roll)
        player_movement(self, data);
    else
        barrel_roll(self, data);

    time = SDL_GetTicks() / 1000.0;
    //slog("Time: %0.3f Next_shot %0.3f", time, data->next_charged_shot);
    
    // CHARGE_SHOT attack
    if (gf2d_mouse_button_pressed(0) && data->curr_mode == CHARGE_SHOT) {
        player_attack(self, data);
        data->next_charged_shot = time + 2.0;
        data->charge_shot_delay = time + 0.75;
        data->change_flag = 1;
    }
    // SINGLE_SHOT, WAVE_SHOT, MISSILE
    else if ((gf2d_mouse_button_pressed(0) || gf2d_mouse_button_held(0)) &&
        data->charge_shot_delay <= time &&
        !data->mid_roll &&
        data->curr_mode != CHARGE_SHOT
    ){
        data->next_charged_shot = time + 2.5;
        player_attack(self, data);
    }

    // debug tools
    if (gfc_input_command_pressed("freelook")) {
        data->freelook = !data->freelook;
        gf3d_camera_enable_free_look(data->freelook);
    }

    if (gfc_input_command_pressed("change_attack")) {
        data->curr_mode++;
        
        if (data->curr_mode > SUPER_NUKE) data->curr_mode = SINGLE_SHOT;
    }

    //slog("weapon: %d", data->curr_mode);
    //slog("X: %f, Y: %f, Z: %f", self->position.x, self->position.y, self->position.z);
}

//gf3d_camera.h
void player_update(Entity* self) {
    PlayerData* data;
    float time;

    data = self->data;
    if (!data) return;


    player_cam(self, data);
    time = SDL_GetTicks() / 1000.0;
    
    // updates model based on current attack type
    /*
    if (time >= data->next_charged_shot && time < data->next_charged_shot + 0.03 && data->curr_mode != WAVE_SHOT) {
        data->curr_mode = CHARGE_SHOT;
        gf3d_model_free(self->model);
        self->model = gf3d_model_load("models/player_ship/player_ship_charged.model");
    }
    else if (time < data->next_charged_shot && data->change_flag && data->curr_mode != WAVE_SHOT) {
        data->curr_mode = SINGLE_SHOT;
        gf3d_model_free(self->model);
        self->model = gf3d_model_load("models/player_ship/player_ship_single.model");
        data->change_flag = 0;
    }
    else if (data->curr_mode == WAVE_SHOT && !data->change_flag) {
        gf3d_model_free(self->model);
        self->model = gf3d_model_load("models/player_ship/player_ship_wave.model");
        data->change_flag = 1;
    }
    */

    //updates hurtbox
    self->hurtbox = gfc_sphere(self->position.x, self->position.y, self->position.z, self->model->bounds.h / 2);
}


void player_free(Entity* self){
    PlayerData *data;
    Entity* enemyList;

    if (!self) return;

    data = self->data;

    free(data);
    self->data = NULL;
}

void player_attack(Entity* self, PlayerData* data) {
    GFC_Vector3D attack_start, cursor_pos;
    float curr_time;
    //GFC_Edge3D ray;
    //int i;

    if (!data) return;
    if (!self) return;
    
    //entity_list = get_entityList();
    //if (!entity_list) return;

    /*
    cursor_pos = gf2d_mouse_get_position();
    mousepos_to_gamepos(&cursor_pos, data);
    attack_dest = gfc_vector3d(cursor_pos.x, self->position.y, cursor_pos.y);

    center = gfc_rect_get_center_point(data->hurt_box);

    if (cursor_pos.x > self->position.x - 3 &&
        cursor_pos.x <= self->position.x + 3 &&
        cursor_pos.y > self->position.z - 3 &&
        cursor_pos.y <= self->position.z + 3
        )
        slog("Attack Hit");
    else {
        slog("ShipX: %f, ShipY: %f", self->position.x, self->position.z);
        slog("CursorX: %f, CursorY: %f", cursor_pos.x, cursor_pos.y);
    }
    */

    gfc_vector3d_copy(attack_start, self->position);
    gfc_vector3d_copy(cursor_pos, data->reticle->position);
    attack_start.z -= 3;

    curr_time = SDL_GetTicks() / 1000.0;

    player_proj_spawn(attack_start, cursor_pos, self, curr_time);

    //slog("ShipX: %f, ShipY: %f", self->position.x, self->position.z);
    /*
    for (i = 0; i < MAX_ENEMY; i++) {
        if (entity_list[i].entity_type != ENEMY)
            continue;

        enemy = &entity_list[i];
        if (!enemy) continue;

        
        if (enemy->position.x == attack_dest.x &&
            enemy->position.y == attack_dest.y &&
            enemy->position.z == attack_dest.z ){
            slog("Attack Hit\n");
            break;
        }
        else {
            slog("EnemyX: %f, EnemyY: %f", enemy->position.x, enemy->position.z);
        }
            
    }
    */
}


