#include "simple_logger.h"
#include "player.h"
#include "enemy.h"
#include "item.h"

/**
* define how an item will be interacted with
* maybe make some global function that can get the item by volume
* make states that define the behavior of the item at a certain point
* item pickups should funciton like enemy projectiles, except they're homing
* two bounding boxes for player, one for enemy targets, one for item pickups
*/

Entity* item_spawn(int type, GFC_Vector3D spawn_pos, void* enemy_data) {
    Entity* self;
    ItemData* data;
    EnemyData* enemy;
    float dist_x, dist_y, conver, dist_to_player;

    // sanity check
    self = entity_new();
    if (!self) return NULL;

    self->position = spawn_pos;
    self->think = item_think;
    self->update = item_update;
    self->free = item_free;
    self->entity_type = ITEM;

    data = gfc_allocate_array(sizeof(ItemData), 1);
    if (data) self->data = data;

    enemy = (EnemyData*) enemy_data;
    data->enemy_data = enemy;

    if (type == NONE) {
        enemy->item_taken = 1;
        entity_free(self);
        return;
    }

    data->type = type;
    dist_to_player = (float) enemy->dist_to_player;

    self->model = gf3d_model_load("models/scrap/enemy_scrap.model");

    data->forspeed = 1.0;
    self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                            self->position.y - (self->model->bounds.h / 2),
                            self->position.z - (self->model->bounds.d / 2),
                            self->model->bounds.w,
                            self->model->bounds.h,
                            self->model->bounds.d);

    dist_x = enemy->player_pos->x - self->position.x;
    dist_y = enemy->player_pos->z - self->position.z;

    conver = dist_to_player / data->forspeed;
    data->rigspeed = (dist_x / conver);
    data->upspeed = (dist_y / conver);

    enemy->item_made = 1;
    enemy->item_taken = 0;

    return self;
}

void item_think(Entity* self) {
    ItemData* data;
    EnemyData* enemy_data;
    PlayerData* player_data;
    Entity* player, *entityList;
    int i;

    data = self->data;
    if (!data) return;

    enemy_data = (EnemyData*)data->enemy_data;
    player_data = (PlayerData*)enemy_data->player_data;

    if (player_data->in_shop || player_data->paused) return;

    self->position.x -= data->rigspeed;
    self->position.y += data->forspeed;
    self->position.z -= data->upspeed;

    // checks if item hit player
    if (self->position.y > -40){
        entityList = get_entityList();
        for (i = 0; i < MAX_ENTITY; i++) {
            player = &entityList[i];

            if (player->entity_type != PLAYER)
                continue;

            // collision detection check
            if (gfc_box_overlap(self->hurtbox, player->hurtbox)) {
                enemy_data->item_taken = 1;
                item_activate(self, data->type, player->data);
                break;
            }
        }
    }
}
void item_update(Entity* self) {
    ItemData* data;
    EnemyData* enemy_data;
    PlayerData* player_data;
    float dist_x, dist_y, conver;

    data = self->data;
    if (!data) return;

    enemy_data = (EnemyData*) data->enemy_data;
    player_data = (PlayerData*) enemy_data->player_data;

    if (player_data->in_shop || player_data->paused) return;

    // updates movement
    dist_x = enemy_data->player_pos->x - self->position.x;
    dist_y = enemy_data->player_pos->z - self->position.z;

    conver = self->position.y / data->forspeed;
    data->rigspeed = (dist_x / conver);
    data->upspeed = (dist_y / conver);

    // updates hurtbox
    self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                            self->position.y - (self->model->bounds.h / 2),
                            self->position.z - (self->model->bounds.d / 2),
                            self->model->bounds.w,
                            self->model->bounds.h,
                            self->model->bounds.d);


    if (self->position.y > 90.0 || player_data->player_dead ) {
        enemy_data->item_taken = 1;
        entity_free(self);
    }
}

void item_activate(Entity* self, int type, void* player_data) {
    PlayerData* player;
    int extra_amount;

    player = (PlayerData*) player_data;

    if (type == SCRAP) {
        extra_amount = 1 + gfc_random_int(3);
        if ((player->currScrap + extra_amount) <= player->maxScrap)
            player->currScrap += extra_amount;
        else
            player->currScrap = player->maxScrap;
    }
    else if (type == HEALTH_PICKUP) {
        extra_amount = 100 * (1 + gfc_random_int(3));
        if ((player->currHealth + extra_amount) <= player->maxHealth)
            player->currHealth += extra_amount;
        else
            player->currHealth = player->maxHealth;
    }
    entity_free(self);
}

void item_free(Entity* self) {
    ItemData* data;

    data = (ItemData*) self->data;
    free(data);
    self->data = NULL;
}




/**
* draw window, gf2d_window.h
*	draws front to back, updates back to front
* bunch of images/sprites layered on top of each other
* health bar, two bars layered on top of each other
* text, uss gf2d_font.h
* element_actor.h, for sprites
* look at menus/yes_no_window.json
*/