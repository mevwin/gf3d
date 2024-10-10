#include "simple_logger.h"
#include "gf3d_camera.h"
#include "player.h"

void player_movement(Entity* self){
    PlayerData* data;

    data = self->data;
    if (!data) return;

    if (!self) return;

    GFC_Vector3D updir = { 0, 0, data->upspeed };
    GFC_Vector2D rigdir = { data->rigspeed, 0 };
    GFC_Vector2D fordir = { 0, data->forspeed };

    gfc_vector3d_rotate_about_x(&updir, self->rotation.x);
    rigdir = gfc_vector2d_rotate(rigdir, self->rotation.z);

    if (gfc_input_command_down("moveup")) {
        gfc_vector3d_add(self->position, self->position, updir);

        // animation panning up
        if (self->rotation.y > -0.25)
            self->rotation.y -= 0.05;
        else self->rotation.y = -0.25;
    }

    if (gfc_input_command_down("movedown")) {
        gfc_vector3d_add(self->position, self->position, -updir);

        // animation panning down
        if (self->rotation.y < 0.25)
            self->rotation.y += 0.05;
        else self->rotation.y = 0.25;
    }

    if (gfc_input_command_down("moveright")) {
        gfc_vector2d_add(self->position, self->position, -rigdir);

        // animation panning right
        if (self->rotation.x > -0.25 && self->rotation.z > -0.25) {
            self->rotation.x -= 0.05;
            self->rotation.z -= 0.05;
        }
        else {
            self->rotation.x = -0.25;
            self->rotation.z = -0.25;
        }
    }

    if (gfc_input_command_down("moveleft")) {
        gfc_vector2d_add(self->position, self->position, rigdir);

        // animation panning left
        if (self->rotation.x < 0.25 && self->rotation.z < 0.25) {
            self->rotation.x += 0.05;
            self->rotation.z += 0.05;
        }

        else {
            self->rotation.x = 0.25;
            self->rotation.z = 0.25;
        }
    }
}

void undo_anim(Entity* self) {
    PlayerData* data;

    data = self->data;
    if (!data) return;

    if (!self) return;

    if (self->rotation.y < 0 && !gfc_input_command_down("moveup"))
        self->rotation.y += 0.01;

    if (self->rotation.y > 0 && !gfc_input_command_down("movedown"))
        self->rotation.y -= 0.01;

    if (self->rotation.x < 0 && !gfc_input_command_down("moveright"))
        self->rotation.x += 0.01;

    if (self->rotation.x > 0 && !gfc_input_command_down("moveleft"))
        self->rotation.x -= 0.01;

    if (self->rotation.z < 0 && !gfc_input_command_down("moveright"))
        self->rotation.z += 0.01;

    if (self->rotation.z > 0 && !gfc_input_command_down("moveleft"))
        self->rotation.z -= 0.01;

    if (self->position.y != data->og_pos.y) self->position.y = data->og_pos.y;
}

void player_cam(Entity* self) {
    GFC_Vector3D lookTarget, camera, dir = { 0 };
    PlayerData* data;

    if (!self) return;
    data = self->data;
    if (!data) return;

    // camera_view
    if (!(data->freelook)) {
        gfc_vector3d_copy(lookTarget, gfc_vector3d(0, self->position.y, 0));
        camera = gfc_vector3d(0, self->position.y + 70, 8);
        gf3d_camera_look_at(lookTarget, &camera);
    }
    else slog("Free Look Enabled");
}

