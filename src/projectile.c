#include "simple_logger.h"
#include "gf2d_mouse.h"
#include "projectile.h"
#include "player.h"
#include "enemy.h"

void player_proj_spawn(GFC_Vector3D position, GFC_Vector3D reticle_pos, Entity* player, float curr_time) {
    Entity* self, *play;
    ProjData* data;
    PlayerData* playdata;
    float dist_x, dist_y, conver, z_angle, y_angle;

    self = entity_new();
    if (!self) return NULL;

    data = gfc_allocate_array(sizeof(ProjData), 1);
    if (data) self->data = data;

    data->owner = player;
    play = data->owner;
    playdata = play->data;

    // make sure to enforce max projectile restrictions
    if ((playdata->curr_mode == WAVE_SHOT && playdata->wave_flag == MAX_WAVE) || 
        (playdata->curr_mode == MISSILE && playdata->proj_count == MAX_MISSILE) ||
        playdata->proj_count == MAX_PROJ
        ) {
        entity_free(self);
        free(data);
        return;
    }

    self->update = proj_update;
    self->entity_type = PROJECTILE;
    self->position = position;
    self->free = proj_free;

    data->type = playdata->curr_mode;
    data->y_bound = -170.0;

    // rotating projectile to reticle
    if (data->type == SINGLE_SHOT || data->type == CHARGE_SHOT || data->type == MISSILE) {
        dist_x = reticle_pos.x - position.x;
        dist_y = reticle_pos.z - position.z;

        z_angle = atan(dist_x / reticle_pos.y);
        self->rotation.z -= z_angle;

        y_angle = atan(dist_y / reticle_pos.y);
        self->rotation.y += y_angle;
    }

    // setup for a specific player attack type
    if (data->type == SINGLE_SHOT || data->type == CHARGE_SHOT) {
        self->think = proj_think_basic;
        self->model = data->type == SINGLE_SHOT ? gf3d_model_load("models/projectiles/single_shot.model") : gf3d_model_load("models/projectiles/charge_shot.model");
        data->forspeed = data->type == SINGLE_SHOT ? playdata->proj_speed : playdata->proj_speed * 1.25;
        data->damage = data->type == SINGLE_SHOT ? playdata->base_damage : playdata->base_damage*3;

        self->hurtbox = gfc_sphere(self->position.x, self->position.y, self->position.z, self->model->bounds.w / 2);
        conver = reticle_pos.y / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);
    }
    else if (data->type == WAVE_SHOT) {
        self->think = proj_think_wave_shot;
        self->model = gf3d_model_load("models/projectiles/wave_shot.model");
        data->forspeed = playdata->proj_speed / 2;
        playdata->wave_flag = 1;
    }
    else if (data->type == MISSILE) {
        self->think = proj_think_missile;
        self->model = gf3d_model_load("models/projectiles/single_shot.model");
        data->forspeed = playdata->proj_speed * 0.75;
        data->damage = playdata->base_damage * 5;

        self->hurtbox = gfc_sphere(self->position.x, self->position.y, self->position.z, self->model->bounds.w / 2);
        conver = reticle_pos.y / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);
    }
    
    //slog("Rig: %f | Up: %f", data->rigspeed, data->upspeed);

    playdata->proj_count++;

    return self;
}

void enemy_proj_spawn(GFC_Vector3D position, GFC_Vector3D player_pos, Entity* enemy) {
    Entity* self, *enem;
    ProjData* data;
    EnemyData* enemydata;
    float dist_x, dist_y, conver, z_angle, y_angle;

    self = entity_new();
    if (!self) return NULL;

    data = gfc_allocate_array(sizeof(ProjData), 1);
    if (data) self->data = data;

    data->owner = enemy;
    enem = data->owner;
    enemydata = enemy->data;

    // enforcing maximum projectile count per entity
    if (enemydata->proj_count == MAX_PROJ) {
        entity_free(self);
        free(data);
        return;
    }

    self->update = proj_update;
    self->entity_type = PROJECTILE;
    self->position = position;
    self->free = proj_free;

    data->type = enemydata->enemy_type;
    data->y_bound = 100;

    // rotating projectile to player
    if (data->type == PEAS || data->type == CHARGERS) {
        dist_x = player_pos.x - position.x;
        dist_y = player_pos.z - position.z;

        z_angle = atan(dist_x / enemydata->dist_to_player);
        self->rotation.z += z_angle;

        y_angle = atan(dist_y / enemydata->dist_to_player);
        self->rotation.y -= y_angle;
    }

    if (data->type == PEAS || data->type == CHARGE_SHOT) {
        self->think = proj_think_basic;
        self->model = data->type == PEAS ? gf3d_model_load("models/projectiles/single_shot.model") : gf3d_model_load("models/projectiles/charge_shot.model");
        data->forspeed = data->type == PEAS ? enemydata->pea_speed : enemydata->pea_speed * 1.25;
        data->damage = data->type == PEAS ? enemydata->base_damage : enemydata->base_damage * 3;

        self->hurtbox = gfc_sphere(self->position.x, self->position.y, self->position.z, 1);
        conver = enemydata->dist_to_player / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);
    }

    //slog("Rig: %f | Up: %f", data->rigspeed, data->upspeed);

    enemydata->proj_count++;

    return self;
}

void proj_update(Entity* self) {
    ProjData* data;
    PlayerData* playdata;
    EnemyData* enemydata;
    Entity* owner, *enemy, *entityList;

    int i, entity_type;

    if (!self) return;

    data = self->data;

    owner = data->owner;
    
    playdata = owner->data;
    
    entityList = get_entityList();

    // updates hurtbox;
    self->hurtbox = gfc_sphere(self->position.x, self->position.y, self->position.z, self->model->bounds.w / 2);

    for (i = 0; i < MAX_ENTITY; i++) {\
        enemy = &entityList[i];
        
        if (enemy->entity_type != ENEMY) continue;

        // collision detection check
        if (gfc_sphere_overlap(self->hurtbox, enemy->hurtbox)) {
            enemydata = enemy->data;
            enemydata->took_damage = 1;
            enemydata->damage_taken = data->damage;
            
            slog("Enemy Radius: %f", enemy->hurtbox.r);
            slog("Proj Radius: %f", self->hurtbox.r);
            entity_free(self);
            break;
        }
    }
}

void proj_free(Entity* self) {
    ProjData* data;
    PlayerData* playdata;
    PlayerData* enemydata;
    Entity* owner;

    if (!self) return;

    data = (ProjData*) self->data;

    owner = data->owner;
    
    if (owner->entity_type == PLAYER) {
        playdata = owner->data;
        playdata->proj_count--;
    }
    else if (owner->entity_type == ENEMY) {
        enemydata = owner->data;
        enemydata->proj_count--;
    }

    free(data);
    self->data = NULL;

}

Uint8 proj_exist(Entity* self, ProjData* data) {
    if (!data) return;
    
    if (data->y_bound >= self->position.y)
        return 0;

    return 1;
}

void proj_think_basic(Entity* self) {
    ProjData* data;

    data = self->data;
    if (!data) return;

    if (data->owner->entity_type == PLAYER) {
        self->position.x -= data->rigspeed;
        self->position.y -= data->forspeed;
        self->position.z -= data->upspeed;
    }
    else if (data->owner->entity_type == ENEMY) {
        self->position.x += data->rigspeed;
        self->position.y += data->forspeed;
        self->position.z += data->upspeed;
    }

    if (!proj_exist(self, self->data))
        entity_free(self);
}

void proj_think_missile(Entity* self) {
    ProjData* data;
    PlayerData* playdata;
    Entity* owner;

    data = self->data;
    if (!data) return;

    owner = data->owner;
    playdata = owner->data;

    if ((gf2d_mouse_button_held(0) || gf2d_mouse_button_pressed(0)) && 
        self->position.y >= data->spawn_pos.y - 5 && 
        owner->entity_type == PLAYER) {
        return;
    }

    self->position.x -= data->rigspeed;
    self->position.y -= data->forspeed;
    self->position.z -= data->upspeed;
  
    if (!proj_exist(self, self->data)) 
        entity_free(self);
    
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
        playdata->wave_flag = 0;
        entity_free(self);
    }    
}
void proj_think_super_nuke(Entity* self) {

}