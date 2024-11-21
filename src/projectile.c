#include "simple_logger.h"
#include "gf2d_mouse.h"
#include "gfc_audio.h"
#include "gf2d_draw.h"
#include "projectile.h"
#include "player.h"
#include "enemy.h"
#include "reticle.h"
#include "item.h"
#include "level.h"
#include "world.h"

#define SHOT_DELAY 0.15f
#define SHOT_DELAY_TRIGGER 0.3f
#define SUPER_NUKE_DMG 20.0f
#define ENEMY_FENCER_BOX(p) (gfc_box(p.x - 24.0f, p.y - 13.0f, p.z - 17.0f, 48.0f, 26.0f, 34.0f))

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

    time = CURRENT_TIME;

    /**
    * prevent projectile from spawning if:
    *   missile count reaches the max limit
    *   currently can't spawn a missile
    *   player has reached allowed amount of projectiles
    *   player can't shoot yet due to shot delay (for single_shot)
    *   a super nuke is active
    */
    if ((player_data->currMode == MISSILE && player_data->missile_count > player_data->max_missile) ||
        (player_data->currMode == MISSILE && !player_data->missile_spawn) ||
        player_data->proj_count >= MAX_PROJ ||
        time < player_data->next_shot ||
        player_data->nuke_flag
        ) {
       //slog("%d", player_data->missile_count);
        entity_free(self);
        return;
    }
    
    player_data->proj_count++;

    self->update = proj_update_player;
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
        data->forspeed = data->type == SINGLE_SHOT ? 
            player_data->proj_speed : player_data->proj_speed * 1.25f;
        
        data->damage = data->type == SINGLE_SHOT ? 
            player_data->base_damage + player_data->single_shot_bonus : player_data->base_damage * player_data->charge_shot_mult;
        
        if (vortexed)
            data->damage = player_data->vortex_damage;

        player_data->next_shot = data->type == SINGLE_SHOT ? curr_time + SHOT_DELAY : 0;

        if (player_data->active_item == HAPPY_TRIGGER)
            player_data->next_shot = curr_time + SHOT_DELAY_TRIGGER;

        conver = reticle_pos.y / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);
        data->vortexed = 0;
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
        data->forspeed = player_data->proj_speed * 0.75f;
        data->damage = player_data->base_damage + player_data->missile_bonus;
        data->missile_target = rec_data->enemy_pos;

        // initial movement speed
        dist_x = data->missile_target->x - self->position.x;
        dist_y = data->missile_target->z - self->position.z;

        conver = data->missile_target->y / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);

        player_data->currScrap--;
        if (player_data->currScrap < 0)
            player_data->currScrap = 0;

        player_data->missile_spawn = 0;
        data->missile_active = 0;
    }
    else if (data->type == VORTEX) {
        self->think = proj_think_vortex;
    }
    else if (data->type == SUPER_NUKE) {
        player_data->nuke_flag = 1;
        self->think = proj_think_super_nuke;
        self->model = get_models()->super_nuke;
        data->damage = SUPER_NUKE_DMG * (get_level_data()->wave_count);
        data->nuke_deton_pos = gfc_vector3d(0, reticle_pos.y, 0);

        data->forspeed = 1.5f;

        dist_x = data->nuke_deton_pos.x - self->position.x;
        dist_y = data->nuke_deton_pos.z - self->position.z;

        conver = data->nuke_deton_pos.y / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);

        player_data->currScrap -= player_data->nuke_cost;
        if (player_data->currScrap < 0)
            player_data->currScrap = 0;

        time = CURRENT_TIME;
        data->nuke_dur = time + 3.0f;
        data->nuke_active = 0;
    }
    
    if (data->type != VORTEX && data->type != SUPER_NUKE) 
        update_hurtbox(self);
    
    if (data->type != VORTEX)
        gfc_sound_play(get_sound_data()->projectile_fire, 0, 0.2f, -1, -1);

    //slog("Rig: %f | Up: %f", data->rigspeed, data->upspeed);

}

void enemy_proj_spawn(GFC_Vector3D position, GFC_Vector3D player_pos, Entity* owner, float curr_time) {
    Entity* self;
    ProjData* data;
    EnemyData* enemy_data;
    LevelData* level;
    Entity_Models* models;
    float dist_x, dist_y, conver, z_angle, y_angle, time;

    self = entity_new();
    if (!self) return;

    if (!owner) return;

    data = gfc_allocate_array(sizeof(ProjData), 1);
    if (data) self->data = data;

    data->owner = owner;
    enemy_data = owner->data;

    time = CURRENT_TIME;
    level = get_level_data();
    models = get_models();

    // enforcing maximum projectile count per entity
    if (enemy_data->proj_count == MAX_PROJ ||
        time < enemy_data->next_single_shot ||
        (enemy_data->enemy_type == FENCERS && enemy_data->proj_count >= 1)
        ){
        entity_free(self);
        return;
    }

    enemy_data->proj_count++;
    self->entity_type = PROJECTILE;
    data->owner_type = ENEMY;
    data->type = enemy_data->enemy_type;

    if (enemy_data->enemy_type == FENCERS) {
        self->think = fencer_think;
        self->position = player_pos;
        
        self->free = proj_free;
        self->model = models->fencer_attack;
        self->hurtbox.s.b = ENEMY_FENCER_BOX(self->position);
        self->no_draw = 1;

        data->damage = enemy_data->base_damage;
        level->fencer_spawn = player_pos;
        return;
    }

    self->update = proj_update_enemy;
    self->position = position;
    self->free = proj_free;
    data->y_bound = 90;
    data->vortexed = 0;

    // rotating projectile to player
    if (data->type == PEAS || data->type == CHARGERS || data->type == BOMBERS) {
        dist_x = player_pos.x - position.x;
        dist_y = player_pos.z - position.z;

        z_angle = atan(dist_x / enemy_data->dist_to_player);
        self->rotation.z += z_angle;

        y_angle = atan(dist_y / enemy_data->dist_to_player);
        self->rotation.y -= y_angle;
    }

    if (data->type == PEAS || data->type == CHARGE_SHOT) {
        self->think = proj_think_basic;
        self->model = data->type == PEAS ? models->peas_shot : models->chargers_shot;
        data->forspeed = data->type == PEAS ? enemy_data->pea_speed : enemy_data->pea_speed * 1.25f;
        data->damage = data->type == PEAS ? enemy_data->base_damage : enemy_data->base_damage * 2.0f;
        enemy_data->next_single_shot = data->type == PEAS ? curr_time + 0.9f : curr_time + 1.8f;

        conver = enemy_data->dist_to_player / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);
    }
    else if (data->type == BOMBERS) {
        self->think = bomber_think;
        self->model = models->single_proj;
        data->forspeed = 1.0f;
        data->damage = enemy_data->base_damage;

        conver = enemy_data->dist_to_player / data->forspeed;
        data->rigspeed = (dist_x / conver);
        data->upspeed = (dist_y / conver);
    }

    update_hurtbox(self);

    //gfc_sound_play(get_sound_data()->projectile_fire, 0, 0.1f, -1, -1);

    //slog("Rig: %f | Up: %f", data->rigspeed, data->upspeed);
}

void proj_update_player(Entity* self) {
    ProjData* data;
    PlayerData* player_data;
    EnemyData* enemy_data;
    LevelData* level;
    Entity* target, *entityList;
    int i;
    float dist_x, dist_y, conver, z_angle, y_angle;

    if (!self) return;

    data = self->data;
    if (!data) return;

    level = get_level_data();

    // updates hurtbox if not vortex or super_nuke
    if (data->type != VORTEX && data->type != SUPER_NUKE)
        update_hurtbox(self);

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
    if (self->position.y < -40.0f && data->type != VORTEX && data->type != SUPER_NUKE) {
        entityList = get_entityList();

        // check which enemy got hit
        for (i = 0; i < MAX_ENTITY; i++) {
            target = &entityList[i];

            if (target->entity_type != ENEMY)
                continue;

            // collision detection check
            if (gfc_box_overlap(self->hurtbox.s.b, target->hurtbox.s.b)) {
                enemy_data = target->data;
                enemy_data->took_damage = 1;
                enemy_data->damaged_type = data->type;
                enemy_data->damage_taken = data->damage;
              
                entity_free(self);
                break;
            }
        }
    }
}

void proj_update_enemy(Entity* self) {
    ProjData* data;
    PlayerData* player_data;
    EnemyData* enemy_data;
    LevelData* level;

    if (!self) return;

    data = self->data;
    if (!data) return;

    level = get_level_data();

    update_hurtbox(self);

    player_data = get_player_data();
    if (player_data->nuke_flag)
        entity_free(self);
 
    enemy_data = data->owner->data;
    if (enemy_data->currHealth <= 0 && enemy_data->enemy_type != BOMBERS)
        data->y_bound = -30;

    // enemy attacking player
    if (self->position.y > -20.0f && gfc_box_overlap(self->hurtbox.s.b, get_player_hurtbox().s.b)) {
        player_data->took_damage = 1;
        player_data->damaged_type = data->type;
        player_data->damage_taken = data->damage;
        entity_free(self);
    }
}

void proj_free(Entity* self) {
    ProjData* data;
    PlayerData* player_data;
    EnemyData* enemydata;

    if (!self) return;

    data = self->data;
    if (!data) return;

    if (data->owner_type == PLAYER) {
        player_data = get_player_data();
        player_data->proj_count--;
        if (data->type == MISSILE)
            player_data->missile_count--;

        // stay is missile mode until all missiles are gone
        if (player_data->missile_count > 0)
            player_data->currMode = MISSILE;
        else
            player_data->currMode = SINGLE_SHOT; 
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
    LevelData* level;

    if (!self) return;

    data = self->data;
    if (!data) return;

    level = get_level_data();
    if (data->vortexed) return;

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


    if ((gf2d_mouse_button_held(2) || gf2d_mouse_button_pressed(2)) && 
        !data->missile_active) 
        return;

    if (get_player_data()->missile_count >= get_player_data()->max_missile)
        entity_free(self);
    
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


    if (p_data->player_dead) 
        entity_free(self);

    if (gf2d_mouse_button_pressed(0) ||
        p_data->vortex_flag && p_data->vortex_damage <= 0 && p_data->vortex_dur <= 0)
        p_data->vortex_flag = 0;

    if (p_data->vortex_flag && p_data->vortex_dur > 0) {
        entityList = get_entityList();
        for (i = 0; i < MAX_ENTITY; i++) {
            proj = &entityList[i];

            if (proj->entity_type != PROJECTILE)
                continue;

            // collision detection check
            data = proj->data;
            
                // skip fencer attack
            if (data->type == FENCERS)
                continue;
         
            gfc_vector3d_copy_ptr(player_pos, p_data->player_pos);

            if (data->owner_type == ENEMY && gfc_vector3d_distance_between_less_than(player_pos, proj->position, 20.0f)) {
                p_data->vortex_damage += data->damage;
                data->damage = 0;
                entity_free(proj);
            }
        }

        p_data->vortex_dur -= 1.0f;
    }
    else if (!p_data->vortex_flag) {
        time = CURRENT_TIME;

        p_data->vortex_damage *= 1.7f; // damage scaling

        if (p_data->vortex_damage > 0) {
            player_pos.x = p_data->player_pos->x;
            player_pos.y = p_data->player_pos->y;
            player_pos.z = p_data->player_pos->z;

            player_proj_spawn(player_pos, get_reticle_pos(), time, 1);
        }
        // else: do nothing if nothing was vortexed

        p_data->currMode = SINGLE_SHOT;
        p_data->vortex_damage = 0;
        p_data->vortex_dur = 0;
        p_data->next_charged_shot = time + 0.9f;
        p_data->next_shot = time + 0.15f;

        entity_free(self);
    }
}

void proj_think_super_nuke(Entity* self) {
    ProjData* data;
    PlayerData* p_data;
    EnemyData* e_data;
    LevelData* level;
    Entity* entityList, *enemy;
    float time;
    int i;

    if (!self) return;

    data = self->data;
    if (!data) return;

    level = get_level_data();

    p_data = get_player_data();
    if (p_data->player_dead) 
        entity_free(self);

    if (!data->nuke_active) {
        // move to detonation position
        self->position.x -= data->rigspeed;
        self->position.y -= data->forspeed;
        self->position.z -= data->upspeed;

        if (self->position.y <= data->nuke_deton_pos.y)
            data->nuke_active = 1;
    }
    else { // nuke is active;
        // stay in place at detonation position
        // let ui.c take care of the visuals
        time = CURRENT_TIME;

        if (time < data->nuke_dur) {
            // attack all enemies
            entityList = get_entityList();
            for (i = 0; i < MAX_ENTITY; i++) {
                enemy = &entityList[i];

                if (enemy->entity_type != ENEMY)
                    continue;

                e_data = enemy->data;
                e_data->took_damage = 1;
                e_data->damaged_type = data->type;
                e_data->damage_taken = data->damage;
            }
        }
        else {
            p_data->nuke_flag = 0;
            entity_free(self);
        }
    }
}

void fencer_think(Entity* self) {
    PlayerData* p_data;
    EnemyData* enemy_data;
    LevelData* level;
    ProjData* data;
    float time;

    if (!self) return;

    data = self->data;
    if (!data) return;

    p_data = get_player_data();
    enemy_data = data->owner->data;
    level = get_level_data();
    time = CURRENT_TIME;
    
    if (!gfc_box_overlap(self->hurtbox.s.b, get_player_hurtbox().s.b)){
        p_data->damage_taken = data->damage;
        p_data->damaged_type = FENCERS;
        p_data->took_damage = 1;
    }

    if (enemy_data->currHealth <= 0)
        entity_free(self);
    
}

void bomber_think(Entity* self) {
    PlayerData* p_data;
    EnemyData* enemy_data;
    LevelData* level;
    ProjData* data;
    float dist_x, dist_y, conver, z_angle, y_angle;

    if (!self) return;

    data = self->data;
    if (!data) return;

    p_data = get_player_data();
    enemy_data = data->owner->data;
    level = get_level_data();

    // update attack trajectory
    dist_x = enemy_data->player_pos->x - self->position.x;
    dist_y = enemy_data->player_pos->z - self->position.z;

    z_angle = atan(dist_x / (enemy_data->dist_to_player + 5));
    self->rotation.z = z_angle;

    y_angle = atan(dist_y / (enemy_data->dist_to_player + 5));
    self->rotation.y = -y_angle;

    conver = self->position.y / data->forspeed;
    data->rigspeed = (dist_x / conver);
    data->upspeed = (dist_y / conver);

    self->position.x -= data->rigspeed;
    self->position.y += data->forspeed;
    self->position.z -= data->upspeed;
}
