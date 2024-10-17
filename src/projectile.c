#include "simple_logger.h"
#include "projectile.h"
#include "player.h"

int proj_count = 0;
int wave_count = 0;
float next_shot_time = 0;

void player_proj_spawn(GFC_Vector3D position, GFC_Vector3D reticle_pos, Entity* player, float curr_time) {
    Entity* self, *play;
    ProjData* data;
    PlayerData* playdata;
    float dist_x, dist_y, conver, z_angle, y_angle;

    self = entity_new();
    if (!self) return NULL;

    if (proj_count == MAX_PROJ) {
        entity_free(self);
        return;
    }

    data = gfc_allocate_array(sizeof(ProjData), 1);
    if (data) self->data = data;

    data->owner = player;
    play = data->owner;
    playdata = play->data;

    if (playdata->curr_mode == WAVE_SHOT && wave_count == MAX_WAVE) {
        entity_free(self);
        return;
    }

    self->update = proj_update;
    self->entity_type = PROJECTILE;
    self->position = position;
    self->free = proj_free;

    data->type = playdata->curr_mode;
    data->reticle_pos = reticle_pos;
    data->y_bound = -180.0;

    // rotating to reticle;
    dist_x = data->reticle_pos.x - position.x;
    dist_y = data->reticle_pos.z - position.z;    

    z_angle = atan(dist_x / data->reticle_pos.y);
    self->rotation.z -= z_angle;

    y_angle = atan(dist_y / data->reticle_pos.y);
    self->rotation.y += y_angle;

    if (data->type == SINGLE_SHOT || data->type == CHARGE_SHOT) {
        self->think = proj_think_basic;
        self->model = data->type == SINGLE_SHOT ? gf3d_model_load("models/projectiles/single_shot.model") : gf3d_model_load("models/projectiles/charge_shot.model");
        data->forspeed = data->type == SINGLE_SHOT ? 9 : 11;
        conver = data->reticle_pos.y / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);
    }
    if (data->type == WAVE_SHOT) {
        self->think = proj_think_wave_shot;
        self->model = gf3d_model_load("models/projectiles/wave_shot.model");
        data->forspeed = 4;
        wave_count++;
    }

    //slog("Rig: %f | Up: %f", data->rigspeed, data->upspeed);

    proj_count++;

    return self;
}

void proj_update(Entity* self) {

}

void proj_free(Entity* self) {
    ProjData* data;

    if (!self) return;

    data = (ProjData*) self->data;
    free(data);
    self->data = NULL;

    proj_count--;
}

int	proj_exist(Entity* self, ProjData* data) {
    if (!data) return;
    
    if (data->y_bound >= self->position.y)
        return 0;

    return 1;
}

void proj_think_basic(Entity* self) {
    ProjData* data;

    data = self->data;
    if (!data) return;

    self->position.x -= data->rigspeed;
    self->position.y -= data->forspeed;
    self->position.z -= data->upspeed;

    if (!proj_exist(self, self->data))
        entity_free(self);
}

void proj_think_missile(Entity* self) {

}
void proj_think_wave_shot(Entity* self) {
    ProjData* data;
    Entity* player;
    PlayerData* playdata;

    data = self->data;
    if (!data) return;

    player = data->owner;
    playdata = player->data;
    self->position.y -= data->forspeed;

    if (!proj_exist(self, self->data)) {
        playdata->curr_mode = SINGLE_SHOT;
        wave_count--;
        entity_free(self);
    }
        
}
void proj_think_super_nuke(Entity* self) {

}