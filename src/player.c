#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_vector.h"
#include "gf2d_mouse.h"
#include "player.h"
#include "player_move.h"
#include "projectile.h"
#include "reticle.h"

Entity* player_spawn() {
    Entity* self;
    PlayerData* data;
    GFC_Vector3D position, reticle_pos;

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
    self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                            self->position.y - (self->model->bounds.h / 2),
                            self->position.z - (self->model->bounds.d / 2),
                            self->model->bounds.w,
                            self->model->bounds.h,
                            self->model->bounds.d);

    self->position = position;
    data->og_pos = self->position;

    reticle_pos = gfc_vector3d(position.x, -60, position.z);
    data->reticle_pos = reticle_spawn(reticle_pos);

    // remaining data init
    data->change_flag = 1;
    data->take_damage_timing = 0;
    data->mid_roll = 0;
    data->wave_flag = 0;
    data->nuke_flag = 0;
    data->proj_count = 0;
    data->took_damage = 0;
    data->damage_taken = 0;

    data->currHealth = 2;
    data->maxHealth = 50;
    data->player_dead = 0;

    // default attack init
    data->curr_mode = SINGLE_SHOT; 
    data->base_damage = 1.0;
    data->proj_speed = 8.0;

    // charge_shot init
    data->next_charged_shot = (SDL_GetTicks() / 1000.0) + 0.9;
    data->charge_shot_delay = 0;

    // debug init
    data->freelook = 0;
    data->no_attack = 0;

    player_count++;

    return self;
}

void player_think(Entity* self) {
    PlayerData* data;
    float time;

    if (!self) return;

    data = self->data;
    if (!data) return;

    // can't do anything if player is dead
    if (data->player_dead) return;
    
    // movement checks
    if (!data->mid_roll)
        player_movement(self, data);
    else
        barrel_roll(self, data);

    time = SDL_GetTicks() / 1000.0;
    
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
        ) {
        data->next_charged_shot = time + 2.5;
        player_attack(self, data);
    }

    // debug tools
    if (gfc_input_command_pressed("freelook")) {
        data->freelook = !data->freelook;
        gf3d_camera_enable_free_look(data->freelook);
    }

    if (gfc_input_command_pressed("change_attack")) {
        //data->curr_mode++;
        
        //if (data->curr_mode > SUPER_NUKE) data->curr_mode = SINGLE_SHOT;
        if (!data->no_attack) {
            data->no_attack = 1;
            player_no_attack = 1;
        }
        else {
            data->no_attack = 0;
            player_no_attack = 0;
        }
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

    // update camera
    player_cam(self, data);

    // updates model based on current attack type
    time = SDL_GetTicks() / 1000.0;

    // CHARGE_SHOT texture
    if (time >= data->next_charged_shot && 
        time < data->next_charged_shot + 0.03 && 
        data->curr_mode != WAVE_SHOT &&
        !data->took_damage
        ){
        data->curr_mode = CHARGE_SHOT;
        gf3d_texture_free(self->model->texture);
        self->model->texture = gf3d_texture_load("models/player_ship/color_44.png");
    }
    // SINGLE_SHOT texture
    else if (data->take_damage_timing < time &&
             data->change_flag && 
             data->curr_mode != WAVE_SHOT &&
             !data->took_damage
        ) {
        data->curr_mode = SINGLE_SHOT;
        gf3d_texture_free(self->model->texture);
        self->model->texture = gf3d_texture_load("models/player_ship/color_77.png");
        data->change_flag = 0;
    }
    // WAVE_SHOT texture
    /*
    else if (data->curr_mode == WAVE_SHOT && 
            data->change_flag &&
            !data->took_damage
        ) {
        self->model->texture = gf3d_texture_load("models/player_ship/color_66.png");
        data->change_flag = 0;
    }
    */
    // updates hurtbox
    self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                            self->position.y - (self->model->bounds.h / 2),
                            self->position.z - (self->model->bounds.d / 2),
                            self->model->bounds.w,
                            self->model->bounds.h,
                            self->model->bounds.d);

    // check if player was hurt
    if (data->took_damage)
        player_take_damage(self, data, time);

    // check if player is dead
    if (data->currHealth <= 0.0)
        player_die(self);
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

    if (!data) return;
    if (!self) return;
    
    gfc_vector3d_copy(attack_start, self->position);
    cursor_pos.x = data->reticle_pos->x;
    cursor_pos.y = data->reticle_pos->y;
    cursor_pos.z = data->reticle_pos->z;

    // creates projectile under the ship
    attack_start.z -= 3;
    curr_time = SDL_GetTicks() / 1000.0;
    player_proj_spawn(attack_start, cursor_pos, self, curr_time);
}

void player_take_damage(Entity* self, PlayerData* data, float time) {
    if (!data) return;

    //slog("test");
    gf3d_texture_free(self->model->texture);
    self->model->texture = gf3d_texture_load("models/player_ship/color_EE.png");
    data->change_flag = 1;
    data->take_damage_timing = time + 0.5;

    data->currHealth -= data->damage_taken;
    data->took_damage = 0;
    data->damage_taken = 0;
}

void player_die(Entity* self) {
    PlayerData* data;

    data = self->data;
    
    data->player_dead = 1;

    self->rotation.y -= 0.2;
    if (data->proj_count > 0 && self->rotation.y != -0.24) {
        return;
    }
    player_death(self);

}

void player_death(Entity* self) {
    player_count--;
    if (player_count != 0) player_count = 0;
    player_no_attack = 1;
    entity_free(self);
}


