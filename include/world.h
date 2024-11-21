#ifndef __WORLD_H__
#define __WORLD_H__

#include <SDL_stdinc.h>
#include "gfc_vector.h"
#include "simple_json.h"

/**
* Purpose of this system is to manage the world state (not in just in gameplay)
* Manages:
*	- Menus
*	- Player Status (enemies killed, number of waves in, etc.)
*	- Assets (models and def/json files)
*/

typedef enum GameState_E{
	START_MENU,
	PAUSE_MENU,
	SHOP,
	WAVE_START,
	WAVE_COMPLETED,
	GAME_OVER,
	IN_GAME				// NONE means game in progress
}GameState;

typedef struct WorldData_S {
	// world state checks
	Uint8			entity_assets_made;
	Uint8			level_assets_made;
	Uint8			player_spawned;
	Uint8			enemy_start;
	Uint8			_done;
	GameState		current_state;

	// def files
	SJson*			player_init;
	const char*		player_save;

}WorldData;

void world_init();
void world_close();

/**
* @brief check inputs that access menus
*/
void world_check_for_menu_input();
void world_update();
void game_data_init_from_save();
void game_save();
WorldData* get_world_data();

#endif