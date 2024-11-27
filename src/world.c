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

void world_check_for_menu_input();
void start_menu_input_check(UIData* ui_data);
void pause_menu_input_check(UIData* ui_data);

void world_init() {
	world = gfc_allocate_array(sizeof(WorldData), 1);

	world->player_assets_made = 0;
	world->enemy_assets_made = 0;
	world->item_assets_made = 0;
	world->level_assets_made = 0;
	world->enemy_start = 0;
	world->_done = 0;
	world->current_state = START_MENU;
	world->last_state = NO_OPTION;
	world->continue_from_save = 0;

	world->player_init = sj_load("def/player_init.def");

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
			if (level->enemy_count != 0)
				level->enemy_count = 0;

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
			if (level->wave_end)
				world->current_state = SHOP;
			else
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
			if (level->wave_end)
				world->current_state = WAVE_START;
			else
				world->current_state = world->last_state;

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

void start_menu_input_check(UIData* ui_data) {
	if (gf2d_mouse_button_released(0)) {
		if (gf2d_mouse_in_rect(ui_data->new_start_block)) {
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);

			player_assets_init();
			world->current_state = LOADING_SCREEN;

			if (world->player_assets_made) {
				gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), GFC_COLOR_BLACK);
				world->enemy_start = 0;
				world->player_spawned = 1;	
			}
		}
		else if (gf2d_mouse_in_rect(ui_data->continue_block)) {
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);

			player_assets_init();
			world->current_state = LOADING_SCREEN;

			if (world->player_assets_made) {
				gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), GFC_COLOR_BLACK);
				world->enemy_start = 0;
				world->continue_from_save = 1;
				world->player_spawned = 1;

			}
		}
		else if (gf2d_mouse_in_rect(ui_data->s_quit_block)) {
			world->_done = 1;
		}
	}
}

void pause_menu_input_check(UIData* ui_data) {
	if (gf2d_mouse_button_released(0)) {
		if (gf2d_mouse_in_rect(ui_data->resume_block)) {
			world->current_state = IN_GAME;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
		else if (gf2d_mouse_in_rect(ui_data->quit_block)) {
			game_save();
			full_level_reset();
			entity_despawn_all();
			entity_assets_close();
			world->current_state = START_MENU;
			world->player_spawned = 0;
			gfc_sound_play(get_sound_data()->cancel, 0, 1, -1, -1);
		}
	}
}

void world_update() {
	PlayerData* p_data;
	UIData* ui;
	LevelData* level;
	
	level = get_level_data();
	ui = get_UI_data();

	if (!level | !ui) return;

	world_check_for_menu_input();

	switch (world->current_state){
		case START_MENU:
			start_menu();
			start_menu_input_check(ui);
			gf2d_mouse_draw();

			break;

		case LOADING_SCREEN:
			if (!world->enemy_assets_made)
				enemy_assets_init();
			if (!world->item_assets_made && world->enemy_assets_made)
				item_assets_init();

			if (world->player_spawned && world->enemy_assets_made && world->item_assets_made) {
				player = player_spawn();

				if (world->continue_from_save) {
					game_data_init_from_save();
					world->continue_from_save = 0;
				}
				world->current_state = WAVE_START;
			}
			//else
				//world->current_state = START_MENU;

			break;

		case PAUSE_MENU:
			pause_menu();
			pause_menu_input_check(ui);
			gf2d_mouse_draw();
			break;

		case SHOP:
			shop_hud_draw();
			shop_think();
			gf2d_mouse_draw();
			break;

		case WAVE_START:
			//entity_draw_all();
			//entity_think_all();
			//entity_update_all();
			//level_update();

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
			if (level->enemy_count < 5 && world->enemy_start) {
				enemy_spawn(&player->position);
			}

			break;
	}	
}

void game_data_init_from_save() {
	LevelData* level;
	UIData* ui;
	SJson* save, * value;

	level = get_level_data();
	ui = get_UI_data();
	save = sj_load("def/player_save.def");

	value = sj_object_get_value(save, "level_data");
	sj_object_get_value_as_uint32(value, "wave_count", &level->wave_count);
	sj_object_get_value_as_uint32(value, "enemy_killed_total", &level->enemy_killed_total);

	value = sj_object_get_value(save, "upgrades");
	sj_object_get_value_as_uint8(value, "shields_check", &ui->shields_check);
	sj_object_get_value_as_uint8(value, "more_scrap_check", &ui->more_scrap_check);
	sj_object_get_value_as_uint8(value, "missiles_check", &ui->missiles_check);
	sj_object_get_value_as_uint8(value, "single_shot_check", &ui->single_shot_check);
	sj_object_get_value_as_uint8(value, "charge_shot_check", &ui->charge_shot_check);
	sj_object_get_value_as_uint8(value, "nuke_check", &ui->nuke_check);
	sj_object_get_value_as_uint8(value, "shields_count", &ui->shields_count);
	sj_object_get_value_as_uint8(value, "more_scrap_count", &ui->more_scrap_count);
	sj_object_get_value_as_uint8(value, "missiles_count", &ui->missiles_count);
	sj_object_get_value_as_uint8(value, "single_shot_count", &ui->single_shot_count);
	sj_object_get_value_as_uint8(value, "harge_shot_count", &ui->missiles_count);

	sj_free(save);
}

void game_save() {
	PlayerData* p_data;
	LevelData* level;
	UIData* ui;
	SJson* save, * value, * data_entry;
	char buffer[4];
	Uint32 wave_check;

	p_data = get_player_data();
	level = get_level_data();
	ui = get_UI_data();

	if (p_data->player_dead)
		return;

	save = sj_load("def/player_save_base.def");

	// level save
	value = sj_object_get_value(save, "level_data");

	data_entry = sj_object_get_value(value, "wave_count");
	sj_object_get_value_as_uint32(value, "wave_count", &wave_check);
	if (level->wave_count == wave_check)    // don't save if currently on starting wave from save
		return;

	data_entry->v.string = sj_value_to_json_string(sj_new_uint32(level->wave_count));
	
	data_entry = sj_object_get_value(value, "enemy_killed_total");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint32(level->enemy_killed_total));

	// player save
	value = sj_object_get_value(save, "player_data");
	data_entry = sj_object_get_value(value, "maxHealth");
	data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->maxHealth));

	data_entry = sj_object_get_value(value, "currHealth");
	data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->currHealth));

	data_entry = sj_object_get_value(value, "maxShield");
	data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->maxShield));

	data_entry = sj_object_get_value(value, "currShield");
	data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->currShield));

	data_entry = sj_object_get_value(value, "maxScrap");
	data_entry->v.string = sj_value_to_json_string(sj_new_int(p_data->maxScrap));

	data_entry = sj_object_get_value(value, "currScrap");
	data_entry->v.string = sj_value_to_json_string(sj_new_int(p_data->currScrap));

	data_entry = sj_object_get_value(value, "single_shot_bonus");
	data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->single_shot_bonus));

	data_entry = sj_object_get_value(value, "charge_shot_mult");
	data_entry->v.string = sj_value_to_json_string(sj_new_float(p_data->charge_shot_mult));

	data_entry = sj_object_get_value(value, "nuke_cost");
	data_entry->v.string = sj_value_to_json_string(sj_new_int(p_data->nuke_cost));

	// upgrades save
	value = sj_object_get_value(save, "upgrades");
	data_entry = sj_object_get_value(value, "shields_check");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->shields_check));

	data_entry = sj_object_get_value(value, "more_scrap_check");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->more_scrap_check));

	data_entry = sj_object_get_value(value, "missiles_check");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->missiles_check));

	data_entry = sj_object_get_value(value, "single_shot_check");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->single_shot_check));

	data_entry = sj_object_get_value(value, "charge_shot_check");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->charge_shot_check));

	data_entry = sj_object_get_value(value, "nuke_check");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->nuke_check));

	data_entry = sj_object_get_value(value, "shields_count");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->shields_count));

	data_entry = sj_object_get_value(value, "more_scrap_count");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->more_scrap_count));

	data_entry = sj_object_get_value(value, "missiles_count");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->missiles_count));

	data_entry = sj_object_get_value(value, "single_shot_count");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->single_shot_count));

	data_entry = sj_object_get_value(value, "charge_shot_count");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint8(ui->charge_shot_count));

	sj_save(save, "def/player_save.def");
	sj_free(save);
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