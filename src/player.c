#include "simple_logger.h"
#include "gf3d_camera.h"
#include "gf2d_mouse.h"
#include "player.h"
#include "player_move.h"
#include "enemy.h"
#include "projectile.h"


Entity* player_spawn(GFC_Vector3D position) {
    Entity* self;
    PlayerData* data;

    self = entity_new();
    if (!self) return NULL;

    self->model = gf3d_model_load("models/player_ship/player_ship.model");
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

    //default attack
    data->curr_mode = SINGLE_SHOT;
    

    return self;
}

void player_think(Entity* self) {
    PlayerData* data;

    if (!self) return;

    data = self->data;
    if (!data) return;
    
    if (!data->mid_roll)
        player_movement(self, data);
    else
        barrel_roll(self, data);

    if (gf2d_mouse_button_pressed(0) || gf2d_mouse_button_held(0))
        player_attack(self, data);

    if (gfc_input_command_pressed("freelook")) {
        data->freelook = !data->freelook;
        gf3d_camera_enable_free_look(data->freelook);
    }
    //slog("X: %f, Y: %f, Z: %f", self->position.x, self->position.y, self->position.z);
}

//gf3d_camera.h
void player_update(Entity* self) {
    player_cam(self, self->data);
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
    GFC_Vector2D cursor_pos;
    GFC_Vector3D attack_dest, player_pos;
    //GFC_Edge3D ray;
    //int i;

    if (!data) return;
    if (!self) return;
    
    //entity_list = get_entityList();
    //if (!entity_list) return;

    //cursor_pos = gf2d_mouse_get_position();
    //mousepos_to_gamepos(&cursor_pos, data);
    //attack_dest = gfc_vector3d(cursor_pos.x, -40, cursor_pos.y);

    //slog("MouseX: %f, MouseY: %f", cursor_pos.x, cursor_pos.y);

    proj = proj_spawn(self->position, data->curr_mode);

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
    ray = gf2d_mouse_get_cast_ray();
    slog("\nPoint A: %f, %f, %f\nPoint B: %f, %f, %f", 
        ray.a.x, ray.a.y, ray.a.z,
        ray.b.x, ray.b.y, ray.b.z
        );
    */
}


