#include "simple_logger.h"
#include "simple_json_array.h"
#include "gf2d_mouse.h"
#include "gfc_input.h"
#include "gfc_audio.h"
#include "gf2d_font.h"
#include "gfc_config.h"
#include "world.h"
#include "ui.h"
#include "level.h"
#include "player.h"
#include "enemy.h"
#include "notifications.h"
#include "level_editor.h"

static WorldData* world;
static Entity* player;
static GFC_List* previous_runs_list;

void world_check_for_menu_input();
void start_menu_input_check(UIData* ui);
void update_time_checks(float curr_time);
int previous_runs_list_init();
void previous_runs_list_free();
void previous_runs_perks_init(SJson* run);
void game_save(SaveType save_type);

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
	world->notif_init = 0;
	world->notif_flag = 0;
	world->notif_type = 0;
	world->notification_time = 0;

	world->player_init = sj_load("def/player_init.def");

	atexit(world_close);
}

void world_close() {
	sj_free(world->player_init);
	free(world);
}

int previous_runs_list_init() {
	SJson* run;
	char buffer[30];
	int i;

	previous_runs_list = gfc_list_new();

	for (i = 1; i > 0; i++) {
		sprintf(buffer, "def/player_runs/run%d.def", i);

		run = sj_load(buffer);

		if (run)
			gfc_list_append(previous_runs_list, run);
		else {
			if (i == 1) // if no runs have been found
				return 0;
			else //if at least 1 run has been found, return true
				return 1;
		}
	}

	atexit(previous_runs_list_free);
}

void previous_runs_list_free() {
	gfc_list_foreach(previous_runs_list, sj_free);
	gfc_list_clear(previous_runs_list);
	gfc_list_delete(previous_runs_list);
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
			if (level->enemy_count < 0)
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
				game_save(GAMESAVE);
				perk_list_close();
				full_level_reset();
				shop_reset();
				entity_despawn_all();
				entity_assets_close();
				world->player_spawned = 0;
				world->current_state = START_MENU;
				gfc_sound_play(get_sound_data()->cancel, 0, 1, -1, -1);
			}
		}
	}
	else if (world->current_state == SHOP) {
		if (gf2d_mouse_button_released(0) && gf2d_mouse_in_rect(ui->next_wave_block)) {
			empty_perk_list();
			level_load_enemy_flock(level->curr_level, player->data);
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
				world->player_spawned = 0;
				gfc_sound_play(get_sound_data()->cancel, 0, 1, -1, -1);
				world->current_state = START_MENU;
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
			if (gf2d_mouse_in_rect(ui->stage_block1) && world->game_mode == ENDLESS) {
				// TODO: add level changing here
			}
			else if (gf2d_mouse_in_rect(ui->stage_block2) && world->game_mode == ENDLESS) {
				// TODO: add level changing here
			}
			else if (gf2d_mouse_in_rect(ui->nextwave_block) && world->game_mode == REGULAR) {
				level_load(world->game_mode, 0); // load next level
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
	else if (world->current_state == PREV_PREVIEW) {
		if (gf2d_mouse_button_released(0)) {
			if (gf2d_mouse_in_rect(ui->exit_block)) {
				gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);

				previous_runs_list_free();
				ui->preview_page_offset = 0;
				world->current_state = START_MENU;
			}
			else if (gf2d_mouse_in_rect(ui->next_block)) {
				gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);

				if (previous_runs_list->count > ui->preview_page_offset){
					ui->preview_page_offset += 3;
					if (ui->preview_page_offset > previous_runs_list->count)
						ui->preview_page_offset -= 3;
				}
				//else do nothing
			}
			else if (gf2d_mouse_in_rect(ui->prev_block)) {
				gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);

				if (ui->preview_page_offset - 3 >= 0)
					ui->preview_page_offset -= 3;
				else
					ui->preview_page_offset = 0;
			}
		}
	}
	else if (world->current_state == PREVIOUS_RUN) {
		if (gf2d_mouse_button_released(0) && gf2d_mouse_in_rect(ui->return_block)) {
			gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);

			free(world->perk1);
			free(world->perk2);
			full_level_reset();
			shop_reset();
			world->current_state = PREV_PREVIEW;
		}
	}
	else if (world->current_state == GAME_COMPLETED) {
		if (gf2d_mouse_button_released(0) && gf2d_mouse_in_rect(ui->return_block)) {
			perk_list_close();
			full_level_reset();
			shop_reset();
			entity_despawn_all();
			entity_assets_close();
			world->player_spawned = 0;
			gfc_sound_play(get_sound_data()->cancel, 0, 1, -1, -1);
			world->current_state = START_MENU;
		}
	}
}

void start_menu_input_check(UIData* ui) {
	if (gf2d_mouse_button_released(0)) {
		if (world->current_state == START_MENU) {
			if (gf2d_mouse_in_rect(ui->new_start_block)) {
				gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
				world->current_state = GAME_MODE_SEL;
			}
			else if (gf2d_mouse_in_rect(ui->continue_block)) {
				gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);

				/*
				level_begin(ENDLESS);
				player_assets_init();

				if (world->player_assets_made) {

					world->enemy_start = 0;
					world->continue_from_save = 1;
					world->player_spawned = 1;
					world->current_state = LOADING_SCREEN;
				}
				*/
			}
			else if (gf2d_mouse_in_rect(ui->editor_block)) {
				gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
				editor_init();
				world->current_state = LEVEL_EDITOR;
			}
			else if (gf2d_mouse_in_rect(ui->previous_block)) {

				if (previous_runs_list_init())
					world->current_state = PREV_PREVIEW;
				else {
					gfc_list_delete(previous_runs_list);
					world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
					world->notif_flag = 1;
					world->notif_type = NO_RUNS;
				}

			}
			else if (gf2d_mouse_in_rect(ui->s_quit_block)) {
				world->_done = 1;
			}
		}
		else if (world->current_state == GAME_MODE_SEL) {
			if (gf2d_mouse_in_rect(ui->regular_block)) {
				gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);

				world->game_mode = REGULAR;
				level_begin(REGULAR);
				player_assets_init();

				if (world->player_assets_made) {

					world->enemy_start = 0;
					world->player_spawned = 1;
					world->current_state = LOADING_SCREEN;
				}
			}
			else if (gf2d_mouse_in_rect(ui->endless_block)) {
				/*
				gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);

				world->game_mode = ENDLESS;
				level_begin(ENDLESS);
				player_assets_init();

				if (world->player_assets_made) {

					world->enemy_start = 0;
					world->player_spawned = 1;
					world->current_state = LOADING_SCREEN;
				}
				*/
			}
			else if (gf2d_mouse_in_rect(ui->s_exit_block)) {
				world->current_state = START_MENU;
			}
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

	if (world->current_state != START_MENU)
		world_check_for_menu_input();

	// fps check
	//sprintf(fps_string, "fps: %f", fps);
	//gf2d_font_draw_line_tag(fps_string, FT_H5, GFC_COLOR_WHITE, gfc_vector2d(0, RES.y - 50));

	switch (world->current_state){
		case START_MENU:
			start_menu(world->current_state);
			start_menu_input_check(ui);
			gf2d_mouse_draw();

			if (world->last_state == LEVEL_EDITOR) {
				world->last_state = NO_OPTION;
				editor_close();
			}

			break;

		case GAME_MODE_SEL:
			start_menu(world->current_state);
			start_menu_input_check(ui);
			gf2d_mouse_draw();

			break;

		case PREV_PREVIEW:
			preview_runs();
			gf2d_mouse_draw();

			break;

		case PREVIOUS_RUN:
			display_previous_run();
			gf2d_mouse_draw();

			break;

		case LEVEL_EDITOR:
			editor_ui(world);
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
					game_data_init_from_save(GAMESAVE, NULL);
					world->continue_from_save = 0;
				}
				// load level's initial enemies
				level_load_enemy_flock(level->curr_level, player->data);

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
			wave_completed(world->game_mode);
			gf2d_mouse_draw();

			break;

		case GAME_OVER:
			player_death_screen(ui->game_over, ui->game_over_data);
			gf2d_mouse_draw();

			break;

		case GAME_COMPLETED:
			game_complete();
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

			break;
	}	
}

void previous_runs_perks_init(SJson* run) {
	SJson* *data_entry, *perk_entry;
	PerkType perk_type;
	const char* name;
	const char* desc;
	int perk_type_int, uses, num_effect;
	GFC_Color color;

	data_entry = sj_object_get_value(run, "player_data");
	perk_entry = sj_object_get_value(data_entry, "perk1");

	sj_object_get_value_as_int(perk_entry, "type", &perk_type_int);
	perk_type = (PerkType) perk_type_int;
	if (perk_type != NO_PERK) {
		sj_object_get_value_as_int(perk_entry, "uses", &uses);
		sj_object_get_value_as_int(perk_entry, "num_effect", &num_effect);
		name = sj_object_get_value_as_string(perk_entry, "name");
		desc = sj_object_get_value_as_string(perk_entry, "desc");
		color = sj_object_get_color(perk_entry, "color");
		world->perk1 = create_perk(perk_type, uses, num_effect, name, desc, color);
	}
	else
		world->perk1 = create_dummy_perk();

	perk_entry = sj_object_get_value(data_entry, "perk2");

	sj_object_get_value_as_int(perk_entry, "type", &perk_type_int);
	perk_type = (PerkType) perk_type_int;
	if (perk_type != NO_PERK) {
		sj_object_get_value_as_int(perk_entry, "uses", &uses);
		sj_object_get_value_as_int(perk_entry, "num_effect", &num_effect);
		name = sj_object_get_value_as_string(perk_entry, "name");
		desc = sj_object_get_value_as_string(perk_entry, "desc");
		color = sj_object_get_color(perk_entry, "color");
		world->perk2 = create_perk(perk_type, uses, num_effect, name, desc, color);
	}
	else
		world->perk2 = create_dummy_perk();

}

void game_data_init_from_save(SaveType type, SJson* json) {
	LevelData* level;
	UIData* ui;
	SJson* save, * value;
	int i;

	level = get_level_data();
	ui = get_UI_data();

	if (type == GAMESAVE)
		save = sj_load("def/player_save.def");
	else if (type == RUNSAVE)
		save = json;
	else
		save = NULL;

	if (!save)
		return;

	value = sj_object_get_value(save, "level_data");
	sj_object_get_value_as_uint32(value, "wave_count", &level->wave_count);
	sj_object_get_value_as_uint32(value, "enemy_killed_total", &level->enemy_killed_total);
	sj_object_get_value_as_float(value, "total_game_time", &level->total_game_time);
	sj_object_get_value_as_uint32(value, "total_scrap", &level->total_scrap);

	sj_object_get_value_as_int(value, "level_type", &i);
	level->level_type = (LevelType) i;

	sj_object_get_value_as_int(value, "obj_type", &i);
	level->obj_type = (ObjType) i;

	switch (level->obj_type) {
		case KILL_ENEMY:
			sj_object_get_value_as_int(value, "level_goal", &level->enemy_goal);
			break;

		case SURVIVE:
			sj_object_get_value_as_float(value, "level_goal", &level->survival_time);
			break;
	}
	strcpy(level->level_obj, sj_object_get_value_as_string(value, "level_obj"));

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

	if (type == GAMESAVE)
		sj_free(save);
	else if (type == RUNSAVE) {
		previous_runs_perks_init(json);
		world->current_state = PREVIOUS_RUN;
	}
}

void game_save(SaveType save_type) {
	PlayerData* p_data;
	LevelData* level;
	UIData* ui;
	SJson* save, *save_check, *value, *data_entry, *perk_entry, *color;
	Uint32 wave_check;
	int i;
	char buffer[30];

	p_data = get_player_data();
	level = get_level_data();
	ui = get_UI_data();

	save = sj_load("def/player_save_base.def");

	// level save
	value = sj_object_get_value(save, "level_data");

	data_entry = sj_object_get_value(value, "wave_count");
	sj_object_get_value_as_uint32(value, "wave_count", &wave_check);
	if (save_type == GAMESAVE && (level->wave_count == wave_check || level->wave_count == 1)) {    // don't save if currently on starting wave from save
		slog("Game Not Saved");
		return;
	}

	data_entry->v.string = sj_new_uint32(level->wave_count)->v.string;
	
	data_entry = sj_object_get_value(value, "enemy_killed_total");
	data_entry->v.string = sj_new_uint32(level->enemy_killed_total)->v.string;

	data_entry = sj_object_get_value(value, "total_game_time");
	data_entry->v.string = sj_new_float(level->total_game_time)->v.string;

	data_entry = sj_object_get_value(value, "total_scrap");
	data_entry->v.string = sj_new_uint32(level->total_scrap)->v.string;

	data_entry = sj_object_get_value(value, "level_type");
	data_entry->v.string = sj_new_int(level->level_type)->v.string;

	data_entry = sj_object_get_value(value, "obj_type");
	data_entry->v.string = sj_new_int(level->obj_type)->v.string;

	data_entry = sj_object_get_value(value, "level_goal");
	switch (level->obj_type) {
		case KILL_ENEMY:
			data_entry->v.string = sj_new_int(level->enemy_goal)->v.string;
			break;

		case SURVIVE:
			data_entry->v.string = sj_new_float(level->survival_time)->v.string;
			break;

		case BOSS:
			data_entry->v.string = sj_new_int(0)->v.string;
			break;
	}

	data_entry = sj_object_get_value(value, "level_obj");
	data_entry->v.string = sj_new_str(level->level_obj)->v.string;

	// player save
	value = sj_object_get_value(save, "player_data");
	data_entry = sj_object_get_value(value, "maxHealth");
	data_entry->v.string = sj_new_float(p_data->maxHealth)->v.string;

	data_entry = sj_object_get_value(value, "currHealth");
	data_entry->v.string = sj_new_float(p_data->currHealth)->v.string;

	data_entry = sj_object_get_value(value, "maxShield");
	data_entry->v.string = sj_new_float(p_data->maxShield)->v.string;

	data_entry = sj_object_get_value(value, "currShield");
	data_entry->v.string = sj_new_float(p_data->currShield)->v.string;

	data_entry = sj_object_get_value(value, "maxScrap");
	data_entry->v.string = sj_new_int(p_data->maxScrap)->v.string;

	data_entry = sj_object_get_value(value, "currScrap");
	data_entry->v.string = sj_new_int(p_data->currScrap)->v.string;

	data_entry = sj_object_get_value(value, "single_shot_bonus");
	data_entry->v.string = sj_new_float(p_data->single_shot_bonus)->v.string;

	data_entry = sj_object_get_value(value, "charge_shot_mult");
	data_entry->v.string = sj_new_float(p_data->charge_shot_mult)->v.string;

	data_entry = sj_object_get_value(value, "nuke_cost");
	data_entry->v.string = sj_new_int(p_data->nuke_cost)->v.string;

		// perks
	if (p_data->perk1->type != NO_PERK) {
		data_entry = sj_object_get_value(value, "perk1");
		perk_entry = sj_object_get_value(data_entry, "type");
		
		i = (int) p_data->perk1->type;
		perk_entry->v.string = sj_new_int(i)->v.string;

		perk_entry = sj_object_get_value(data_entry, "name");
		perk_entry->v.string = sj_new_str(p_data->perk1->name)->v.string;

		perk_entry = sj_object_get_value(data_entry, "desc");
		perk_entry->v.string = sj_new_str(p_data->perk1->desc)->v.string;

		perk_entry = sj_object_get_value(data_entry, "num_effect");
		perk_entry->v.string = sj_new_int(p_data->perk1->type)->v.string;

		perk_entry = sj_object_get_value(data_entry, "uses");
		perk_entry->v.string = sj_new_int(p_data->perk1->uses)->v.string;

		perk_entry = sj_object_get_value(data_entry, "color");
		color = sj_array_new();

		sj_array_append(color, sj_new_float(p_data->perk1->color.r));
		sj_array_append(color, sj_new_float(p_data->perk1->color.g));
		sj_array_append(color, sj_new_float(p_data->perk1->color.b));
		sj_array_append(color, sj_new_float(p_data->perk1->color.a));

		perk_entry->v.array = color->v.array;
	}

	if (p_data->perk2->type != NO_PERK) {
		data_entry = sj_object_get_value(value, "perk2");
		perk_entry = sj_object_get_value(data_entry, "type");

		i = (int) p_data->perk2->type;
		perk_entry->v.string = sj_new_int(i)->v.string;

		perk_entry = sj_object_get_value(data_entry, "name");
		perk_entry->v.string = sj_new_str(p_data->perk2->name)->v.string;

		perk_entry = sj_object_get_value(data_entry, "desc");
		perk_entry->v.string = sj_new_str(p_data->perk2->desc)->v.string;

		perk_entry = sj_object_get_value(data_entry, "num_effect");
		perk_entry->v.string = sj_new_int(p_data->perk2->type)->v.string;

		perk_entry = sj_object_get_value(data_entry, "uses");
		perk_entry->v.string = sj_new_int(p_data->perk2->uses)->v.string;

		perk_entry = sj_object_get_value(data_entry, "color");
		color = sj_array_new();

		sj_array_append(color, sj_new_float(p_data->perk2->color.r));
		sj_array_append(color, sj_new_float(p_data->perk2->color.g));
		sj_array_append(color, sj_new_float(p_data->perk2->color.b));
		sj_array_append(color, sj_new_float(p_data->perk2->color.a));

		perk_entry->v.array = color->v.array;
	}

	// upgrades save
	value = sj_object_get_value(save, "upgrades");
	data_entry = sj_object_get_value(value, "shields_check");
	data_entry->v.string = sj_new_uint8(ui->shields_check)->v.string;

	data_entry = sj_object_get_value(value, "more_scrap_check");
	data_entry->v.string = sj_new_uint8(ui->more_scrap_check)->v.string;

	data_entry = sj_object_get_value(value, "missiles_check");
	data_entry->v.string = sj_new_uint8(ui->missiles_check)->v.string;

	data_entry = sj_object_get_value(value, "single_shot_check");
	data_entry->v.string = sj_new_uint8(ui->single_shot_check)->v.string;

	data_entry = sj_object_get_value(value, "charge_shot_check");
	data_entry->v.string = sj_new_uint8(ui->charge_shot_check)->v.string;

	data_entry = sj_object_get_value(value, "nuke_check");
	data_entry->v.string = sj_new_uint8(ui->nuke_check)->v.string;

	data_entry = sj_object_get_value(value, "shields_count");
	data_entry->v.string = sj_new_uint8(ui->shields_count)->v.string;

	data_entry = sj_object_get_value(value, "more_scrap_count");
	data_entry->v.string = sj_new_uint8(ui->more_scrap_count)->v.string;

	data_entry = sj_object_get_value(value, "missiles_count");
	data_entry->v.string = sj_new_uint8(ui->missiles_count)->v.string;

	data_entry = sj_object_get_value(value, "single_shot_count");
	data_entry->v.string = sj_new_uint8(ui->single_shot_count)->v.string;

	data_entry = sj_object_get_value(value, "charge_shot_count");
	data_entry->v.string = sj_new_uint8(ui->charge_shot_count)->v.string;

	if (save_type == GAMESAVE) {
		sj_save(save, "def/player_save.def");
		slog("Game Saved");
	}
	else if (save_type == RUNSAVE) {
		for (i = 1; i > 0; i++) {
			sprintf(buffer, "def/player_runs/run%d.def", i);
			//slog("%s", buffer);

			save_check = sj_load(buffer);

			if (!save_check) {
				sj_save(save, buffer);
				slog("Run saved");
				break;
			}
			else {
				sj_free(save_check);
				strcpy(buffer, "");
			}
		}
	}
	sj_free(save);
	
}

WorldData* get_world_data() {
	return world;
}

GFC_List* get_previous_runs() {
	return previous_runs_list;
}