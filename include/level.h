#ifndef __LEVEL_H__
#define __LEVEL_H__

#define ENEMY_MIN_LIMIT 5 // minimum limit amount of enemies on-screen
#define ENEMY_GOAL 20
#define ASTEROID_MAX 20



/**
* Notes here (TODO)
*/

typedef enum ObjType_E {
	KILL_ENEMY,
	SURVIVE,
	COLLECT,
	BOSS
}ObjType;

typedef enum LevelType_E {
	ASTEROID_BELT,
	LAVA_WORLD,
	ICE_CAVERN,
	BLACK_HOLE
}LevelType;

typedef struct LevelData_S{
	// game stats
	float			last_powerup;		// time stamp of last active powerup
	Uint8			enemy_count;		// enemies currently on-screen

	Uint8			emper_flag;
	Uint8			fencer_flag;
	GFC_Vector3D	fencer_spawn;		// position of fencer attack region
	Uint32			wave_count;
	Uint8			wave_end;
	float			game_start;

	// level/objective type
	LevelType		level_type;
	ObjType			obj_type;
	char			level_obj[50];

		// objective 1: kill x enemies
	int				enemy_killed;
	int				enemy_goal;
	Uint32			enemy_killed_total;

		// objective 2: survive for x minutes
	float			survival_time;
	float			goal_timestamp;

		// objective 3: collect x items
	// TODO

		// objective 4: mini-boss
	// TODO

}LevelData;

void level_init();

void level_load();

void new_wave_level_reset();
void full_level_reset();
void asteroid_init();
void asteroid_free();
void level_visuals();
void level_update();
void level_free();
LevelData* get_level_data();

#endif