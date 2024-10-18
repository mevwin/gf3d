#include "gf2d_mouse.h"
#include "gf3d_vgraphics.h"
#include "simple_logger.h"
#include "reticle.h"

GFC_Vector3D* reticle_spawn(GFC_Vector3D position) {
    Entity* self;
    ReticleData* data;

    self = entity_new();
    if (!self) return NULL;

    self->model = gf3d_model_load("models/reticle/reticle.model");
    self->update = reticle_update;

    self->position = position;
    self->free = reticle_free;
    self->entity_type = RETICLE;

    data = gfc_allocate_array(sizeof(ReticleData), 1);
    if (data) self->data = data;

    data->x_bound = 74; // left is positive, right is negative
    data->y_bound = -60;
    data->z_bound = 50; 

    return &(self->position);
}

void reticle_update(Entity* self) {
    ReticleData* data;

    data = self->data;
    if (!data) return;

    GFC_Vector2D res = gf3d_vgraphics_get_resolution();
    GFC_Vector2D cursor = gf2d_mouse_get_position();
    cursor.x -= res.x/2;
    cursor.y -= res.y/2;

    self->position.x = cursor.x / (-res.x / (2 * data->x_bound));
    self->position.z = cursor.y / (-res.y / (2 * data->z_bound));

    //slog("CursorX: %f, CursorY: %f", cursor.x, cursor.y);

    // keep reticle within camera
    if (self->position.x >= data->x_bound - 1)
        self->position.x = data->x_bound - 2;
    if (self->position.x <= -data->x_bound + 1)
        self->position.x = -data->x_bound + 2;
    if (self->position.z >= data->z_bound - 1)
        self->position.z = data->z_bound - 2;
    if (self->position.z <= -data->z_bound + 1)
        self->position.z = -data->z_bound + 2;
}

void reticle_free(Entity* self) {
    ReticleData* data;

    if (!self) return;

    data = (ReticleData*)self->data;
    free(data);
    self->data = NULL;
}

int check_recbounds(Entity* self, GFC_Vector3D movement, ReticleData* data) {
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