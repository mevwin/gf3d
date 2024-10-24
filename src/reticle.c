#include "gf2d_mouse.h"
#include "simple_logger.h"
#include "reticle.h"
#include "enemy.h"
#include "player.h"
#include "projectile.h"

Entity* reticle_spawn(GFC_Vector3D position, void* player_data){
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
    data->z_bound = 58; // 172 x 116

    data->player_data = player_data;


    self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                            self->position.y - (self->model->bounds.h / 2),
                            self->position.z - (self->model->bounds.d / 2),
                            self->model->bounds.w,
                            self->model->bounds.h,
                            self->model->bounds.d);

    return self;
}

void reticle_update(Entity* self) {
    GFC_Vector2D cursor;
    ReticleData* data;
    Entity* entityList, *target;
    EnemyData* enemy_data;
    PlayerData* player_data;
    int i;

    data = self->data;
    if (!data) return;

    player_data = (PlayerData*) data->player_data;
    if (player_data->in_shop || player_data->paused || player_data->player_dead) return;

    // updating reticle position
    cursor = gfc_2DPos_to_3DPos(gf2d_mouse_get_position(), data->x_bound, data->z_bound);
    self->position.x = cursor.x;
    self->position.z = cursor.y;

    // only check reticle targeting if in missile mode
    if (player_data->curr_mode == MISSILE) {
        entityList = get_entityList();
        for (i = 0; i < MAX_ENTITY; i++) {
            target = &entityList[i];

            if (target->entity_type != ENEMY)
                continue;

            // collision detection check for missile attack
            if (gfc_box_overlap(self->hurtbox, target->hurtbox)) {
                enemy_data = target->data;
                
                // only activate target untargeted, alive enemies
                if (!enemy_data->missile_targeted && enemy_data->currHealth > 0.0) {
                    data->locked_on = 1;
                    data->enemy_pos = &(target->position);
                    enemy_data->missile_targeted = 1;
                    break;
                }
                else 
                    data->locked_on = 0;
            }
        }
    }

    // update hurtbox
    self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                            self->position.y - (self->model->bounds.h / 2),
                            self->position.z - (self->model->bounds.d / 2),
                            self->model->bounds.w,
                            self->model->bounds.h,
                            self->model->bounds.d);

    // keep reticle within camera
    if (self->position.x >= data->x_bound - 1.0)
        self->position.x = data->x_bound - 2.0;
    if (self->position.x <= -data->x_bound + 1.0)
        self->position.x = -data->x_bound + 2.0;
    if (self->position.z >= data->z_bound - 1.0)
        self->position.z = data->z_bound - 2.0;
    if (self->position.z <= -data->z_bound + 1.0)
        self->position.z = -data->z_bound + 2.0;
}

void reticle_free(Entity* self) {
    ReticleData* data;

    if (!self) return;

    data = (ReticleData*) self->data;
    free(data);
    self->data = NULL;
}