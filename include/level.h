#ifndef __LEVEL_H__
#define __LEVEL_H__

#define ENEMY_MIN_LIMIT 5 // minimum limit amount of enemies on-screen
#define ASTEROID_MAX 20

/**
* Notes here (TODO)
*/

typedef enum ObjType_E {
	KILL_ENEMY,
	SURVIVE,
	BOSS
}ObjType;

typedef enum LevelType_E {
	ASTEROID_BELT,
	BLACK_HOLE,
	BOSS_STAGE,
	LAVA_WORLD,
	ICE_CAVERN
}LevelType;

typedef struct LevelData_S{
	// game stats
	float			last_powerup;		// time stamp of last active powerup
	int				enemy_count;		// enemies currently on-screen

	Uint8			emper_flag;
	Uint8			fencer_flag;
	GFC_Vector3D	fencer_spawn;		// position of fencer attack region
	Uint32			wave_count;
	Uint8			wave_end;
	Uint8			obj_complete;
	float			game_start;
	float			wave_end_time;
	float			total_game_time;
	Uint32			total_scrap;

	// level/objective type
	SJson*			level_base; 
	SJson*			level_def;
	char			name[20];
	LevelType		level_type;
	ObjType			obj_type;
	char			level_obj[30];
	Uint32			wave_goal;
	SJson*			curr_level;
	Uint8			flock_num;
	void*			background;
	GFC_List*		hazard_list;

		// objective 1: kill x enemies
	int				enemy_killed;
	int				enemy_goal;
	Uint32			enemy_killed_total;

		// objective 2: survive for x minutes
	float			survival_time;
	float			goal_timestamp;

		// objective 3: mini-boss
	// TODO

		// objective 4: collect x items
	// TODO

}LevelData;

void level_init();

void level_begin(Uint8 game_mode);
void level_load(Uint8 game_mode, Uint8 slot);
void level_load_enemy_flock(SJson* curr_level, void* p_data);

void new_wave_level_reset();
void full_level_reset();
void asteroid_init();
void asteroid_free();

void level_visuals_init();
void level_visuals_update();
void level_update();
void level_free();
LevelData* get_level_data();
SJson* get_current_level();

#endif