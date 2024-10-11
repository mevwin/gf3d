#include "simple_logger.h"
#include "gf3d_camera.h"
#include "gf2d_mouse.h"
#include "player.h"
#include "player_move.h"


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

    data = gfc_allocate_array(sizeof(PlayerData), 1);
    if (data) self->data = data;

    data->upspeed = (float) 1.2;
    data->rigspeed = (float) 1.2;

    data->og_pos = self->position;
    data->mid_roll = 0;

    data->x_bound = 49; // left is positive, right is negative
    data->z_bound = 35;

    // 98 x 70

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
    GFC_Vector2D cursor_pos;
    int x, y;

    if (!data) return;
    if (!self) return;

    cursor_pos = gf2d_mouse_get_position();
    mousepos_to_gamepos(&cursor_pos, data);

    slog("MouseX: %f, MouseY: %f", cursor_pos.x, cursor_pos.y);
    slog("ShipX: %f, ShipY: %f", self->position.x, self->position.z);

    
    //if (gf2d_mouse_get_cast_ray())
        //slog("ShipX: %f, ShipY: %f", self->position.x, self->position.z);
}


