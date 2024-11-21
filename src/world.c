#include "gf2d_mouse.h"
#include "gfc_input.h"
#include "gfc_audio.h"
#include "world.h"
#include "ui.h"
#include "level.h"
#include "player.h"

static WorldData* world;
static Entity* player;

void world_init() {
	world = gfc_allocate_array(sizeof(WorldData), 1);

	world->entity_assets_made = 0;
	world->level_assets_made = 0;
	world->enemy_start = 0;
	world->_done = 0;
	world->current_state = START_MENU;
	
	world->player_init = sj_load("def/player_init.json");

	atexit(world_close);
}

void world_close() {
	sj_free(world->player_init);
	free(world);
}

void world_check_for_menu_input() {
	SoundData* sounds;

	sounds = get_sound_data();

	if (world->current_state == IN_GAME || 
		world->current_state == WAVE_COMPLETED ||
		world->current_state == WAVE_START
		) { // currently in-game
		if (gfc_input_command_pressed("shop"))
			world->current_state = SHOP;
		else if (gfc_input_command_pressed("escape"))
			world->current_state = PAUSE_MENU;
	}
	else if (world->current_state == SHOP) {
		if (gfc_input_command_pressed("shop"))
			world->current_state = IN_GAME;
		else if (gfc_input_command_pressed("escape"))
			world->current_state = PAUSE_MENU;
	}
	else if (world->current_state == PAUSE_MENU) {
		if (gfc_input_command_pressed("shop"))
			world->current_state = SHOP;
		else if (gfc_input_command_pressed("escape"))
			world->current_state = IN_GAME;
	}
	else if (world->current_state == GAME_OVER) {
		if (gfc_input_command_pressed("escape")) 
			world->current_state = PAUSE_MENU;
		else if (gf2d_mouse_button_released(0)) {
			world->current_state = WAVE_START;
			player_respawn();
		}
	}
}

void world_update() {
	PlayerData* p_data;
	
	world_check_for_menu_input();

	switch (world->current_state){
		case START_MENU:
			start_menu();
			start_menu_think();
			gf2d_mouse_draw();
			if (world->player_spawned) {
				player = player_spawn();
				world->current_state = WAVE_START;
			}
			break;

		case PAUSE_MENU:
			pause_menu();
			pause_menu_think(world);
			gf2d_mouse_draw();
			break;

		case SHOP:
			shop_hud_draw();
			shop_think();
			gf2d_mouse_draw();
			break;

		case WAVE_START:
			wave_start();
			break;

		case WAVE_COMPLETED:
			wave_completed();
			break;

		case GAME_OVER:
			player_death_screen();
			gf2d_mouse_draw();
			break;

		case IN_GAME: // game plays here
			player = get_player();
			p_data = player->data;

			if (!p_data)
				return;

			// game updates
			entity_draw_all();
			entity_think_all();
			entity_update_all();
			level_update();

			// hud draws
			enemy_hud_all();
			player_hud(p_data);

			if (p_data->player_dead) {
				entity_reset();
				world->current_state = GAME_OVER;
			}

			break;
	}
	
}

WorldData* get_world_data() {
	return world;
}

/**
* procedural content generator
*	- provide a palette/framework for the content
*	- at what time frame should you spawn things?
*	- provide different cases/scenarios for certain things
*	- weigh the choices based on the world state
*/