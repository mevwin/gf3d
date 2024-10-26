#include "simple_logger.h"
#include "gf2d_mouse.h"
#include "projectile.h"
#include "player.h"
#include "enemy.h"
#include "reticle.h"

void player_proj_spawn(GFC_Vector3D position, GFC_Vector3D reticle_pos, float curr_time, Uint8 vortexed) {
    Entity* self;
    ProjData* data;
    PlayerData* player_data;
    ReticleData* rec_data;
    float dist_x, dist_y, conver, z_angle, y_angle, time;

    self = entity_new();
    if (!self) return;

    data = gfc_allocate_array(sizeof(ProjData), 1);
    if (data) self->data = data;

    player_data = get_player_data();

    time = SDL_GetTicks() / 1000.0;

    /**
    * prevent projectile from spawning if:
    *   one wave_shot is active
    *   missile count reaches the max limit
    *   currently can't spawn a missile
    *   player has reached allowed amount of projectiles
    *   player can't shoot yet due to shot delay (for single_shot)
    */
    if ((player_data->currMode == MISSILE && player_data->missile_count > player_data->max_missile) ||
        (player_data->currMode == MISSILE && !player_data->missile_spawn) ||
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

    data->y_bound = -170;
    if (vortexed) {
        data->type = CHARGE_SHOT;
        data->vortexed = 1;
    }
    else
        data->type = player_data->currMode;

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
        self->model = data->type == SINGLE_SHOT ? get_models()->single_proj : get_models()->charge_proj;
        data->forspeed = data->type == SINGLE_SHOT ? player_data->proj_speed : player_data->proj_speed * 1.25;
        data->damage = data->type == SINGLE_SHOT ? player_data->base_damage + player_data->single_shot_bonus : player_data->base_damage * player_data->charge_shot_mult;
        
        if (vortexed)
            data->damage = player_data->vortex_damage;

        player_data->next_shot = data->type == SINGLE_SHOT ? curr_time + 0.15 : 0;

        conver = reticle_pos.y / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);
    }
    else if (data->type == MISSILE) {
        player_data->missile_count++;
        if (player_data->missile_count > player_data->max_missile) {
            entity_free(self);
            return;
        }

        rec_data = player_data->reticle->data;

        self->think = proj_think_missile;
        self->model = get_models()->single_proj;
        data->forspeed = player_data->proj_speed * 0.75;
        //data->forspeed = 1.0;
        data->damage = player_data->base_damage * player_data->missile_mult;
        data->missile_target = rec_data->enemy_pos;
        //data->missile_target = &(player_data->reticle->position);

        // initial movement speed
        dist_x = data->missile_target->x - self->position.x;
        dist_y = data->missile_target->z - self->position.z;

        conver = data->missile_target->y / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);

        player_data->currScrap--;
        player_data->missile_spawn = 0;
        data->missile_active = 0;
    }
    else if (data->type == VORTEX) {
        self->think = proj_think_vortex;
    }
    
    if (data->type != VORTEX) {
        self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                                self->position.y - (self->model->bounds.h / 2),
                                self->position.z - (self->model->bounds.d / 2),
                                self->model->bounds.w,
                                self->model->bounds.h,
                                self->model->bounds.d);
    }

    //slog("Rig: %f | Up: %f", data->rigspeed, data->upspeed);

}

void enemy_proj_spawn(GFC_Vector3D position, GFC_Vector3D player_pos, Entity* owner, float curr_time) {
    Entity* self;
    ProjData* data;
    EnemyData* enemy_data;
    float dist_x, dist_y, conver, z_angle, y_angle, time;

    self = entity_new();
    if (!self) return;

    if (!owner) return;

    data = gfc_allocate_array(sizeof(ProjData), 1);
    if (data) self->data = data;

    data->owner = owner;
    enemy_data = owner->data;

    time = SDL_GetTicks() / 1000.0;

    // enforcing maximum projectile count per entity
    if (enemy_data->proj_count == MAX_PROJ || 
        time < enemy_data->next_single_shot
    ) {
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
    data->y_bound = 90;
    data->vortexed = 0;

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
        self->model = data->type == PEAS ? get_models()->peas : get_models()->charge_proj;
        data->forspeed = data->type == PEAS ? enemy_data->pea_speed : enemy_data->pea_speed * 1.25;
        data->damage = data->type == PEAS ? enemy_data->base_damage : enemy_data->base_damage * 3;
        enemy_data->next_single_shot = data->type == PEAS ? curr_time + 1.0 : curr_time + 1.0;

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
    if (!data) return;

    if (get_player_data()->in_shop || get_player_data()->paused) return;

    if (data->owner_type == ENEMY) {
        enemy_data = data->owner->data;
        if (enemy_data->currHealth <= 0.0) 
            data->y_bound = -30;
    }

    // updates hurtbox if not
    if (data->type != VORTEX) {
        self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
                                self->position.y - (self->model->bounds.h / 2),
                                self->position.z - (self->model->bounds.d / 2),
                                self->model->bounds.w,
                                self->model->bounds.h,
                                self->model->bounds.d);
    }


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
    // only initate check if enemy projectile is close enough to player or if the projectile is from player
    if ((self->position.y > -20.0 && data->owner_type == ENEMY) || 
        (data->owner_type == PLAYER && self->position.y < -40.0 && data->type != VORTEX)) 
    {
        entityList = get_entityList();
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
                    if (data->type = MISSILE)
                        get_player_data()->currMode = SINGLE_SHOT;
                    
                }
                else if (data->owner_type == ENEMY) {
                    player_data = target->data;
                    player_data->took_damage = 1;
                    player_data->damaged_type = data->type;
                    player_data->damage_taken = data->damage;
                }
                entity_free(self);
                break;
            }
        }
    }
}

void proj_free(Entity* self) {
    ProjData* data;
    EnemyData* enemydata;
    Entity* owner;

    if (!self) return;

    data = self->data;
    if (!data) return;
    
    if (data->owner_type == PLAYER) {
        get_player_data()->proj_count--;
        if (data->type == MISSILE)
            get_player_data()->missile_count--;
    }
    else if (data->owner_type == ENEMY) {
        enemydata = data->owner->data;
        enemydata->proj_count--;
    }

    free(data);
}

Uint8 proj_exist(Entity* self, ProjData* data) {
    if (!self || !data) return 0;

    if (data->owner_type == PLAYER && self->position.y < data->y_bound) 
       return 0;

    else if (data->owner_type == ENEMY && self->position.y > data->y_bound) 
        return 0;
    
    return 1;
}

void proj_think_basic(Entity* self) {
    ProjData* data;
    EnemyData* enemy_data;

    if (!self) return;

    data = self->data;
    if (!data) return;

    if (get_player_data()->in_shop || get_player_data()->paused || data->vortexed) return;

    if (data->owner_type == PLAYER)
        self->position.y -= data->forspeed;
    else
        self->position.y += data->forspeed;

    self->position.x -= data->rigspeed;    
    self->position.z -= data->upspeed;
    
    if (!proj_exist(self, self->data))
        entity_free(self);
}

void proj_think_missile(Entity* self) {
    ProjData* data;

    if (!self) return;

    data = self->data;
    if (!data) return;

    if (get_player_data()->in_shop || get_player_data()->paused) return;

    if ((gf2d_mouse_button_held(2) || gf2d_mouse_button_pressed(2)) && 
        !data->missile_active) 
        return;
    
    data->missile_active = 1;

    self->position.x -= data->rigspeed;
    self->position.y -= data->forspeed;
    self->position.z -= data->upspeed;
  
    if (!proj_exist(self, self->data))
        entity_free(self);
   
}

void proj_think_vortex(Entity* self) {
    PlayerData* p_data;
    ProjData* data;
    Entity* entityList, *proj;
    GFC_Vector3D player_pos;
    int i;
    float time;

    if (!self) return;

    p_data = get_player_data();
    if (!p_data) return;

    if (p_data->in_shop || p_data->paused) return;

    if (gf2d_mouse_button_pressed(0))
        p_data->vortex_flag = 0;

    entityList = get_entityList();
    if (p_data->vortex_flag && p_data->vortex_dur > 0.0) {
        if (p_data->vortex_dur - 1.0 <= 0.0)
            p_data->vortex_dur = 0.0;
        else
            p_data->vortex_dur -= 1.0;

        for (i = 0; i < MAX_ENTITY; i++) {
            proj = &entityList[i];

            if (proj->entity_type != PROJECTILE)
                continue;

            // collision detection check
            data = proj->data;
            player_pos.x = p_data->player_pos->x;
            player_pos.y = p_data->player_pos->y;
            player_pos.z = p_data->player_pos->z;

            if (data->owner_type == ENEMY && gfc_vector3d_distance_between_less_than(player_pos, proj->position, 20.0)) {
                p_data->vortex_damage += data->damage;
                data->damage = 0.0;
                entity_free(proj);
            }
        }
    }

    if (!p_data->vortex_flag) {
        time = SDL_GetTicks() / 1000.0;
        if (p_data->vortex_damage > 0.0) {
            player_pos.x = p_data->player_pos->x;
            player_pos.y = p_data->player_pos->y;
            player_pos.z = p_data->player_pos->z;

            player_proj_spawn(player_pos, get_reticle_pos(), time, 1);
        }

        p_data->currMode = SINGLE_SHOT;
        p_data->vortex_damage = 0.0;
        p_data->next_charged_shot = time + 0.9;
        p_data->next_shot = time + 0.15;

        for (i = 0; i < MAX_ENTITY; i++) {
            proj = &entityList[i];

            if (proj->entity_type != PROJECTILE)
                continue;

            data = proj->data;
            if (data->vortexed) {
                data->vortexed = 0;
            }
        }
        entity_free(self);
    }
}

void proj_think_super_nuke(Entity* self) {

}