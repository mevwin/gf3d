#include "simple_logger.h"
#include "gf2d_mouse.h"
#include "gfc_input.h"
#include "gfc_audio.h"
#include "gf2d_font.h"
#include "world.h"
#include "ui.h"
#include "level.h"
#include "player.h"
#include "enemy.h"

static WorldData* world;
static Entity* player;

void world_check_for_menu_input();
void start_menu_input_check(UIData* ui_data);
void update_time_checks(float curr_time);
void game_data_init_from_save();
void game_save();

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
	UIData* ui;
	int i;

	sounds = get_sound_data();
	level = get_level_data();
	ui = get_UI_data();

	if (!level || !sounds || !ui) return;

	if (world->current_state == WAVE_START) {
		level->wave_end = 0;

		if (gf2d_mouse_button_released(2)) {
			world->enemy_start = 1;
			if (level->enemy_count != 0)
				level->enemy_count = 0;

			level->game_start = CURRENT_TIME;

			world->current_state = IN_GAME;

			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
		else if (gfc_input_command_pressed("escape")) {
			world->current_state = PAUSE_MENU;
			world->last_state = WAVE_START;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
	}
	else if (world->current_state == PAUSE_MENU) {
		if (gfc_input_command_pressed("escape")) {
			if (level->wave_end)
				world->current_state = world->last_state;
			else {
				update_time_checks(CURRENT_TIME);
				world->current_state = world->last_state;
			}

			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
		else if (gf2d_mouse_button_released(0)) {
			if (gf2d_mouse_in_rect(ui->resume_block)) {
				if (level->wave_end)
					world->current_state = world->last_state;
				else {
					update_time_checks(CURRENT_TIME);
					world->current_state = world->last_state;
				}
				gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
			}
			else if (gf2d_mouse_in_rect(ui->quit_block)) {
				game_save();
				perk_list_close();
				full_level_reset();
				shop_reset();
				entity_despawn_all();
				entity_assets_close();
				world->current_state = START_MENU;
				world->player_spawned = 0;
				gfc_sound_play(get_sound_data()->cancel, 0, 1, -1, -1);
			}
		}
	}
	else if (world->current_state == SHOP) {
		if (gf2d_mouse_button_released(0) && gf2d_mouse_in_rect(ui->next_wave_block)) {
			empty_perk_list();
			world->current_state = WAVE_START;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
		else if (gfc_input_command_pressed("escape")) {
			world->current_state = PAUSE_MENU;
			world->last_state = SHOP;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
	}
	else if (world->current_state == GAME_OVER) {
		if (gf2d_mouse_button_released(0)) {
			if (gf2d_mouse_in_rect(ui->respawn_block)) {
				// TODO: add respawn function
			}
			else if (gf2d_mouse_in_rect(ui->g_quit_block)) {
				perk_list_close();
				full_level_reset();
				shop_reset();
				entity_despawn_all();
				entity_assets_close();
				world->current_state = START_MENU;
				world->player_spawned = 0;
				gfc_sound_play(get_sound_data()->cancel, 0, 1, -1, -1);
			}
		}
	}
	else if (world->current_state == WAVE_COMPLETED) {
		if (gfc_input_command_pressed("escape")) {
			world->current_state = PAUSE_MENU;
			world->last_state = WAVE_COMPLETED;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
		else if (gf2d_mouse_button_released(2)) {
			if (gf2d_mouse_in_rect(ui->stage_block1)) {
				// TODO: add level changing here
			}
			else if (gf2d_mouse_in_rect(ui->stage_block2)) {
				// TODO: add level changing here
			}
			else
				return;

			// create three new perks for shop display
			for (i = 0; i < 3; i++) {
				make_random_perk();
			}

			world->current_state = SHOP;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
	}
	else if (world->current_state == IN_GAME) {
		if (gfc_input_command_pressed("escape")) {
			world->pause_time = CURRENT_TIME;
			world->current_state = PAUSE_MENU;
			world->last_state = IN_GAME;
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
		}
	}
}

void start_menu_input_check(UIData* ui_data) {
	if (gf2d_mouse_button_released(0)) {
		if (gf2d_mouse_in_rect(ui_data->new_start_block)) {
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);

			level_begin();
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

			level_begin();
			player_assets_init();
			world->current_state = LOADING_SCREEN;

			if (world->player_assets_made) {
				gf2d_draw_rect_filled(gfc_rect(0, 0, RES.x, RES.y), GFC_COLOR_BLACK);
				
				world->enemy_start = 0;
				world->continue_from_save = 1;
				world->player_spawned = 1;
			}
		}
		else if (gf2d_mouse_in_rect(ui_data->previous_block)){
			//TODO: add this deliverable
		}
		else if (gf2d_mouse_in_rect(ui_data->s_quit_block)) {
			world->_done = 1;
		}
	}
}

/**
* Time checks to update while game is paused:
*	- player shot timings
*	- emper countdown
*/
void update_time_checks(float curr_time) {
	Entity* entityList, *enemy;
	PlayerData* p_data;
	EnemyData* emper;
	int i;
	float added_time;

	p_data = get_player_data();
	entityList = get_entityList();

	added_time = curr_time - world->pause_time;

	// update player timings
	p_data->next_shot += added_time;
	p_data->next_charged_shot += added_time;
	p_data->charge_shot_delay += added_time;

	if (get_level_data()->emper_flag) {
		for (i = 0; i < MAX_ENTITY; i++) {
			enemy = &entityList[i];
			if (enemy->entity_type != ENEMY) continue;

			emper = enemy->data;
			if (emper->enemy_type != EMPERS) continue;

			// update emper countdown
			emper->emper_attack_time += added_time;

			break;
		}
	}
}

void world_update(float fps) {
	PlayerData* p_data;
	UIData* ui;
	LevelData* level;
	char fps_string[30];

	level = get_level_data();
	ui = get_UI_data();

	if (!level | !ui) return;

	world_check_for_menu_input();

	// fps check
	//sprintf(fps_string, "fps: %f", fps);
	//gf2d_font_draw_line_tag(fps_string, FT_H5, GFC_COLOR_WHITE, gfc_vector2d(0, RES.y - 50));

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
				perk_list_init();
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
			pause_menu(ui->pause_menu, ui->pause_menu_data);
			gf2d_mouse_draw();

			break;

		case SHOP:
			shop_hud_draw();
			shop_think();
			gf2d_mouse_draw();

			// reset perk updates
			p_data = get_player_data();

			p_data->perk1->updated = 0;
			p_data->perk2->updated = 0;

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
			gf2d_mouse_draw();

			update_player_perks();
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
			update_player_perks();

			// hud draws
			enemy_hud_all();
			player_hud(p_data);

			// player death check
			if (p_data->player_dead) {
				entity_reset();
				level->total_game_time += CURRENT_TIME - level->game_start;
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
	sj_object_get_value_as_float(value, "total_game_time", &level->total_game_time);
	sj_object_get_value_as_uint32(value, "total_scrap", &level->total_scrap);

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
	if (level->wave_count == wave_check || level->wave_count == 1) {    // don't save if currently on starting wave from save
		slog("Game Not Saved");
		return;
	}

	data_entry->v.string = sj_value_to_json_string(sj_new_uint32(level->wave_count));
	
	data_entry = sj_object_get_value(value, "enemy_killed_total");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint32(level->enemy_killed_total));

	data_entry = sj_object_get_value(value, "total_game_time");
	data_entry->v.string = sj_value_to_json_string(sj_new_float(level->total_game_time));

	data_entry = sj_object_get_value(value, "total_scrap");
	data_entry->v.string = sj_value_to_json_string(sj_new_uint32(level->total_scrap));

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

	slog("Game Saved");
}

WorldData* get_world_data() {
	return world;
}