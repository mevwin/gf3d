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

    gfc_vector3d_rotate_about_x(&updir, self->rotation.x);
    rigdir = gfc_vector2d_rotate(rigdir, self->rotation.z);

    if (gfc_input_command_down("moveup")) {
        gfc_vector3d_add(self->position, self->position, updir);

        // animation panning up
        if (self->rotation.y > -0.25)
            self->rotation.y -= 0.03;
        else self->rotation.y = -0.25;
    }

    if (gfc_input_command_down("movedown")) {
        gfc_vector3d_add(self->position, self->position, -updir);

        // animation panning down
        if (self->rotation.y < 0.25)
            self->rotation.y += 0.04;
        else self->rotation.y = 0.25;
    }

    if (gfc_input_command_down("moveright")) {
        gfc_vector2d_add(self->position, self->position, -rigdir);

        // animation panning right
        if (self->rotation.x > -0.25 && self->rotation.z > -0.25) {
            self->rotation.x -= 0.04;
            self->rotation.z -= 0.04;
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
            self->rotation.x += 0.04;
            self->rotation.z += 0.04;
        }

        else {
            self->rotation.x = 0.25;
            self->rotation.z = 0.25;
        }
    }

    // undoes rotation on ship when player isn't pressing a button
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

    // barrel_roll checks
    if (gfc_input_command_released("moveup") && 
        gfc_input_command_pressed("movedown") && 
        self->rotation.y < -0.08 &&
        self->rotation.y > -0.2
        ) {
            data->mid_roll = 1;
            data->roll = DOWN;
    }
    if (gfc_input_command_released("moveleft") &&
        gfc_input_command_pressed("moveright") &&
        self->rotation.x > 0.16 &&
        self->rotation.x < 0.2
        ) {
        data->mid_roll = 1;
        data->roll = RIGHT;
    }
    if (gfc_input_command_released("movedown") &&
        gfc_input_command_pressed("moveup") &&
        self->rotation.y > 0.08 &&
        self->rotation.y < 0.2
        ) {
        data->mid_roll = 1;
        data->roll = UP;
    }
    if (gfc_input_command_released("moveright") &&
        gfc_input_command_pressed("moveleft") &&
        self->rotation.x < -0.16 &&
        self->rotation.x > -0.2
        ) {
        data->mid_roll = 1;
        data->roll = LEFT;
    }
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

void barrel_roll(Entity* self){
    PlayerData* data;

    data = self->data;
    if (!data) return;

    if (!self) return;

    GFC_Vector3D updir = { 0, 0, data->upspeed * 3};
    GFC_Vector2D rigdir = { data->rigspeed*3, 0 };

    gfc_vector3d_rotate_about_x(&updir, self->rotation.x);
    rigdir = gfc_vector2d_rotate(rigdir, self->rotation.z);

    if (data->roll == DOWN) {
        gfc_vector3d_add(self->position, self->position, -updir);

        data->upspeed -= 0.1;

        if (data->upspeed < 0) {
            data->mid_roll = 0;
            data->upspeed = 1.2;
        }
    }
    else if (data->roll == RIGHT) {
        gfc_vector2d_add(self->position, self->position, -rigdir);

        data->rigspeed -= 0.1;

        if (data->rigspeed < 0) {
            data->mid_roll = 0;
            data->rigspeed = 1.2;
        }
    }
    else if (data->roll == UP) {
        gfc_vector3d_add(self->position, self->position, updir);

        data->upspeed -= 0.1;

        if (data->upspeed < 0) {
            data->mid_roll = 0;
            data->upspeed = 1.2;
        }
    }
    else if (data->roll == LEFT) {
        gfc_vector2d_add(self->position, self->position, rigdir);

        data->rigspeed -= 0.1;

        if (data->rigspeed < 0) {
            data->mid_roll = 0;
            data->rigspeed = 1.2;
        }
    }
}
