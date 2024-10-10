#include "simple_logger.h"
#include "gf3d_camera.h"
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

    data->upspeed = (float) 0.8;
    data->rigspeed = (float) 0.8;
    data->forspeed = (float) -0.2;

    data->og_pos = self->position;

    return self;
}

void player_think(Entity* self) {
    PlayerData* data;

    data = self->data;
    if (!data) return;

    if (!self) return;
    
    player_movement(self);

    if (gfc_input_command_pressed("freelook")) {
        data->freelook = !data->freelook;
        gf3d_camera_enable_free_look(data->freelook);
    }

    
}

//gf3d_camera.h
void player_update(Entity* self) {
    undo_anim(self);
    player_cam(self);
}


void player_free(Entity* self){
    PlayerData *data;

    if (!self) return;

    data = (PlayerData*) self->data;
    free(data);
    self->data = NULL;
}


