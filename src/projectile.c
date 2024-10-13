#include "simple_logger.h"
#include "projectile.h"

int proj_count = 0;
float next_shot_time = 0;

Entity* proj_spawn(GFC_Vector3D position, int curr_mode, float curr_time) {
    Entity* self;
    Entity* entity_list;
    ProjData* data;
    int i;

    self = entity_new();
    if (!self) return NULL;

    if (proj_count == MAX_PROJ) return NULL;

    data = gfc_allocate_array(sizeof(ProjData), 1);
    if (data) self->data = data;

    self->update = proj_update;
    self->entity_type = PROJECTILE;
    self->position = position;
    self->free = proj_free;

    data->curr_mode = curr_mode;

    if (curr_mode == SINGLE_SHOT) {
        self->think = proj_think_basic;
        self->model = gf3d_model_load("models/projectiles/single_shot.model");
        data->forspeed = 15;
    }
    if (curr_mode == CHARGE_SHOT) {
        self->think = proj_think_basic;
        self->model = gf3d_model_load("models/projectiles/charge_shot.model");
        data->forspeed = 20;
    }

    //data->forspeed = 15;
    data->upspeed = 2;
    data->rigspeed = 2;

    data->y_bound = -180;
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

    self->position.y -= data->forspeed;

    if (!proj_exist(self, self->data))
        entity_free(self);

}

void proj_think_missile(Entity* self) {

}
void proj_think_spread_shot(Entity* self) {

}
void proj_think_super_nuke(Entity* self) {

}