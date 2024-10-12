#include "simple_logger.h"
#include "projectile.h"

int proj_count = 0;

Entity* proj_spawn(GFC_Vector3D position) {
    Entity* self;
    Entity* entity_list;
    ProjData* data;
    int i;

    self = entity_new();
    if (!self) return NULL;

    if (proj_count == MAX_PROJ) return NULL;

    self->model = gf3d_model_load("models/projectile.model");
    self->think = proj_think;
    self->update = proj_update;

    self->position = position;
    self->free = proj_free;
    self->entity_type = PROJECTILE;

    data = gfc_allocate_array(sizeof(ProjData), 1);
    if (data) self->data = data;

    data->forspeed = 5;
    data->upspeed = 2;
    data->rigspeed = 2;

    data->y_bound = -90;
    proj_count++;
}

void proj_think(Entity* self) {
    ProjData* data;
    
    data = self->data;
    if (!data) return;

    //GFC_Vector2D fordir = { 0, data->forspeed };

    if (!proj_exist(self, self->data))
        entity_free(self);

    self->position.y -= data->forspeed;

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
    
    if (data->y_bound == self->position.y)
        return 0;

    return 1;
}