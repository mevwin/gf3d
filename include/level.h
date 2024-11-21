#ifndef __LEVEL_H__
#define __LEVEL_H__

#define ENEMY_MIN_LIMIT 5 // minimum limit amount of enemies on-screen
#define ENEMY_GOAL 20.0f
#define ASTEROID_MAX 20



/**
* Notes here (TODO)
*/


typedef struct {
	// game stats
	float			last_powerup;		// time stamp of last active powerup
	Uint32			enemy_count;
	Uint32			enemy_killed;
	Uint8			emper_flag;
	Uint8			fencer_flag;
	GFC_Vector3D	fencer_spawn;		// position of fencer attack region
	Uint32			wave_count;
	Uint8			wave_start;
	Uint8			wave_end;

	// level visuals
	Uint8			level_type;

}LevelData;

void level_init();

void level_reset();
void asteroid_init();
void asteroid_free();
void level_visuals();
void level_update();
void level_free();
LevelData* get_level_data();

#endif