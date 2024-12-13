#include "gf2d_mouse.h"
#include "simple_logger.h"
#include "reticle.h"
#include "enemy.h"
#include "player.h"
#include "projectile.h"
#include "level.h"

void reticle_update(Entity* self);
void reticle_free(Entity* self);

static Entity* self;

Entity* reticle_spawn(GFC_Vector3D position){
    ReticleData* data;

    self = entity_new();
    if (!self) return NULL;

    self->model = get_models()->reticle;
    self->update = reticle_update;

    self->position = position;
    self->free = reticle_free;
    self->entity_type = RETICLE;

    data = gfc_allocate_array(sizeof(ReticleData), 1);
    if (data) self->data = data;

    data->x_bound = 86; // left is positive, right is negative
    data->y_bound = -60;
    data->z_bound = 58; // 172 x 116

    update_hurtbox(self);

    return self;
}

void reticle_update(Entity* self) {
    GFC_Vector2D cursor;
    ReticleData* data;
    Entity* entityList, *target;
    EnemyData* enemy_data;
    PlayerData* player_data;
    LevelData* level;
    int i;

    data = self->data;
    if (!data) return;

    player_data = get_player_data();
    level = get_level_data();

    // updating reticle position
    cursor = gfc_2DPos_to_3DPos(gf2d_mouse_get_position(), data->x_bound, data->z_bound);
    self->position.x = cursor.x;
    self->position.z = cursor.y;

    // update hurtbox
    update_hurtbox(self);

    // only check reticle targeting if in missile mode
    if (player_data->currMode == MISSILE) {
        entityList = get_entityList();
        for (i = 0; i < MAX_ENTITY; i++) {
            target = &entityList[i];

            if (target->entity_type != ENEMY)
                continue;

            // collision detection check for missile attack
            if (gfc_box_overlap(self->hurtbox.s.b, target->hurtbox.s.b)) {
                enemy_data = target->data;
                
                // only activate target untargeted, alive enemies
                if (!enemy_data->missile_targeted && enemy_data->currHealth > 0) {
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

    // keep reticle within camera
    if (self->position.x >= data->x_bound - 1)
        self->position.x = data->x_bound - 2.0f;
    if (self->position.x <= -data->x_bound + 1)
        self->position.x = -data->x_bound + 2.0f;
    if (self->position.z >= data->z_bound - 1)
        self->position.z = data->z_bound - 2.0f;
    if (self->position.z <= -data->z_bound + 1)
        self->position.z = -data->z_bound + 2.0f;
}

void reticle_free(Entity* self) {
    ReticleData* data;

    if (!self) return;

    data = (ReticleData*) self->data;
    free(data);
}

GFC_Vector3D get_reticle_pos() {
    return self->position;
}