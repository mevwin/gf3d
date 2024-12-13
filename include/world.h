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

typedef enum SaveType_E {
	GAMESAVE,		// player quits mid-game, so save progress
	RUNSAVE			// player dies so save run for displaying
}SaveType;

// TODO: implement these modes
typedef enum GameMode_E {
	REGULAR,		// premade levels
	ENDLESS,			// procedurally generated
	PREV_DISPLAY
}GameMode;

typedef enum GameState_E{
	START_MENU,
	GAME_MODE_SEL,
	PREV_PREVIEW,
	PREVIOUS_RUN,
	LEVEL_EDITOR,
	LOADING_SCREEN,
	PAUSE_MENU,
	SHOP,
	WAVE_START,
	WAVE_COMPLETED,
	GAME_OVER,
	GAME_COMPLETED,
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

	// notifications
	Uint8			notif_init;
	Uint8			notif_flag;
	Uint8			notif_type;
	float			notification_time;      // timestamp to close notifcation popup

	GameMode		game_mode;
	GameState		current_state;
	GameState		last_state;
	float			pause_time;
	
	// dummy perk containers for previous runs
	void*			perk1; 
	void*			perk2;

	// def files
	SJson*			player_init;

}WorldData;

void world_init();
void world_close();
void world_update(float fps);
void game_data_init_from_save(SaveType type, SJson* json);
void game_save(SaveType save_type);
WorldData* get_world_data();
GFC_List* get_previous_runs();

#endif