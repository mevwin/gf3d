#include "simple_logger.h"
#include "gf3d_camera.h"
#include "player.h"

Entity* player_spawn(GFC_Vector3D position) {
    Entity* self;
    PlayerData* data;

    self = entity_new();
    if (!self) return NULL;

    self->model = gf3d_model_load("models/dino.model");
    self->think = player_think;
    self->update = player_update;
    self->position = position;
    self->free = player_free;

    data = gfc_allocate_array(sizeof(PlayerData), 1);
    if (data) self->data = data;

    data->upspeed = (float) 0.39;
    data->rigspeed = (float) 0.39;
    data->forspeed = (float) -0.30;

    return self;
}

void player_think(Entity* self) {
    PlayerData* data;
    int dx, dy;

    data = self->data;
    if (!data) return;

    if (!self) return;

    GFC_Vector3D updir = { 0, 0, data->upspeed };
    GFC_Vector2D rigdir = { data->rigspeed, 0 };
    GFC_Vector2D fordir = { 0, data->forspeed };

    //self->rotation.z = self->rotation.z + 0.01;
    gfc_vector3d_rotate_about_x(&updir, self->rotation.x);
    rigdir = gfc_vector2d_rotate(rigdir, self->rotation.z);

    //input.cfg, include gfc_input.h
    if (gfc_input_command_down("moveup")) {
        gfc_vector3d_add(self->position, self->position, updir);
    }

    if (gfc_input_command_down("movedown")) {
        gfc_vector3d_add(self->position, self->position, -updir);
    }

    if (gfc_input_command_down("moveright")) {
        gfc_vector2d_add(self->position, self->position, -rigdir);
    }

    if (gfc_input_command_down("moveleft")) {
        gfc_vector2d_add(self->position, self->position, rigdir);
    }

    if (gfc_input_command_down("freelook")) {
        data->freelook = !data->freelook;
    }
    if (gfc_input_command_down("lookforward")) {
        self->rotation.z = 0;
    }
    
    // auto-move
    gfc_vector2d_add(self->position, self->position, fordir);
}

//gf3d_camera.h
void player_update(Entity* self) {
    GFC_Vector3D lookTarget, camera, dir = { 0 };

    if (!self) return;
    
    gfc_vector3d_copy(lookTarget, self->position);
    dir.y = -1.0;
    gfc_vector3d_rotate_about_z(&dir, self->rotation.z);
    gfc_vector3d_sub(camera, self->position, dir);

    //camera offset
    camera.y += 50;
    camera.z += 10;

    
    lookTarget.y -= 30;
    lookTarget.z = 0;
    //slog("Player Position: %f, %f, %f", self->position.x, self->position.y, self->position.z);
    gf3d_camera_look_at(lookTarget, &camera);
}


void player_free(Entity* self){
    PlayerData *data;

    if (!self) return;

    data = (PlayerData*) self->data;
    free(data);
    self->data = NULL;
}


