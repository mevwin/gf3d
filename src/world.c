#include "gf2d_mouse.h"
#include "gfc_input.h"
#include "gfc_audio.h"
#include "world.h"
#include "ui.h"
#include "level.h"
#include "player.h"
#include "enemy.h"

static WorldData* world;
static Entity* player;

void world_init() {
	world = gfc_allocate_array(sizeof(WorldData), 1);

	world->entity_assets_made = 0;
	world->level_assets_made = 0;
	world->enemy_start = 0;
	world->_done = 0;
	world->current_state = START_MENU;
	world->last_state = NO_OPTION;
	
	world->player_init = sj_load("def/player_init.json");

	atexit(world_close);
}

void world_close() {
	sj_free(world->player_init);
	free(world);
}

void world_check_for_menu_input() {
	LevelData* level;
	SoundData* sounds;

	sounds = get_sound_data();
	level = get_level_data();

	if (!level || !sounds) return;

	if (world->current_state == WAVE_START) {
		level->wave_end = 0;

		if (gf2d_mouse_button_released(2)) {
			world->enemy_start = 1;
			world->current_state = IN_GAME;

			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
		else if (gfc_input_command_pressed("escape")) {
			world->current_state = PAUSE_MENU;
			world->last_state = WAVE_START;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
		else if (gfc_input_command_pressed("shop")) {
			world->current_state = SHOP;
			world->last_state = WAVE_START;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
	}
	else if (world->current_state == PAUSE_MENU) {
		if (gfc_input_command_pressed("escape")) {
			world->current_state = world->last_state;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
		else if (gfc_input_command_pressed("shop")) {
			world->current_state = SHOP;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
	}
	else if (world->current_state == SHOP) {
		if (gfc_input_command_pressed("shop")) {
			world->current_state = world->last_state;
			if (level->wave_end)
				world->current_state = WAVE_START;

			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
		else if (gfc_input_command_pressed("escape")) {
			world->current_state = PAUSE_MENU;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
	}
	else if (world->current_state == GAME_OVER) {
		// TODO: add player respawn function
		if (gfc_input_command_pressed("escape")) {
			world->current_state = PAUSE_MENU;
			world->last_state = GAME_OVER;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
	}
	else if (world->current_state == WAVE_COMPLETED) {
		if (gfc_input_command_pressed("escape")) {
			world->current_state = PAUSE_MENU;
			world->last_state = WAVE_COMPLETED;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
		else if (gfc_input_command_pressed("shop")) {
			world->current_state = SHOP;
			world->last_state = WAVE_START;	//always
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
	}
	else if (world->current_state == IN_GAME) {
		if (gfc_input_command_pressed("escape")) {
			world->current_state = PAUSE_MENU;
			world->last_state = IN_GAME;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
		else if (gfc_input_command_pressed("shop")) {
			world->current_state = SHOP;
			world->last_state = IN_GAME;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
	}
}

void world_update() {
	PlayerData* p_data;
	LevelData* level;
	
	level = get_level_data();

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
			entity_draw_all();
			entity_think_all();
			entity_update_all();
			level_update();

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

			// player death check
			if (p_data->player_dead) {
				entity_reset();
				world->current_state = GAME_OVER;
			}

			// enemy spawning (TODO: remove later)
			if (level->enemy_count < 2 && world->enemy_start)
				enemy_spawn(&player->position);

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