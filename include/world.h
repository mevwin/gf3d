#ifndef __WORLD_H__
#define __WORLD_H__

#include <SDL_stdinc.h>
#include "gfc_vector.h"

/**
* Purpose of this system is to manage the world state (not in just in gameplay)
* Manages:
*	- Menus
*	- Player Status (enemies killed, number of waves in, etc.)
*	- Assets (models and def/json files)
*/

typedef enum GameState_E{
	START_MENU,
	LOADING_SCREEN,
	PAUSE_MENU,
	SHOP,
	WAVE_START,
	WAVE_COMPLETED,
	NEXT_MAP,
	GAME_OVER,
	IN_GAME,
	NO_OPTION				// option for nothing
}GameState;

typedef struct WorldData_S {
	// world state 
	Uint8			player_assets_made;
	Uint8			enemy_assets_made;
	Uint8			item_assets_made;
	Uint8			level_assets_made;
	Uint8			player_spawned;
	Uint8			enemy_start;
	Uint8			_done;
	Uint8			continue_from_save;
	GameState		current_state;
	GameState		last_state;
	float			pause_time;

	// def files
	SJson*			player_init;
	const char*		player_save;

}WorldData;

void world_init();
void world_close();
void world_update();
WorldData* get_world_data();

#endif