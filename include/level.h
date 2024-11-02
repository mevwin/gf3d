#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "gfc_list.h"

#define ENEMY_MIN_LIMIT 5 // minimum limit amount of enemies on-screen
#define ENEMY_GOAL 20.0f
#define ASTEROID_MAX 20

typedef struct {
	Uint8			game_start;
	Uint8			enemy_start;
	Uint8			_done;

	Uint32			enemy_count;
	int				enemy_killed;
	Uint8			emper_flag;
	Uint8			fencer_flag;
	GFC_Vector3D	fencer_spawn;		// position of fencer attack region
	
	Uint32			wave_count;
	Uint8           in_shop;            // is player in the shop menu
	Uint8           paused;             // is the player pausing the game
	Uint8           wave_end;

	Model*			asteroid;
	GFC_List*		asteroid_list;

}LevelData;

void level_init();
void level_visuals();
void level_update(void* p, void* player_data);
void level_free();
LevelData* get_level_data();

#endif