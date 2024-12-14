#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"
#include "perk.h"

#define NEXT_CHARGE_SHOT 1.0f
#define CHARGE_SHOT_DELAY 0.5f

typedef enum Player_Atk_Type_E{
    SINGLE_SHOT,
    CHARGE_SHOT,
    VORTEX,	
    MISSILE,	
    SUPER_NUKE
}Player_Atk_Type;

typedef struct PlayerData_S{
    // player values
    float           upspeed;            // current vertical speed
    float           upspeed_def;        // default vertical speed
    float           upspeed_slow;       // slowed vertical speed
    float           rigspeed;           // current horizontal speed
    float           rigspeed_def;       // default vertical speed
    float           rigspeed_slow;      // current vertical speed

    float           maxHealth;          // player's maximum health, must change when more shields are added
    float           currHealth;         // player's current health
    float           maxShield;          // player's current health
    float           currShield;         // player's current shield count
    float           total_health_bar;   // maxHealth + maxShield
    int             maxScrap;           // maximum amount of ship parts a player can hold
    int             currScrap;          // player's currrent amount of ship parts
    float           vortex_dur;         // player's current vortex duration
    float           vortex_max;         // max vortex duration

    Uint8           currMode;          // current attack mode
    float           base_damage;        // defaults to SINGLE_SHOT dmg
    float           proj_speed;         // defaults to SINGLE_SHOT speed
    float           vortex_damage;

    float           single_shot_bonus;  // bonus damage for single_shot from upgrades
    float           charge_shot_mult;   // charge shot multipler
    int             max_missile;        // maximum number of missiles a player can spawn per missile attack attempt
    float           missile_bonus;      // missile bonus damage
    int             nuke_cost;          // scrap cost of using SUPER_NUKE

    Uint8           active_item;        // current item in use
    float           item_duration;
    float           powerup_dur;

    Perk*           perk1;
    Perk*           perk2;

    // movement bounds (keeps player within camera view)
    int             x_bound;            // from origin to leftmost side
    int             z_bound;            // from origin to topmost side

    // player personal flags/checks
    Uint8           change_flag;        // flag for model switching
    Uint8           mid_roll;           // flag for player's barrel roll mechanic
    Uint8           roll;               // type of barrel roll

    Uint8           took_damage;        // flag for activating player_take_damage
    float           take_damage_timing; // amount of time player is in damage state for visuals
    float           damage_taken;       // damage received from enemy
    Uint8           damaged_type;       // type of enemy attack
    float           emp_time;
    Uint8           player_dead;        // flag for player death state          

    // player attack flags/checks
    int             proj_count;         // current amount of projectiles fired
    Uint8           nuke_flag;          // flag for making sure only one super_nuke is on-screen
    Uint8           vortex_flag;
    int             missile_count;      // container for checking player's missile count
    Uint8           missile_spawn;      // missile only spawn if reticle is on enemy

    // shot timing
    float           next_shot;          // time til next projectile fire
    float           next_charged_shot;  // the next time for CHARGE_SHOT to be active
    float           charge_shot_delay;  // the small delay time after releasing a CHARGE_SHOT

    // other
    GFC_Vector3D    og_pos;             // container for keeping model in place due to constant rotation
    Entity*         reticle;            // pointer to player reticle
    GFC_Vector3D*   player_pos;         // pointer for other entities to get player position

    // debug
    Uint8           freelook;           // debug camera
    Uint8           player_no_attack;   // flag to make enemies docile 
}PlayerData;

Entity* player_spawn();
void player_think(Entity* self);
void player_update(Entity* self);
void player_respawn();
void player_upgrade();
Entity* get_player();
PlayerData* get_player_data();
GFC_Primitive get_player_hurtbox();

#endif