#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "entity.h"

typedef enum EnemyType_E{
	PEAS,		// SINGLE_SHOT
	CHARGERS,	// CHARGE_SHOT
	BOMBERS,	// trynamite-type enemy
	EMPERS,		// don't kill it in time, EMP that freezes ship
	FENCERS		// restrict player space
}EnemyType;

typedef enum EnemyMove_E{
	HORIZONTAL,
	DVD_LOGO,
	STATIONARY
}EnemyMove;

typedef struct EnemyData_S{
	// enemy stats
	EnemyType		enemy_type;			// type of enemy
	float           maxHealth;          // enemy's maximum health
	float           currHealth;         // enemy's current health
	float			base_damage;		// starting damage
	float			pea_speed;			// projectile speed

	// enemy movement
	EnemyMove		move_type;
	float           upspeed;			// z movement
	float           rigspeed;			// x movement
	
	// enemy bounds
	int             x_bound;			// horizontal bounds
	int             z_bound;			// vertical bounds
	int				dist_to_player;		// distance to player
	
	// enemy flags
	int				proj_count;			// amount of active projectiles
	Uint8			took_damage;		// have they taken damage?
	float			damage_taken;		// amount of damage received
	Uint8           damaged_type;		// type of damage received
	float			next_single_shot;	// next shot time
	Uint8			enemy_dead;			// is enemy health less than zero
	float			emper_attack_time;
	Uint8			emper_attack_active;

	// other
	GFC_Vector3D*	player_pos;			// pointer to player's current position
	GFC_Vector3D    spawn_pos;			// initial spawn position
	Uint8			missile_targeted;	// flag for player missile attack
	int				scrap_amount;
	int				item_type;			// pre-determined item type (-1 = random, 0 = none)

}EnemyData;

Entity* enemy_spawn(GFC_Vector3D* player_pos, EnemyType enemy_type, SJson* object, int item);

#endif