#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_audio.h"
#include "gf2d_mouse.h"
#include "player.h"
#include "enemy.h"
#include "player_move.h"
#include "projectile.h"
#include "reticle.h"
#include "item.h"
#include "level.h"

#define PLAYER_SPAWN gfc_vector3d(0, 0 ,0);
#define DAMAGE_TIMING 0.5f

static Entity* self;

Entity* player_spawn() {
    PlayerData* data;
    GFC_Vector3D position, reticle_pos;

    // sanity check
    self = entity_new();
    if (!self) return NULL;

    self->model = get_models()->player;
    self->think = player_think;
    self->update = player_update;
    self->free = player_free;
    self->entity_type = PLAYER;

    data = gfc_allocate_array(sizeof(PlayerData), 1);
    if (data) self->data = data;

    if (!data) return NULL;

    player_data_init(data);

    position = PLAYER_SPAWN;
    self->position = position;
    data->og_pos = self->position;
    data->player_pos = &(self->position);
    
    update_hurtbox(self);

    reticle_pos = gfc_vector3d(position.x, -60, position.z);
    data->reticle = reticle_spawn(reticle_pos);

    return self;
}

void player_data_init(PlayerData* data) {
    if (!data) return;

    // movement speed
    data->upspeed = 1.3f;
    data->rigspeed = 1.3f;

    // default player health/resources
    data->maxHealth = 1200.0f;
    data->currHealth = 1200.0f;
    data->maxShield = 0;
    data->currShield = 0;
    data->maxScrap = 50;
    data->currScrap = 0;
    data->vortex_max = 40.0f;
    data->vortex_dur = data->vortex_max; 

    // default player attack
    data->currMode = SINGLE_SHOT;
    data->base_damage = 200.0f;
    data->proj_speed = 8.0f;
    data->vortex_damage = 0.0f;

    data->single_shot_bonus = 0;
    data->charge_shot_mult = 3.0f;
    data->missile_bonus = data->base_damage * 2.0f;
    data->max_missile = 5;
    data->nuke_cost = data->maxScrap;
   
    data->active_item = NONE;
    data->item_duration = 0;

    // default player bounds
    data->x_bound = 49; // left is positive, right is negative
    data->z_bound = 35; // 98 x 70

    // player personal flags/checks
    data->change_flag = 1;
    data->took_damage = 0;
    data->take_damage_timing = 0;
    data->damage_taken = 0;
    data->emp_time = 0;
    data->player_dead = 0;

    // player attack flags/checks
    data->proj_count = 0;
    data->nuke_flag = 0;
    data->vortex_flag = 0;
    data->missile_count = 0;
    data->missile_spawn = 0;
    
    // default shot timing
    data->next_shot = CURRENT_TIME;
    data->next_charged_shot = CURRENT_TIME + NEXT_CHARGE_SHOT;
    data->charge_shot_delay = 0;
    
    // debug init
    data->player_no_attack = 0;
}

void player_think(Entity* self) {
    PlayerData* data;
    ReticleData* rec_data;
    LevelData* level;
    float time;

    if (!self) return;

    data = self->data;
    if (!data) return;

    // don't do anything if player is dead or in_shop or game is pause
    level = get_level_data();
    time = CURRENT_TIME;
    if (data->player_dead || level->in_shop || level->paused || level->wave_end) return;
    
    if (data->emp_time > time && data->active_item != INVINCIBILITY) return;

    // movement checks
    if (!data->mid_roll)
        player_movement(self, data);
    else
        barrel_roll(self, data);

    /* player attack checks */
    time = CURRENT_TIME;
    rec_data = data->reticle->data;

        // CHARGE_SHOT
    if (gf2d_mouse_button_pressed(0) && data->currMode == CHARGE_SHOT && !data->vortex_flag) {
        player_attack(self, data);
        data->next_charged_shot = time + NEXT_CHARGE_SHOT;
        data->charge_shot_delay = time + CHARGE_SHOT_DELAY;
        data->change_flag = 1;
    }
        // SINGLE_SHOT
    else if ((gf2d_mouse_button_pressed(0) || gf2d_mouse_button_held(0)) && !data->mid_roll &&
        data->charge_shot_delay <= time &&
        data->currMode != CHARGE_SHOT && 
        !data->vortex_flag)
    {
        data->next_charged_shot = time + NEXT_CHARGE_SHOT;
        player_attack(self, data);
    }
        // MISSILE
    else if (gf2d_mouse_button_held(2) && data->currScrap > 0 && 
        data->missile_count < data->max_missile && !data->vortex_flag)
    {
        data->currMode = MISSILE;
        
        if (rec_data->locked_on)
            data->missile_spawn = 1;
        else
            data->missile_spawn = 0;
        
        data->next_charged_shot = time + NEXT_CHARGE_SHOT;
        player_attack(self, data);
    }
        // VORTEX
    else if (gfc_input_command_released("vortex") && data->currMode != VORTEX && data->vortex_dur >= (data->vortex_max / 3.0)) {
        data->currMode = VORTEX;
        data->next_charged_shot = time + NEXT_CHARGE_SHOT;
        data->vortex_flag = 1;

        player_attack(self, data);
    }
    else if (gfc_input_command_released("nuke") && !data->vortex_flag && data->currScrap >= data->nuke_cost) {
        data->currMode = SUPER_NUKE;
        data->next_charged_shot = time + NEXT_CHARGE_SHOT;

        player_attack(self, data);
    }

    // debug tools
    //if (gfc_input_command_pressed("freelook")) {
        //data->freelook = !data->freelook;
        //gf3d_camera_enable_free_look(data->freelook);
    //}

    if (gfc_input_command_pressed("change_attack")) {
        if (!data->player_no_attack)
            data->player_no_attack = 1;
        else
            data->player_no_attack = 0;

        data->currScrap = data->maxScrap;
    }
}

void player_update(Entity* self) {
    PlayerData* data;
    LevelData* level;
    float time;

    if (!self) return;

    data = self->data;
    if (!data) return;

    level = get_level_data();
    if (level->in_shop || level->paused || data->player_dead || level->wave_end) return;

    // update camera
    player_cam(self, data);

    /* updates model based on current attack type */
    time = CURRENT_TIME;

        // CHARGE_SHOT texture
    if (time >= data->next_charged_shot && 
        time < data->next_charged_shot + 0.03f && 
        !data->took_damage &&
        !data->vortex_flag &&
        !data->currMode != MISSILE
        ){
        data->currMode = CHARGE_SHOT;
        self->model->texture = get_models()->charge_shot;
    }
        // SINGLE_SHOT texture
    else if (data->take_damage_timing < time &&
             data->change_flag && 
             !data->took_damage &&
             !data->currMode != MISSILE
        ) {
        data->currMode = SINGLE_SHOT;
        self->model->texture = get_models()->single_shot;
        data->change_flag = 0;
    }

    update_hurtbox(self);

    // sanity check, making sure player stats are not over the max
    if (data->currHealth > data->maxHealth)
        data->currHealth = data->maxHealth;
    if (data->currScrap > data->maxScrap)
        data->currScrap = data->maxScrap;
    if (data->currShield > data->maxShield)
        data->currShield = data->maxShield;
    if (data->vortex_dur > data->vortex_max)
        data->vortex_dur = data->vortex_max;

    // update health bar
    data->total_health_bar = data->maxHealth + data->maxShield;
    
    // reduce player movement when shooting
    if (!data->mid_roll) {
        if (gf2d_mouse_button_pressed(0) || gf2d_mouse_button_held(0)) {
            data->upspeed = 1.0f;
            data->rigspeed = 1.0f;
        }
        else {
            data->upspeed = 1.3f;
            data->rigspeed = 1.3f;
        }
    }

    // shield restoration
    if (data->currShield < data->maxShield && data->maxShield > 0) 
        data->currShield += 1.5f;

    // vortex duration restoration
    if (data->currMode != VORTEX && data->vortex_dur < data->vortex_max && !gfc_input_command_held("vortex"))
        data->vortex_dur += 0.1f;

    // active powerup checks
    time = CURRENT_TIME;
    if (data->active_item == HAPPY_TRIGGER && time < data->item_duration) {
        data->currMode = CHARGE_SHOT;
        data->next_charged_shot = 0;
    }
    else if (data->active_item == INVINCIBILITY && time < data->item_duration) {
        // do nothing here
    }
    else
        data->active_item = NONE;

    // check if player was hurt
    if (data->took_damage) {
        player_take_damage(self, data, time);
        if (data->active_item != INVINCIBILITY && data->damaged_type != FENCERS)
            gfc_sound_play(get_sound_data()->player_damaged, 0, 0.3f, -1, -1);
    }

    // check if player is dead
    if (data->currHealth <= 0 && !data->player_dead)
        player_die(self);
}

void player_free(Entity* self){
    PlayerData *data;

    if (!self) return;

    data = (PlayerData*)self->data;
    
    if (data->reticle)
        entity_free(data->reticle);

    free(data);
}

void player_attack(Entity* self, PlayerData* data) {
    GFC_Vector3D attack_start, cursor_pos;
    float curr_time;

    if (!data) return;
    if (!self) return;
    
    gfc_vector3d_copy(attack_start, self->position);
    gfc_vector3d_copy(cursor_pos, data->reticle->position);

    // creates projectile under the ship
    attack_start.z -= 3.0f;
    curr_time = CURRENT_TIME;
    player_proj_spawn(attack_start, cursor_pos, curr_time, 0);
}

void player_take_damage(Entity* self, PlayerData* data, float time) {
    if (!data) return;

    if (data->active_item == INVINCIBILITY)
        return;

    self->model->texture = get_models()->damaged;
    data->change_flag = 1;
    data->take_damage_timing = time + DAMAGE_TIMING;

    if (data->currShield > 0) {
        if (data->currShield - data->damage_taken <= 0) { // not enough shields
            data->currHealth += data->currShield;
            data->currShield = 0;
            data->currHealth -= data->damage_taken;
        }
        else // has enough shields
            data->currShield -= data->damage_taken;
    }
    else // no shields
        data->currHealth -= data->damage_taken;

    data->took_damage = 0;
    data->damage_taken = 0;
}

void player_die(Entity* self) {
    PlayerData* data;

    if (!self) return;

    data = self->data;
    if (!data) return;

    data->player_dead = 1;
    player_death(self);

    /* TODO: add player death animation maybe */
}

void player_death(Entity* self) {
    PlayerData* data;

    data = self->data;
    data->player_no_attack = 1;
    self->model->texture = get_models()->dead;
    
    gfc_sound_play(get_sound_data()->death_sound, 0, 0.5f, -1, -1);

    // kill reticle
    entity_free(data->reticle);
}

void player_respawn(Entity* self) {
    PlayerData* data;
    GFC_Vector3D reticle_pos;

    if (!self) return;

    data = self->data;
    if (!data) return;

    self->position = PLAYER_SPAWN;
    self->model->texture = get_models()->single_shot;
    player_data_init(data);
    reticle_pos = gfc_vector3d(self->position.x, -60.0f, self->position.z);
    data->reticle = reticle_spawn(reticle_pos);
}

void player_quit(Entity* self) {
    if (!self) return;

    entity_free(self);
}

void player_upgrade( PlayerData* data) {
    if (!data) return;

    data->currHealth += data->maxShield;
    data->currShield = 0;
    data->maxShield = 0;

    data->base_damage += data->single_shot_bonus;
    data->single_shot_bonus = 0;
}

PlayerData* get_player_data() {
    if (!self->data) return;
    return self->data;
}

GFC_Primitive get_player_hurtbox(){
    if (!self) return;
    return self->hurtbox;
}




