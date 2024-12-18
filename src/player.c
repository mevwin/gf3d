#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_config.h"
#include "gfc_audio.h"
#include "gf2d_mouse.h"
#include "enemy.h"
#include "player_move.h"
#include "projectile.h"
#include "reticle.h"
#include "item.h"
#include "world.h"

#define PLAYER_SPAWN gfc_vector3d(0, 0 ,0);
#define DAMAGE_TIMING 0.5f

/**
* @brief initialize player data
* @note some values need to be hard-coded
*/
void player_data_init(PlayerData* data);

/**
* @brief initialize player data from a json file
* @note hard-code values as needed
*/
void player_data_init_from_save(PlayerData* data);
void player_free(Entity* self);
void player_attack(Entity* self, PlayerData* data);
void player_take_damage(Entity* self, PlayerData* data, float time);
void player_die();
void player_death();

static Entity* self;

// timing var for passive health regen perk
static float then = 0;

// make custom draw function for animations

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


    if (get_world_data()->continue_from_save)
        player_data_init_from_save(data);
    else 
        player_data_init(data);
        
    position = PLAYER_SPAWN;
    self->position = position;
    data->og_pos = self->position;
    data->player_pos = &(self->position);
    
    update_hurtbox(self);

    reticle_pos = gfc_vector3d(position.x, -60, position.z);

    if (get_world_data()->last_state != LEVEL_EDITOR)
        data->reticle = reticle_spawn(reticle_pos);

    return self;
}

void player_data_init(PlayerData* data) {
    WorldData* world;
    SJson* value;

    if (!data) return;
    
    world = get_world_data();
    value = sj_object_get_value(world->player_init, "data");

    // movement speed
    sj_object_get_value_as_float(value, "upspeed_def", &data->upspeed_def);
    data->upspeed = data->upspeed_def;
    sj_object_get_value_as_float(value, "rigspeed_def", &data->rigspeed_def);
    data->rigspeed = data->rigspeed_def;

    sj_object_get_value_as_float(value, "upspeed_slow", &data->upspeed_slow);
    sj_object_get_value_as_float(value, "rigspeed_slow", &data->rigspeed_slow);

    // default player health/resources
    sj_object_get_value_as_float(value, "maxHealth", &data->maxHealth);
    data->currHealth = data->maxHealth;
    
    // hard code shield stats
    data->maxShield = 0;
    data->currShield = 0;

    sj_object_get_value_as_int(value, "maxScrap", &data->maxScrap);
    data->currScrap = 0;

    sj_object_get_value_as_float(value, "vortex_max", &data->vortex_max);
    data->vortex_dur = data->vortex_max;

    // default player attack
    data->currMode = SINGLE_SHOT;   //hard-coded
    sj_object_get_value_as_float(value, "base_damage", &data->base_damage);
    data->proj_speed = 8.0f;        //hard-coded
    data->vortex_damage = 0.0f;     //hard-coded

    data->single_shot_bonus = 0;
    sj_object_get_value_as_float(value, "charge_shot_mult", &data->charge_shot_mult);
    data->missile_bonus = data->base_damage * 2.0f;
    data->max_missile = 5;
    data->nuke_cost = data->maxScrap;
   
    data->active_item = NONE;
    data->item_duration = 0;

    // create default perks (NO_PERKS)
    // made to be updated later
    data->perk1 = create_dummy_perk();
    data->perk2 = create_dummy_perk();

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
    data->next_shot = 0;
    data->next_charged_shot = 0;
    data->charge_shot_delay = 0;
    
    // debug init
    data->player_no_attack = 0;
}

void player_data_init_from_save(PlayerData* data) {
    SJson* save, *data_entry, *perk_entry;
    PerkType perk_type;

    if (!data) return;

    player_data_init(data);

    save = sj_load("def/player_save.def");
    data_entry = sj_object_get_value(save, "player_data");

    sj_object_get_value_as_float(data_entry, "maxHealth", &data->maxHealth);
    sj_object_get_value_as_float(data_entry, "currHealth", &data->currHealth);
    sj_object_get_value_as_float(data_entry, "maxShield", &data->maxShield);
    sj_object_get_value_as_float(data_entry, "currShield", &data->currShield);
    sj_object_get_value_as_int(data_entry, "maxScrap", &data->maxScrap);
    sj_object_get_value_as_int(data_entry, "currScrap", &data->currScrap);
    sj_object_get_value_as_float(data_entry, "single_shot_bonus", &data->single_shot_bonus);
    sj_object_get_value_as_float(data_entry, "charge_shot_mult", &data->charge_shot_mult);
    sj_object_get_value_as_int(data_entry, "nuke_cost", &data->nuke_cost);

    // load perks
    perk_entry = sj_object_get_value(data_entry, "perk1");
    sj_object_get_value_as_int(perk_entry, "type", &perk_type);
    if (perk_type != NO_PERK) {
        data->perk1->type = perk_type;
        sj_object_get_value_as_int(perk_entry, "uses", &data->perk1->uses);
        sj_object_get_value_as_int(perk_entry, "num_effect", &data->perk1->num_effect);
        strcpy(data->perk1->name, sj_object_get_value_as_string(perk_entry, "name"));
        strcpy(data->perk1->desc, sj_object_get_value_as_string(perk_entry, "desc"));
        data->perk1->color = sj_object_get_color(perk_entry, "color");
    }

    perk_entry = sj_object_get_value(data_entry, "perk2");
    sj_object_get_value_as_int(data_entry, "type", &perk_type);
    if (perk_type != NO_PERK) {
        data->perk2->type = perk_type;
        sj_object_get_value_as_int(perk_entry, "uses", &data->perk2->uses);
        sj_object_get_value_as_int(perk_entry, "num_effect", &data->perk2->num_effect);
        strcpy(data->perk2->name, sj_object_get_value_as_string(perk_entry, "name"));
        strcpy(data->perk2->desc, sj_object_get_value_as_string(perk_entry, "desc"));
        data->perk2->color = sj_object_get_color(perk_entry, "color");
    }

    sj_free(save);
}

void player_think(Entity* self) {
    PlayerData* data;
    ReticleData* rec_data;
    WorldData* world;
    float time;

    if (!self) return;

    data = self->data;
    if (!data) return;

    world = get_world_data();

    time = CURRENT_TIME;
 
    if (data->emp_time > time && data->active_item != INVINCIBILITY) return;

    // movement checks
    if (!data->mid_roll)
        player_movement(self, data);
    else
        barrel_roll(self, data);

    if (world->last_state != LEVEL_EDITOR_START) {
        /* player attack checks */
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

            if (data->perk1->type == REFLECTOR_SHIELD)
                data->perk1->uses--;
            else if (data->perk2->type == REFLECTOR_SHIELD)
                data->perk2->uses--;
        }
        // NUKE
        else if (gfc_input_command_released("nuke") && !data->vortex_flag && data->currScrap >= data->nuke_cost) {
            data->currMode = SUPER_NUKE;
            data->next_charged_shot = time + NEXT_CHARGE_SHOT;

            player_attack(self, data);
        }

        /* debug tools */
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
}

void player_update(Entity* self) {
    WorldData* world;
    PlayerData* data;
    float time;
    float health_rate;   // float container for PASSIVE_HEALS num_effect

    if (!self) return;

    data = self->data;
    if (!data) return;

    world = get_world_data();

    // update camera
    player_cam(self, data);

    /* updates model based on current attack type */
    time = CURRENT_TIME;

    // reduce player movement when shooting
    if (!data->mid_roll) {
        if (gf2d_mouse_button_pressed(0) || gf2d_mouse_button_held(0)) {
            data->upspeed = data->rigspeed_slow;
            data->rigspeed = data->rigspeed_slow;
        }
        else {
            data->upspeed = data->upspeed_def;
            data->rigspeed = data->rigspeed_def;
        }
    }

    if (world->current_state != LEVEL_EDITOR_START) {
        // CHARGE_SHOT texture
        if (time >= data->next_charged_shot &&
            time < (data->next_charged_shot + 0.03f) &&
            !data->took_damage &&
            !data->vortex_flag &&
            !data->currMode != MISSILE
            ) {
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


        // PASSIVE_HEALS perk implementation
        if (data->perk1->type == PASSIVE_HEALS)
            health_rate = (data->perk1->num_effect / 100.0f) * data->maxHealth;
        else if (data->perk2->type == PASSIVE_HEALS)
            health_rate = (data->perk2->num_effect / 100.0f) * data->maxHealth;
        else
            health_rate = 0;


        if (CURRENT_TIME - then >= 1 && data->currHealth + health_rate <= data->maxHealth) {
            data->currHealth += health_rate;
            then = CURRENT_TIME;
        }

        // shield restoration
        if (data->currShield < data->maxShield && data->maxShield > 0)
            data->currShield += 1.5f;

        // vortex duration restoration
        if (data->currMode != VORTEX && data->vortex_dur < data->vortex_max && !gfc_input_command_held("vortex"))
            data->vortex_dur += 0.2f;

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
}

void player_free(Entity* self){
    PlayerData *data;

    if (!self) return;

    data = (PlayerData*) self->data;
    
    if (data->reticle)
        entity_free(data->reticle);

    free(data->perk1);
    free(data->perk2);

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
    Perk* perk;
    float dmg_percentage;      // float container for DMG_RESIST perk (1 means take the full dmg)
    
    if (!data) return;

    if (data->active_item == INVINCIBILITY)
        return;

    // INVINCIBLE_ROLL PERK IMPLEMENTATION
    if (data->mid_roll && (data->roll == RIGHT || data->roll == LEFT)) {
        if (data->perk1->type == INVINCIBLE_ROLL || data->perk2->type == INVINCIBLE_ROLL)
            return;
    }

    // DMG_RESIST PERK IMPLEMENTATION
    perk = data->perk1;
    if (perk->type == DMG_RESIST) 
        dmg_percentage = 1.0f - (( (float) perk->num_effect) / 100.0f);
    else { // first perk is not DMG_RESIST, check other perk
        perk = data->perk2;
        
        if (perk->type == DMG_RESIST)
            dmg_percentage = 1.0f - (((float)perk->num_effect) / 100.0f);
        else
            dmg_percentage = 1.0f;
    }

    self->model->texture = get_models()->damaged;
    data->change_flag = 1;
    data->take_damage_timing = time + DAMAGE_TIMING;

    data->damage_taken *= dmg_percentage;

    if (data->currShield > 0) {
        if (data->currShield - data->damage_taken <= 0) { // not enough shields
            data->currHealth += data->currShield;
            data->currShield = 0;
            data->currHealth -= data->damage_taken * dmg_percentage;

        }
        else // has enough shields
            data->currShield -= data->damage_taken * dmg_percentage;
    }
    else // no shields
        data->currHealth -= data->damage_taken * dmg_percentage;

    // debugging to check if dmg was actually cut
    //if (dmg_percentage < 1.0f)
        //slog("OG dmg: %f | New Dmg: %f", data->damage_taken, data->damage_taken * dmg_percentage);

    data->took_damage = 0;
    data->damage_taken = 0;
}

void player_die() {
    PlayerData* data;

    if (!self) return;

    data = self->data;
    if (!data) return;

    data->player_dead = 1;
    player_death(self);

    /* TODO: add player death animation maybe */
}

void player_death() {
    PlayerData* data;

    data = self->data;
    data->player_no_attack = 1;
    self->model->texture = get_models()->dead;
    
    //gfc_sound_play(get_sound_data()->death_sound, 0, 0.5f, -1, -1);

    // kill reticle
    entity_free(data->reticle);
}

void player_respawn() {
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

void player_upgrade() {
    PlayerData* data;

    data = self->data;
    if (!data) return;

    data->currHealth += data->maxShield;
    data->currShield = 0;
    data->maxShield = 0;

    data->base_damage += data->single_shot_bonus;
    data->single_shot_bonus = 0;
}

Entity* get_player() {
    return self;
}

PlayerData* get_player_data() {
    if (!self) 
        return NULL;
    else 
        return self->data;
}

GFC_Primitive get_player_hurtbox(){
    if (!self) return;
    return self->hurtbox;
}




