#include "simple_logger.h"
#include "gf3d_camera.h"
#include "gfc_shape.h"
#include "gfc_input.h"
#include "player.h"
#include "player_move.h"
#include "enemy.h"
#include "projectile.h"
#include "reticle.h"


Entity* player_spawn(GFC_Vector3D position) {
    Entity* self;
    PlayerData* data;
    GFC_Vector2D cursor;

    self = entity_new();
    if (!self) return NULL;

    self->model = gf3d_model_load("models/player_ship/player_ship_single.model");
    self->think = player_think;
    self->update = player_update;

    self->position = position;
    self->free = player_free;
    self->entity_type = PLAYER;

    data = gfc_allocate_array(sizeof(PlayerData), 1);
    if (data) self->data = data;

    data->upspeed = 1.2;
    data->rigspeed = 1.2;

    data->og_pos = self->position;
    data->mid_roll = 0;

    data->x_bound = 49; // left is positive, right is negative
    data->z_bound = 35; // 98 x 70

    
    data->curr_mode = SINGLE_SHOT; //default attack
    data->next_charged_shot = (SDL_GetTicks() / 1000.0) + 2.0;
    data->shot_delay = 0;

    data->change_flag = 1;

    data->hurt_box = gfc_rect(
        self->position.x - 3,
        self->position.z + 3,
        6.0,
        -6.0
    );

    data->reticle = reticle_spawn(gfc_vector3d(position.x, -40, position.z));

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
    
    if (time >= data->next_charged_shot && time < data->next_charged_shot + 0.03) {
        data->curr_mode = CHARGE_SHOT;
        gf3d_model_free(self->model);
        self->model = gf3d_model_load("models/player_ship/player_ship_charged.model");
    }
    else if (time < data->next_charged_shot && data->change_flag) {
        data->curr_mode = SINGLE_SHOT;
        gf3d_model_free(self->model);
        self->model = gf3d_model_load("models/player_ship/player_ship_single.model");
        data->change_flag = 0;
    }

    if (gf2d_mouse_button_pressed(0) && data->curr_mode == CHARGE_SHOT) {
        player_attack(self, data);
        data->next_charged_shot = time + 2.0;
        data->shot_delay = time + 0.75;
        data->change_flag = 1;
    }
    else if ((gf2d_mouse_button_pressed(0) || gf2d_mouse_button_held(0)) &&
        data->shot_delay <= time) {
        data->next_charged_shot = time + 2.5;
        player_attack(self, data);
        //slog("Time: %0.3f", data->next_charged_shot);
    }

    if (gfc_input_command_pressed("freelook")) {
        data->freelook = !data->freelook;
        gf3d_camera_enable_free_look(data->freelook);
    }
    //slog("X: %f, Y: %f, Z: %f", self->position.x, self->position.y, self->position.z);
}

//gf3d_camera.h
void player_update(Entity* self) {
    PlayerData* data;

    data = self->data;
    if (!data) return;

    player_cam(self, data);

    ease_anim(self,data);

    //updates hurtbox
    data->hurt_box = gfc_rect(
        self->position.x - 3,
        self->position.z + 3,
        6.0,
        -6.0
    );
}


void player_free(Entity* self){
    PlayerData *data;

    if (!self) return;

    data = (PlayerData*) self->data;
    free(data);
    self->data = NULL;
}

void player_attack(Entity* self, PlayerData* data) {
    //Entity* entity_list;
    Entity* proj;
    GFC_Vector3D attack_start, attack_dest, center;
    GFC_Vector2D cursor_pos;
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
    attack_start.z -= 3;

    curr_time = SDL_GetTicks() / 1000.0;

    proj = proj_spawn(attack_start, data->curr_mode, curr_time);

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

    /*
    ray = gf2d_mouse_get_cast_ray();
    slog("\nPoint A: %f, %f, %f\nPoint B: %f, %f, %f", 
        ray.a.x, ray.a.y, ray.a.z,
        ray.b.x, ray.b.y, ray.b.z
        );
    */
}


