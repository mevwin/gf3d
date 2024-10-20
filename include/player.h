#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "gfc_shape.h"
#include "entity.h"

typedef struct {
    GFC_Vector3D    og_pos;             // container for keeping model in place due to constant rotation
    GFC_Vector3D*   reticle_pos;        // pointer to reticle's position

    // player values
    float           upspeed;            // vertical speed
    float           rigspeed;           // horizontal speed

    GFC_List        inventory;          // list of items
    GFC_List        upgrades;           // list of upgrades currently active

    float           maxHealth;          // player's maximum health
    float           currHealth;         // player's current health
    float           maxShipParts;       // maximum amount of ship parts a player can hold
    float           currShipParts;      // player's currrent amount of ship parts

    float           base_damage;        // defaults to SINGLE_SHOT dmg
    float           proj_speed;         // defaults to SINGLE_SHOT speed

    Uint8           curr_mode;          // current attack mode

    // movement bounds (keeps player within camera view)
    int             x_bound;            // from origin to leftmost side
    int             z_bound;            // from origin to topmost side

    // SINGLE_SHOT timing
    float           next_single_shot;

    // CHARGE_SHOT timing
    float           next_charged_shot;  // the next time for CHARGE_SHOT to be active
    float           charge_shot_delay;  // the small delay time after releasing a CHARGE_SHOT

    // player flags/checks
    Uint8           change_flag;        // flag for model switching
    Uint8           mid_roll;           // flag for player's barrel roll mechanic
    Uint8           roll;               // type of barrel roll
    Uint8           wave_flag;          // flag for making sure only one wave_shot is on-screen
    Uint8           nuke_flag;          // flag for making sure only one super_nuke is on-screen
    int             proj_count;         // current amount of projectiles fired
    Uint8           took_damage;        // flag for activating player_take_damage
    float           damage_taken;       // damage received from enemy
    Uint8           player_dead;        // flag for player death state

    // debug camera
    Uint8           freelook;           // debug camera
    Uint8           no_attack;          // flag for no attack;

}PlayerData;

Uint8 player_count;
Uint8 player_no_attack;

Entity* player_spawn();
void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity*  self);
void player_attack(Entity* self, PlayerData* data);
void player_take_damage(Entity* self, PlayerData* data);
void player_die(Entity* self);
void player_death(Entity* self);

#endif