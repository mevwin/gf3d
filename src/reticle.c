#include "gf2d_mouse.h"
#include "gf3d_vgraphics.h"
#include "simple_logger.h"
#include "reticle.h"

Entity* reticle_spawn(GFC_Vector3D position) {
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

    data->x_bound = 86; // left is positive, right is negative
    data->y_bound = -60;
    data->z_bound = 58; 

    self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                            self->position.y - (self->model->bounds.h / 2),
                            self->position.z - (self->model->bounds.d / 2),
                            self->model->bounds.w,
                            self->model->bounds.h,
                            self->model->bounds.d);

    return self;
}

void reticle_update(Entity* self) {
    ReticleData* data;
    Entity* entityList, *target;
    int i;

    data = self->data;
    if (!data) return;

    GFC_Vector2D res = gf3d_vgraphics_get_resolution();
    GFC_Vector2D cursor = gf2d_mouse_get_position();
    cursor.x -= res.x/2;
    cursor.y -= res.y/2;

    self->position.x = cursor.x / (-res.x / (2 * data->x_bound));
    self->position.z = cursor.y / (-res.y / (2 * data->z_bound));

    //slog("CursorX: %f, CursorY: %f", cursor.x, cursor.y);

    entityList = get_entityList();
    for (i = 0; i < MAX_ENTITY; i++) {
        target = &entityList[i];

        if (target->entity_type != ENEMY)
            continue;

        // collision detection check
        if (gfc_box_overlap(self->hurtbox, target->hurtbox)) {
            data->locked_on = 1;
            slog("locked in");
            break;
        }
        data->locked_on = 0;
    }

    // update hurtbox
    self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
        self->position.y - (self->model->bounds.h / 2),
        self->position.z - (self->model->bounds.d / 2),
        self->model->bounds.w,
        self->model->bounds.h,
        self->model->bounds.d);

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

    data = (ReticleData*) self->data;
    free(data);
    self->data = NULL;
}

Uint8 check_recbounds(Entity* self, GFC_Vector3D movement, ReticleData* data) {
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