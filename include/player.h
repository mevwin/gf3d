#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "gfc_shape.h"
#include "entity.h"

typedef enum {
    SHIELDS,            // increase maxHealth
    SCRAP_HELD,         // increase maxScrap
    SINGLE_SHOT_UP,     // increase single_shot bonus damage
    CHARGE_SHOT_UP,     // increase charge_shot multipler
    MORE_MISSILES,      // increase max_missile
    NUKE_COST_DOWN      // decrease nuke_cost
}Upgrade_Type;

typedef struct {
    GFC_Vector3D    og_pos;             // container for keeping model in place due to constant rotation
    Entity*         reticle;        
    GFC_Color       shop_color;
    float           shop_color_hue;

    // player values
    float           upspeed;            // vertical speed
    float           rigspeed;           // horizontal speed

    GFC_List        inventory;          // list of items
    GFC_List        upgrades;           // list of upgrades currently active

    float           maxHealth;          // player's maximum health, must change when more shields are added
    float           currHealth;         // player's current health
    float           maxShield;          // player's current health
    float           currShield;         // player's current shield count
    float           total_health_bar;   // maxHealth + maxShield
    int             maxScrap;           // maximum amount of ship parts a player can hold
    int             currScrap;          // player's currrent amount of ship parts

    float           base_damage;        // defaults to SINGLE_SHOT dmg
    float           proj_speed;         // defaults to SINGLE_SHOT speed

    float           single_shot_bonus;  // bonus damage for single_shot from upgrades
    float           charge_shot_mult;   // charge shot multipler
    int             max_missile;        // maximum number of missiles a player can spawn per missile attack attempt
    int             nuke_cost;          // scrap cost of using SUPER_NUKE

    Uint8           curr_mode;          // current attack mode
    Uint8           active_item;

    // movement bounds (keeps player within camera view)
    int             x_bound;            // from origin to leftmost side
    int             z_bound;            // from origin to topmost side

    // shot timing
    float           next_shot;          // time til next projectile fire
    float           next_charged_shot;  // the next time for CHARGE_SHOT to be active
    float           charge_shot_delay;  // the small delay time after releasing a CHARGE_SHOT

    // player personal flags/checks
    Uint8           change_flag;        // flag for model switching
    Uint8           mid_roll;           // flag for player's barrel roll mechanic
    Uint8           roll;               // type of barrel roll

    Uint8           took_damage;        // flag for activating player_take_damage
    float           take_damage_timing; // amount of time player is in damage state for visuals
    float           damage_taken;       // damage received from enemy
    Uint8           damaged_type;       // type of enemy attack

    Uint8           player_dead;        // flag for player death state
    Uint8           in_shop;            // is player in the shop menu
    Uint8           paused;             // is the player pausing the game

    // player attack flags/checks
    int             proj_count;         // current amount of projectiles fired
    Uint8           wave_flag;          // flag for making sure only one wave_shot is on-screen
    Uint8           nuke_flag;          // flag for making sure only one super_nuke is on-screen
    int             missile_count;      // container for checking player's missile count
    Uint8           missile_spawn;      // missile only spawn if reticle is on enemy

    // player upgrade checks
    Uint8           shields_check;

    // debug camera
    Uint8           freelook;           // debug camera
    Uint8           player_no_attack;

}PlayerData;

Entity* player_spawn();
void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity*  self);
void player_attack(Entity* self, PlayerData* data);
void player_take_damage(Entity* self, PlayerData* data, float time);
void player_die(Entity* self);
void player_death(Entity* self);
void player_respawn(Entity* self);
void player_quit(Entity* self);

#endif