#include "simple_logger.h"
#include "gf3d_camera.h"
#include "gf3d_vgraphics.h"
#include "player.h"
#include "player_move.h"

void player_movement(Entity* self, PlayerData* data) {
    if (!data) return;
    if (!self) return;

    GFC_Vector3D updir = { 0, 0, data->upspeed };
    GFC_Vector2D rigdir = { data->rigspeed, 0 };

    gfc_vector3d_rotate_about_x(&updir, self->rotation.x);
    rigdir = gfc_vector2d_rotate(rigdir, self->rotation.z);

    if (gfc_input_command_down("moveup")) {
        if (!check_movebounds(self, updir, data))
            updir = gfc_vector3d(0, 0, 0);

        gfc_vector3d_add(self->position, self->position, updir);

        // animation panning up
        if (self->rotation.y > -0.25)
            self->rotation.y -= 0.03;
        else self->rotation.y = -0.25;
    }

    if (gfc_input_command_down("movedown")) {
        gfc_vector3d_negate(updir, updir);
        if (!check_movebounds(self, updir, data))
            updir = gfc_vector3d(0, 0, 0);

        gfc_vector3d_add(self->position, self->position, updir);

        // animation panning down
        if (self->rotation.y < 0.25)
            self->rotation.y += 0.04;
        else self->rotation.y = 0.25;
    }

    if (gfc_input_command_down("moveright")) {
        gfc_vector2d_negate(rigdir, rigdir);
        if (!check_movebounds(self, gfc_vector3d(rigdir.x, rigdir.y, 0), data))
            rigdir = gfc_vector2d(0, 0);

        gfc_vector2d_add(self->position, self->position, rigdir);

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
        if (!check_movebounds(self, gfc_vector3d(rigdir.x, rigdir.y, 0), data))
            rigdir = gfc_vector2d(0, 0);

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

    if (self->rotation.x < 0 && self->rotation.z < 0 && !gfc_input_command_down("moveright")) {
        self->rotation.x += 0.01;
        self->rotation.z += 0.01;
    }
    if (self->rotation.x > 0 && self->rotation.z > 0 && !gfc_input_command_down("moveleft")) {
        self->rotation.x -= 0.01;
        self->rotation.z -= 0.01;
    }

    // fix for offsetting due to model Z rotation
    if (self->position.y != data->og_pos.y)
        self->position.y = data->og_pos.y;

    // resets barrel roll rotation
    if (self->rotation.x < -5 || self->rotation.x > 5) self->rotation.x = 0;


    // barrel_roll checks
    if (gfc_input_command_down("movedown") && 
        gfc_input_command_released("roll")
        ){
            data->mid_roll = 1;
            data->roll = DOWN;
    }
    if (gfc_input_command_down("moveright") &&
        gfc_input_command_released("roll")
        ){
        data->mid_roll = 1;
        data->roll = RIGHT;
    }
    if (gfc_input_command_down("moveup") &&
        gfc_input_command_released("roll")
        ){
        data->mid_roll = 1;
        data->roll = UP;
    }
    if (gfc_input_command_down("moveleft") &&
        gfc_input_command_released("roll")
        ) {
        data->mid_roll = 1;
        data->roll = LEFT;
    }
}

void player_cam(Entity* self, PlayerData* data) {
    GFC_Vector3D lookTarget, camera = { 0 };

    if (!self) return;
    if (!data) return;

    // camera_view
    if (!(data->freelook)) {
        gfc_vector3d_copy(lookTarget, gfc_vector3d(0, self->position.y, 0));
        camera = gfc_vector3d(0, self->position.y + 90, 0);
        gf3d_camera_look_at(lookTarget, &camera);
        //gf3d_camera_set_position(gfc_vector3d(0, self->position.y + 90, 3));
    }
    else slog("Free Look Enabled");
}

void barrel_roll(Entity* self, PlayerData* data){
    if (!data) return;
    if (!self) return;

    GFC_Vector3D updir = { 0, 0, data->upspeed * 3};
    GFC_Vector2D rigdir = { data->rigspeed*5, 0 };

    gfc_vector3d_rotate_about_x(&updir, self->rotation.x);
    rigdir = gfc_vector2d_rotate(rigdir, self->rotation.z);

    if (data->roll == DOWN) {
        gfc_vector3d_negate(updir, updir);
        if (!check_movebounds(self, updir, data))
            updir = gfc_vector3d(0, 0, 0);

        gfc_vector3d_add(self->position, self->position, updir);

        // animation
        data->upspeed -= 0.1;
       
        if (data->upspeed < 0) {
            data->mid_roll = 0;
            data->upspeed = 1.2;
        }
    }
    else if (data->roll == RIGHT) {
        gfc_vector2d_negate(rigdir, rigdir);
        if (!check_movebounds(self, gfc_vector3d(rigdir.x, rigdir.y, 0), data))
            rigdir = gfc_vector2d(0, 0);

        gfc_vector2d_add(self->position, self->position, rigdir);

        data->rigspeed -= 0.1;

        // animation
        self->rotation.x -= 0.55;

        if (data->rigspeed < 0) {
            data->mid_roll = 0;
            data->rigspeed = 1.2;
        }
    }
    else if (data->roll == UP) {
        if (!check_movebounds(self, updir, data))
            updir = gfc_vector3d(0, 0, 0);

        gfc_vector3d_add(self->position, self->position, updir);

        // animation
        data->upspeed -= 0.1;

        if (data->upspeed < 0) {
            data->mid_roll = 0;
            data->upspeed = 1.2;
        }
    }
    else if (data->roll == LEFT) {
        if (!check_movebounds(self, gfc_vector3d(rigdir.x, rigdir.y, 0), data))
            rigdir = gfc_vector2d(0, 0);

        gfc_vector2d_add(self->position, self->position, rigdir);

        data->rigspeed -= 0.1;

        // animation
        self->rotation.x += 0.55;

        if (data->rigspeed < 0) {
            data->mid_roll = 0;
            data->rigspeed = 1.2;
        }
    }
}

int check_movebounds(Entity* self, GFC_Vector3D movement, PlayerData* data) {
    if (!self) return;
    if (!data) return;

    // if outside camera view
    if (self->position.x + movement.x >= data->x_bound ||
        self->position.x + movement.x <= -data->x_bound ||
        self->position.z + movement.z >= data->z_bound ||
        self->position.z + movement.z <= -data->z_bound
        )
        return 0;

    return 1;
}

/*
void mousepos_to_gamepos(GFC_Vector2D* cursor_pos, PlayerData* data) {
    GFC_Vector2D res = gf3d_vgraphics_get_resolution();

    cursor_pos->x = -(cursor_pos->x / (res.x / (2*data->x_bound)));
    cursor_pos->x += 46.5;

    if (cursor_pos->x > data->x_bound) cursor_pos->x = data->x_bound;
    if (cursor_pos->x < -data->x_bound) cursor_pos->x = -data->x_bound;
  
    cursor_pos->y = -(cursor_pos->y / (res.y / (2*data->z_bound)));
    cursor_pos->y += 32.5;

    if (cursor_pos->y > data->z_bound) cursor_pos->y = data->z_bound;
    if (cursor_pos->y < -data->z_bound) cursor_pos->y = -data->z_bound;
}
*/