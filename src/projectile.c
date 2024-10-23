#include "simple_logger.h"
#include "gf2d_mouse.h"
#include "projectile.h"
#include "player.h"
#include "enemy.h"
#include "reticle.h"

void player_proj_spawn(GFC_Vector3D position, GFC_Vector3D reticle_pos, Entity* owner, float curr_time) {
    Entity* self;
    ProjData* data;
    PlayerData* player_data;
    ReticleData* rec_data;
    float dist_x, dist_y, conver, z_angle, y_angle, time;

    self = entity_new();
    if (!self) return;

    data = gfc_allocate_array(sizeof(ProjData), 1);
    if (data) self->data = data;

    data->owner = owner;
    player_data = owner->data;

    time = SDL_GetTicks() / 1000.0;

    /**
    * prevent projectile from spawning if:
    *   one wave_shot is active
    *   missile count reaches the max limit
    *   currently can't spawn a missile
    *   player has reached allowed amount of projectiles
    *   player can't shoot yet due to shot delay (for single_shot)
    */
    if ((player_data->curr_mode == WAVE_SHOT && player_data->wave_flag == MAX_WAVE) ||
        (player_data->curr_mode == MISSILE && player_data->missile_count >= player_data->max_missile) ||
        (player_data->curr_mode == MISSILE && !player_data->missile_spawn) ||
        player_data->proj_count >= MAX_PROJ ||
        time < player_data->next_shot
        ) {
       //slog("%d", player_data->missile_count);
        entity_free(self);
        return;
    }
    
    player_data->proj_count++;

    self->update = proj_update;
    self->entity_type = PROJECTILE;
    data->owner_type = PLAYER;
    self->position = position;
    self->free = proj_free;

    data->type = player_data->curr_mode;
    data->player_in_shop = 0;
    data->y_bound = -170;

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
        data->forspeed = data->type == SINGLE_SHOT ? player_data->proj_speed : player_data->proj_speed * 1.25;
        data->damage = data->type == SINGLE_SHOT ? player_data->base_damage + player_data->single_shot_bonus : player_data->base_damage * player_data->charge_shot_mult;
        player_data->next_shot = data->type == SINGLE_SHOT ? curr_time + 0.15 : 0;

        conver = reticle_pos.y / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);
    }
    /**
    else if (data->type == WAVE_SHOT) {
        self->think = proj_think_wave_shot;
        self->model = gf3d_model_load("models/projectiles/wave_shot.model");
        data->forspeed = player_data->proj_speed / 2;
        player_data->wave_flag = 1;
    }
    */
    else if (data->type == MISSILE) {
        rec_data = player_data->reticle->data;

        self->think = proj_think_missile;
        self->model = gf3d_model_load("models/projectiles/single_shot.model");
        data->forspeed = player_data->proj_speed * 0.75;
        //data->forspeed = 1.0;
        data->damage = player_data->base_damage * 3;
        data->missile_target = rec_data->enemy_pos;
        //data->missile_target = &(player_data->reticle->position);

        // initial movement speed
        dist_x = data->missile_target->x - self->position.x;
        dist_y = data->missile_target->z - self->position.z;

        conver = data->missile_target->y / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);

        player_data->missile_count++;
        player_data->currScrap--;
        player_data->missile_spawn = 0;
        data->missile_active = 0;
    }
    
    self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                            self->position.y - (self->model->bounds.h / 2),
                            self->position.z - (self->model->bounds.d / 2),
                            self->model->bounds.w,
                            self->model->bounds.h,
                            self->model->bounds.d);

    //slog("Rig: %f | Up: %f", data->rigspeed, data->upspeed);

    
}

void enemy_proj_spawn(GFC_Vector3D position, GFC_Vector3D player_pos, Entity* owner, float curr_time) {
    Entity* self;
    ProjData* data;
    EnemyData* enemy_data;
    float dist_x, dist_y, conver, z_angle, y_angle, time;

    self = entity_new();
    if (!self) return;

    data = gfc_allocate_array(sizeof(ProjData), 1);
    if (data) self->data = data;

    data->owner = owner;
    enemy_data = owner->data;

    time = SDL_GetTicks() / 1000.0;

    // enforcing maximum projectile count per entity
    if (enemy_data->proj_count == MAX_PROJ || 
        time < enemy_data->next_single_shot
    ) {
        //slog("%i", enemydata->proj_count);
        entity_free(self);
        return;
    }

    enemy_data->proj_count++;

    self->update = proj_update;
    self->entity_type = PROJECTILE;
    data->owner_type = ENEMY;
    self->position = position;
    self->free = proj_free;

    data->type = enemy_data->enemy_type;
    data->player_in_shop = 0;
    data->y_bound = 90;

    // rotating projectile to player
    if (data->type == PEAS || data->type == CHARGERS) {
        dist_x = player_pos.x - position.x;
        dist_y = player_pos.z - position.z;

        z_angle = atan(dist_x / enemy_data->dist_to_player);
        self->rotation.z += z_angle;

        y_angle = atan(dist_y / enemy_data->dist_to_player);
        self->rotation.y -= y_angle;
    }

    if (data->type == PEAS || data->type == CHARGE_SHOT) {
        self->think = proj_think_basic;
        self->model = data->type == PEAS ? gf3d_model_load("models/projectiles/single_shot_enem.model") : gf3d_model_load("models/projectiles/charge_shot.model");
        data->forspeed = data->type == PEAS ? enemy_data->pea_speed : enemy_data->pea_speed * 1.25;
        data->damage = data->type == PEAS ? enemy_data->base_damage : enemy_data->base_damage * 3;
        enemy_data->next_single_shot = data->type == PEAS ? curr_time + 1.0 : 3.0;

        conver = enemy_data->dist_to_player / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);
    }

    self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                            self->position.y - (self->model->bounds.h / 2),
                            self->position.z - (self->model->bounds.d / 2),
                            self->model->bounds.w,
                            self->model->bounds.h,
                            self->model->bounds.d
    );

    //slog("Rig: %f | Up: %f", data->rigspeed, data->upspeed);
}

void proj_update(Entity* self) {
    ProjData* data;
    PlayerData* player_data;
    EnemyData* enemy_data;
    Entity* target, * entityList;
    int i;
    float dist_x, dist_y, conver, z_angle, y_angle;

    if (!self) return;

    data = self->data;
    entityList = get_entityList();

    if (data->player_in_shop) return;

    // updates hurtbox
    self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                            self->position.y - (self->model->bounds.h / 2),
                            self->position.z - (self->model->bounds.d / 2),
                            self->model->bounds.w,
                            self->model->bounds.h,
                            self->model->bounds.d);

    // update missile trajectory if missile is active
    if (data->type == MISSILE && data->missile_active){
        dist_x = data->missile_target->x - self->position.x;
        dist_y = data->missile_target->z - self->position.z;

        z_angle = atan(dist_x / data->missile_target->y);
        self->rotation.z = -z_angle;

        y_angle = atan(dist_y / data->missile_target->y);
        self->rotation.y = y_angle;

        conver = (data->missile_target->y - self->position.y) / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);
    }
    
    
    // checks if projectile hits anything
    for (i = 0; i < MAX_ENTITY; i++) {
        target = &entityList[i];
        
        if ((target->entity_type != ENEMY && data->owner_type == PLAYER) ||
            (target->entity_type != PLAYER && data->owner_type == ENEMY) ||
            target->entity_type == PROJECTILE ||
            target->entity_type == RETICLE ||
            target->entity_type == ITEM
            )
            continue;

        // collision detection check
        if (gfc_box_overlap(self->hurtbox, target->hurtbox)) {
            if (data->owner_type == PLAYER) {
                enemy_data = target->data;
                enemy_data->took_damage = 1;
                enemy_data->damaged_type = data->type;
                enemy_data->damage_taken = data->damage;
                if (data->type = MISSILE) {
                    player_data = data->owner->data;
                    player_data->curr_mode = SINGLE_SHOT;
                }
                if (enemy_data->currHealth > 0.0) {
                    entity_free(self);
                }
            }
            else if (data->owner_type == ENEMY) {
                player_data = target->data;
                player_data->took_damage = 1;
                player_data->damaged_type = data->type;
                player_data->damage_taken = data->damage;
                if (player_data->currHealth > 0.0) {
                    entity_free(self);
                }
            }
            break;
        }
    }
}

void proj_free(Entity* self) {
    ProjData* data;
    PlayerData* playdata;
    EnemyData* enemydata;
    Entity* owner;

    if (!self) return;

    data = (ProjData*)self->data;
    owner = data->owner;
    
    if (data->owner_type == PLAYER) {
        playdata = owner->data;
        playdata->proj_count--;
        if (data->type == MISSILE)
            playdata->missile_count--;
    }
    else if (data->owner_type == ENEMY) {
        enemydata = owner->data;
        enemydata->proj_count--;
    }

    free(data);
    self->data = NULL;

}

Uint8 proj_exist(Entity* self, ProjData* data) {
    if (data->owner_type == PLAYER && self->position.y < data->y_bound) 
       return 0;

    else if (data->owner_type == ENEMY && self->position.y > data->y_bound) 
        return 0;
    
    return 1;
}

void proj_think_basic(Entity* self) {
    ProjData* data;
    PlayerData* player_data;
    EnemyData* enemy_data;

    data = self->data;
    if (!data) return;

    if (data->owner_type == PLAYER) {
        player_data = (PlayerData*) data->owner->data;
        if (player_data->in_shop) {
            data->player_in_shop = 1;
            return;
        }
        self->position.y -= data->forspeed;
    }
    else {
        enemy_data = (EnemyData*) data->owner->data;
        player_data = (PlayerData*) enemy_data->player_data;
        if (player_data->in_shop) {
            data->player_in_shop = 1;
            return;
        }
        self->position.y += data->forspeed;
    }

    self->position.x -= data->rigspeed;    
    self->position.z -= data->upspeed;
    
    if (!proj_exist(self, self->data))
        entity_free(self);
}

void proj_think_missile(Entity* self) {
    ProjData* data;
    PlayerData* player_data;

    data = self->data;
    if (!data) return;

    player_data = data->owner->data;

    if (player_data->in_shop) {
        data->player_in_shop = 1;
        return;
    }

    if ((gf2d_mouse_button_held(2) || gf2d_mouse_button_pressed(2)) && 
        !data->missile_active) {
        return;
    }
    data->missile_active = 1;

    self->position.x -= data->rigspeed;
    self->position.y -= data->forspeed;
    self->position.z -= data->upspeed;
  
    if (!proj_exist(self, self->data))
        entity_free(self);
   
}

void proj_think_wave_shot(Entity* self) {
    ProjData* data;
    PlayerData* player_data;

    data = self->data;
    if (!data) return;

    player_data = data->owner->data;
    self->position.y -= data->forspeed;

    if (!proj_exist(self, self->data)) {
        player_data->curr_mode = SINGLE_SHOT;
        player_data->change_flag = 1;
        player_data->wave_flag = 0;
        entity_free(self);
    }    
}
void proj_think_super_nuke(Entity* self) {

}