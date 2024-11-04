#include "simple_logger.h"
#include "gf3d_camera.h"
#include "gf3d_vgraphics.h"
#include "gf2d_mouse.h"
#include "player_move.h"
#include "projectile.h"

#define NO_MOVEMENT3D (gfc_vector3d(0, 0, 0))
#define NO_MOVEMENT2D (gfc_vector2d(0, 0))

void player_movement(Entity* self, PlayerData* data) {
    GFC_Vector3D updir;
    GFC_Vector2D rigdir;

    if (!data) return;
    if (!self) return;

    updir = gfc_vector3d(0, 0, data->upspeed);
    rigdir = gfc_vector2d(data->rigspeed, 0);

    if (gfc_input_command_down("moveup") && !gfc_input_command_down("movedown")) {
        if (!check_movebounds(self, updir, data))
            updir = NO_MOVEMENT3D;

        gfc_vector3d_add(self->position, self->position, updir);

        // animation panning up
        if (self->rotation.y > -0.25f)
            self->rotation.y -= 0.03f;
        else self->rotation.y = -0.25f;
    }

    if (gfc_input_command_down("movedown") && !gfc_input_command_down("moveup")) {
        gfc_vector3d_negate(updir, updir);
        if (!check_movebounds(self, updir, data))
            updir = NO_MOVEMENT3D;

        gfc_vector3d_add(self->position, self->position, updir);

        // animation panning down
        if (self->rotation.y < 0.25f)
            self->rotation.y += 0.04f;
        else self->rotation.y = 0.25f;
    }

    if (gfc_input_command_down("moveright") && !gfc_input_command_down("moveleft")) {
        gfc_vector2d_negate(rigdir, rigdir);
        if (!check_movebounds(self, gfc_vector3d(rigdir.x, rigdir.y, 0), data))
            rigdir = NO_MOVEMENT2D;

        gfc_vector2d_add(self->position, self->position, rigdir);

        // animation panning right
        if (self->rotation.x > -0.25f && self->rotation.z > -0.25f) {
            self->rotation.x -= 0.04f;
            self->rotation.z -= 0.04f;
        }
        else {
            self->rotation.x = -0.25f;
            self->rotation.z = -0.25f;
        }
    }

    if (gfc_input_command_down("moveleft") && !gfc_input_command_down("moveright")) {
        if (!check_movebounds(self, gfc_vector3d(rigdir.x, rigdir.y, 0), data))
            rigdir = NO_MOVEMENT2D;

        gfc_vector2d_add(self->position, self->position, rigdir);

        // animation panning left
        if (self->rotation.x < 0.25f && self->rotation.z < 0.25f) {
            self->rotation.x += 0.04f;
            self->rotation.z += 0.04f;
        }

        else {
            self->rotation.x = 0.25f;
            self->rotation.z = 0.25f;
        }
    }

    // undoes rotation on ship when player isn't pressing a button
    if (self->rotation.y < 0 && !gfc_input_command_down("moveup"))
        self->rotation.y += 0.03f;

    if (self->rotation.y > 0 && !gfc_input_command_down("movedown"))
        self->rotation.y -= 0.03f;

    if (self->rotation.x < 0 && self->rotation.z < 0 && !gfc_input_command_down("moveright")) {
        self->rotation.x += 0.03f;
        self->rotation.z += 0.03f;
    }
    if (self->rotation.x > 0 && self->rotation.z > 0 && !gfc_input_command_down("moveleft")) {
        self->rotation.x -= 0.03f;
        self->rotation.z -= 0.03f;
    }

    // fix for offsetting due to model Z rotation
    if (self->position.y != data->og_pos.y)
        self->position.y = data->og_pos.y;

    // resets barrel roll rotation
    if (self->rotation.x < -5.0f || self->rotation.x > 5.0f) 
        self->rotation.x = 0;

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
    GFC_Vector3D lookTarget, camera;

    if (!self) return;
    if (!data) return;

    // camera_view
    if (!(data->freelook)) {
        gfc_vector3d_copy(lookTarget, gfc_vector3d(0, self->position.y, 0));
        camera = gfc_vector3d(0, self->position.y + 90.0f, 0);
        gf3d_camera_look_at(lookTarget, &camera);
    }
    else slog("Free Look Enabled");
}

void barrel_roll(Entity* self, PlayerData* data){
    GFC_Vector3D updir;
    GFC_Vector2D rigdir;
    if (!data) return;
    if (!self) return;

    updir = gfc_vector3d(0, 0, data->upspeed * 3.0f);
    rigdir = gfc_vector2d(data->rigspeed * 5.0f, 0);

    if (data->roll == DOWN) {
        gfc_vector3d_negate(updir, updir);
        if (!check_movebounds(self, updir, data))
            updir = NO_MOVEMENT3D;

        gfc_vector3d_add(self->position, self->position, updir);

        // animation
        data->upspeed -= 0.1f;
       
        if (data->upspeed < 0) {
            data->mid_roll = 0;
            data->upspeed = 1.2f;
        }
    }
    else if (data->roll == RIGHT) {
        gfc_vector2d_negate(rigdir, rigdir);
        if (!check_movebounds(self, gfc_vector3d(rigdir.x, rigdir.y, 0), data))
            rigdir = NO_MOVEMENT2D;

        gfc_vector2d_add(self->position, self->position, rigdir);

        data->rigspeed -= 0.1f;

        // animation
        self->rotation.x -= 0.55f;

        if (data->rigspeed < 0) {
            data->mid_roll = 0;
            data->rigspeed = 1.2f;
        }
    }
    else if (data->roll == UP) {
        if (!check_movebounds(self, updir, data))
            updir = NO_MOVEMENT3D;

        gfc_vector3d_add(self->position, self->position, updir);

        // animation
        data->upspeed -= 0.1f;

        if (data->upspeed < 0) {
            data->mid_roll = 0;
            data->upspeed = 1.2f;
        }
    }
    else if (data->roll == LEFT) {
        if (!check_movebounds(self, gfc_vector3d(rigdir.x, rigdir.y, 0), data))
            rigdir = NO_MOVEMENT2D;

        gfc_vector2d_add(self->position, self->position, rigdir);

        data->rigspeed -= 0.1f;

        // animation
        self->rotation.x += 0.55f;

        if (data->rigspeed < 0) {
            data->mid_roll = 0;
            data->rigspeed = 1.2f;
        }
    }
}

Uint8 check_movebounds(Entity* self, GFC_Vector3D movement, PlayerData* data) {
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