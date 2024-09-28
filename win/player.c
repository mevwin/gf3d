#include "simple_logger.h"
#include "gf3d_camera.h"
#include "player.h"

typedef struct {
    Uint8   freelook;
    //        cameraPitch; //raise and lower target instead


}PlayerData;


Entity* player_spawn(GFC_Vector3D position) {
    Entity* self;
    PlayerData* data;

    self = entity_new();
    if (!self) return NULL;

    self->model = gf3d_model_load("models/dino.model");
    self->think = player_think;
    self->update = player_update;
    self->position = position;
    data = gfc_allocate_array(sizeof(PlayerData), 1);
    if (data) self->data = data;

    return self;
}

void player_think(Entity* self) {
    GFC_Vector3D updir = { 0, 0, 1 };
    GFC_Vector2D rigdir = { 1, 0 };
    PlayerData* data;

    data = self->data;
    if (!data) return;

    if (!self) return;
    //self->rotation.z = self->rotation.z + 0.01;
    gfc_vector3d_rotate_about_x(&updir, self->rotation.x);
    rigdir = gfc_vector2d_rotate(rigdir, self->rotation.z);

    //input.cfg, include gfc_input.h
    if (gfc_input_command_down("walkforward")) {
        gfc_vector3d_add(self->position, self->position, updir);
    }

    if (gfc_input_command_down("walkback")) {
        gfc_vector3d_add(self->position, self->position, -updir);
    }

    if (gfc_input_command_down("walkright")) {
        gfc_vector2d_add(self->position, self->position, -rigdir);
    }

    if (gfc_input_command_down("walkleft")) {
        gfc_vector2d_add(self->position, self->position, rigdir);
    }

    if (gfc_input_command_down("freelook")) {
        data->freelook = !data->freelook;
    }
    GFC_Vector2D fordir = { 0, -1 };
    gfc_vector2d_add(self->position, self->position, fordir);
}

//gf3d_camera.h
void player_update(Entity* self) {
    GFC_Vector3D lookTarget, camera, dir = { 0 };
    //int dx, dy;
    //SDL_GetRelativeMouseState(&dx, &dy);

    if (!self) return;
    
    gfc_vector3d_copy(lookTarget, self->position);
    dir.y = -1.0;


    gfc_vector3d_rotate_about_z(&dir, self->rotation.z);
    gfc_vector3d_sub(camera, self->position, dir);
    
    //lookTarget.x += dx;
    //lookTarget.y += dy;
    camera.y += 50;
    camera.z += 10;




    gf3d_camera_look_at(lookTarget, &camera);
}

/*
void player_free(self){
    DinoData *data;
    if !self reutnr
    data = (DinoData*) self->data;
    free(data);
    self->data = NULL;
}

*/


