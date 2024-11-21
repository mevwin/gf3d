#include "simple_logger.h"
#include "gfc_audio.h"
#include "player.h"
#include "enemy.h"
#include "item.h"
#include "level.h"

#define ITEM_Z_OFFSET 15.0f
#define POWERUP_DURATION 25.0f
#define SCRAP_SPEED 1.0f
#define POWERUP_SPEED 2.0f

void item_spawn(int type, GFC_Vector3D spawn_pos, float dist_to_player) {
    Entity* self;
    ItemData* data;
    LevelData* level;
    float dist_x, dist_y, conver, time;
    Item_Type item_t;

    item_t = (Item_Type) type;

    // only allow powerups within the past 20 seconds
    level = get_level_data();
    time = CURRENT_TIME;
    if (time < level->last_powerup && (item_t == HAPPY_TRIGGER || item_t == INVINCIBILITY))
        item_t = NONE;

    if (item_t == NONE) return;

    // sanity check
    self = entity_new();
    if (!self) return;

    self->position = spawn_pos;
    self->think = item_think;
    self->update = item_update;
    self->free = item_free;
    self->entity_type = ITEM;

    data = gfc_allocate_array(sizeof(ItemData), 1);
    if (data) self->data = data;

    if (type == SCRAP) {
        self->model = get_models()->scrap;
        data->forspeed = SCRAP_SPEED;
    }
    else if (type == HEALTH_PICKUP) {
        self->position.z += ITEM_Z_OFFSET;
        self->model = get_models()->health_pickup;
        data->forspeed = POWERUP_SPEED;
    }
    else if (type == HAPPY_TRIGGER) {
        self->position.z += ITEM_Z_OFFSET;
        self->model = get_models()->happy_trigger;
        data->forspeed = POWERUP_SPEED;
        level->last_powerup = time + POWERUP_DURATION;
    }
    else if (type == INVINCIBILITY) {
        self->position.z += ITEM_Z_OFFSET;
        self->model = get_models()->invincibility;
        data->forspeed = POWERUP_SPEED;
        level->last_powerup = time + POWERUP_DURATION;
    }

    data->type = type;
    data->dist_to_player = dist_to_player;

    update_hurtbox(self);

    dist_x = get_player_data()->player_pos->x - self->position.x;
    dist_y = get_player_data()->player_pos->z - self->position.z;

    conver = dist_to_player / data->forspeed;
    data->rigspeed = (dist_x / conver);
    data->upspeed = (dist_y / conver);
    data->active = 1;
}

void item_think(Entity* self) {
    ItemData* data; 
    LevelData* level;

    data = self->data;
    if (!data) return;

    level = get_level_data();

    if (level->wave_end) 
        item_activate(self, data->type);

    if (!data->active) return;

    self->position.x -= data->rigspeed;
    self->position.y += data->forspeed;
    self->position.z -= data->upspeed;

    // checks if item hit player (use spheres)
    if (self->position.y > -30.0f && gfc_sphere_overlap(self->hurtbox.s.s, self->hurtbox.s.s) && data->active) {
        item_activate(self, data->type);
        data->active = 0;
    }
}

void item_update(Entity* self) {
    ItemData* data;
    PlayerData* player_data;
    LevelData* level;
    float dist_x, dist_y, conver;

    data = self->data;
    if (!data) return;

    player_data = get_player_data();
    level = get_level_data();

    if (!data->active) return;

    if (self->position.y > 90.0f || player_data->player_dead)
        entity_free(self);

    // updates movement
    dist_x = player_data->player_pos->x - self->position.x;
    dist_y = player_data->player_pos->z - self->position.z;

    conver = self->position.y / data->forspeed;
    data->rigspeed = (dist_x / conver);
    data->upspeed = (dist_y / conver);

    // updates hurtbox
    update_hurtbox(self);
}

void item_activate(Entity* self, int type) {
    PlayerData* player;
    int extra_amount;
    float time;

    player = get_player_data();
    time = CURRENT_TIME;

    if (type == SCRAP) {
        extra_amount = 1 + gfc_random_int(3);
        if ((player->currScrap + extra_amount) < player->maxScrap)
            player->currScrap += extra_amount;
        else
            player->currScrap = player->maxScrap;
    }
    else if (type == HEALTH_PICKUP) {
        extra_amount = 100 * (1 + gfc_random_int(3));
        if ((player->currHealth + extra_amount) < player->maxHealth)
            player->currHealth += (float) extra_amount;
        else
            player->currHealth = player->maxHealth;
    }
    else if (type == HAPPY_TRIGGER) {
        player->active_item = HAPPY_TRIGGER;
        player->item_duration = time + 5.0f;
    }
    else if (type == INVINCIBILITY) {
        player->active_item = INVINCIBILITY;
        player->item_duration = time + 10.0f;
    }

    if (type != SCRAP)
        gfc_sound_play(get_sound_data()->item_pickup, 0, 0.5f, -1, -1);
 
    entity_free(self);
}

void item_free(Entity* self) {
    ItemData* data;

    data = (ItemData*) self->data;
    free(data);
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