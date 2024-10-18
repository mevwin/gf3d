#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "gfc_shape.h"
#include "entity.h"

typedef struct {
    // debug camera
    Uint8           freelook;

    GFC_Vector3D    og_pos;             // container for keeping model in place due to constant rotation
    Entity*         reticle;            // pointer to player reticle

    // player values
    float           upspeed;            // vertical speed
    float           rigspeed;           // horizontal speed

    GFC_List        inventory;          // list of items
    GFC_List        upgrades;           // list of upgrades currently active

    float           maxHealth;          // player's maximum health
    float           currHealth;         // player's current health
    float           maxShipParts;       // maximum amount of ship parts a player can hold
    float           currShipParts;      // player's currrent amount of ship parts

    // movement bounds (keeps player within camera view)
    int             x_bound;            // from origin to leftmost side
    int             z_bound;            // from origin to topmost side

    // CHARGE_SHOT timing
    float           next_charged_shot;  // the next time for CHARGE_SHOT to be active
    float           shot_delay;         // the small delay time after releasing a CHARGE_SHOT

    // player flags
    Uint8           change_flag;        // flag for model switching
    Uint8           mid_roll;           // flag for player's barrel roll mechanic
    int             roll;               // type of barrel roll
    int             curr_mode;          // current attack mode
    Uint8           wave_flag;
    Uint8           missile_flag;
    Uint8           nuke_flag;
    int             proj_count;

}PlayerData;

Entity* player_spawn(GFC_Vector3D position);
void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity*  self);
void player_attack(Entity* self, PlayerData* data);
void player_death(Entity* self);

#endif