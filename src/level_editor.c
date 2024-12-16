#include "simple_logger.h"
#include "gfc_audio.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf2d_mouse.h"
#include "gfc_input.h"
#include "gfc_config.h"
#include "world.h"
#include "level.h"
#include "player.h"
#include "enemy.h"
#include "item.h"
#include "notifications.h"
#include "level_editor.h"

typedef enum TrackMode_E {
	ENEMY_TRACK,
	HAZARD_TRACK
}TrackMode;

typedef struct EditorUI_S {
	Sprite*			editor_menu;
	Sprite*			level_prop;
	
	// sidebar
	GFC_Rect		new_asset;
	GFC_Rect		prev_asset;
	GFC_Rect		next_asset;
	GFC_Vector2D	asset_prev_location;
	GFC_Rect		level_prop_rect;
	GFC_Rect		save_level_rect;

	// trackbar
	GFC_Rect		trackbar;
	GFC_Rect		enemy_select;
	GFC_Color		enemy_color;
	GFC_Rect		hazard_select;
	GFC_Color		hazard_color;

	// asset properties
	Sprite*			enemy_prop;
	Sprite*			hazard_prop;
	GFC_Vector2D	prop_option;

	// level properties
	GFC_Vector2D	text_offset;
	GFC_Color		textbox_color;
	GFC_Rect		textbox;
	GFC_Rect		textbox_inc;
	GFC_Rect		textbox_dec;
	GFC_Rect		textbox_confirm;
	GFC_Rect		next_page;
	GFC_Rect		prev_page;
	GFC_Rect		exit_button;

}EditorUI;

typedef struct EditorManager_S {
	SJson*			editor_def;
	EditorUI*		ui;

	// flags and checks
	Uint8			level_type_selected;
	Uint8			obj_type_selected;
	Uint8			set_level_prop;
	Uint8			in_editor;
	Uint8			toggle_menu;

	int				page_offset;
	int				int_buffer;
	int				asset_index;
	TrackMode		track_mode;
	Uint8			asset_loaded;
	Uint8			asset_selected;
	Uint8			asset_prop;
	Uint8			asset_changing;
	Uint8			curr_flock;
	GFC_Vector2D	asset_prop_offset;
	GFC_Rect		asset_prop_rect;
	Entity*			selected_asset;
	Entity*			preview_asset;
	GFC_List*		enemy_flocks;
	GFC_List*		hazard_list;

}EditorManager;

static EditorManager editor = { 0 };

int select_num_effect(ObjType obj_type);
void move_asset(Entity* asset);
void level_save(LevelData* level);

void editor_init() {
	SJson *menu;
	GFC_Vector4D rect;

	editor.ui = gfc_allocate_array(sizeof(EditorUI), 1);
	editor.ui->editor_menu = gf2d_sprite_load_image("images/UI/level_editor/level_editor.png");
	editor.ui->level_prop = gf2d_sprite_load_image("images/UI/level_editor/level_prop.png");

	editor.editor_def = sj_load("menus/level_editor.menu");

	// sidebar
	menu = sj_object_get_value(editor.editor_def, "sidebar");
	
	sj_object_get_vector4d(sj_object_get_value(menu, "asset_select"), "new_asset", &rect);
	editor.ui->new_asset = gfc_rect_from_vector4(rect);

	sj_object_get_vector4d(sj_object_get_value(menu, "asset_select"), "prev_asset", &rect);
	editor.ui->prev_asset = gfc_rect_from_vector4(rect);

	sj_object_get_vector4d(sj_object_get_value(menu, "asset_select"), "next_asset", &rect);
	editor.ui->next_asset = gfc_rect_from_vector4(rect);

	sj_object_get_vector2d(sj_object_get_value(menu, "asset_select"), "asset_prev_location", &editor.ui->asset_prev_location);

	sj_object_get_vector4d(menu, "level_prop", &rect);
	editor.ui->level_prop_rect = gfc_rect_from_vector4(rect);

	sj_object_get_vector4d(menu, "save_level", &rect);
	editor.ui->save_level_rect = gfc_rect_from_vector4(rect);

	// trackbar
	menu = sj_object_get_value(editor.editor_def, "trackbar");

	sj_object_get_vector4d(menu, "trackbar_rect", &rect);
	editor.ui->trackbar = gfc_rect_from_vector4(rect);

	editor.ui->enemy_color = sj_object_get_color(sj_object_get_value(menu, "enemy"), "color");
	sj_object_get_vector4d(sj_object_get_value(menu, "enemy"), "select_rect", &rect);
	editor.ui->enemy_select = gfc_rect_from_vector4(rect);

	editor.ui->hazard_color = sj_object_get_color(sj_object_get_value(menu, "hazard"), "color");
	sj_object_get_vector4d(sj_object_get_value(menu, "hazard"), "select_rect", &rect);
	editor.ui->hazard_select = gfc_rect_from_vector4(rect);

	editor.ui->enemy_prop = gf2d_sprite_load_image("images/UI/level_editor/enemy_prop.png");
	sj_object_get_vector2d(menu, "prop_option", &editor.ui->prop_option);

	// level properties
	menu = sj_object_get_value(editor.editor_def, "level_prop");

	sj_object_get_vector2d(menu, "text_offset", &editor.ui->text_offset);

	editor.ui->textbox_color = sj_object_get_color(sj_object_get_value(menu, "textbox"), "color");

	sj_object_get_vector4d(sj_object_get_value(menu, "textbox"), "rect", &rect);
	editor.ui->textbox = gfc_rect_from_vector4(rect);

	editor.ui->textbox_dec = gfc_rect(
		editor.ui->textbox.x, editor.ui->textbox.y + editor.ui->textbox.h - 40.0f,
		100.0f, 40.0f);

	editor.ui->textbox_inc = gfc_rect(
		editor.ui->textbox.x + editor.ui->textbox.w - 100.0f, 
		editor.ui->textbox.y + editor.ui->textbox.h - 40.0f,
		100.0f, 40.0f);

	editor.ui->textbox_confirm = gfc_rect(
		editor.ui->textbox.x + (editor.ui->textbox.w/2.0f) - 50.0f,
		editor.ui->textbox.y + editor.ui->textbox.h - 40.0f,
		100.0f, 40.0f);

	sj_object_get_vector4d(menu, "next_page", &rect);
	editor.ui->next_page = gfc_rect_from_vector4(rect);

	sj_object_get_vector4d(menu, "prev_page", &rect);
	editor.ui->prev_page = gfc_rect_from_vector4(rect);

	sj_object_get_vector4d(menu, "exit_button", &rect);
	editor.ui->exit_button = gfc_rect_from_vector4(rect);

	editor.in_editor = 0;
	editor.set_level_prop = 0;
	editor.page_offset = 0;
	editor.level_type_selected = 0;
	editor.obj_type_selected = 0;
	editor.toggle_menu = 1;

	editor.int_buffer = 1;
	editor.asset_index = 0;
	editor.track_mode = ENEMY_TRACK;
	editor.asset_loaded = 0;
	editor.curr_flock = 0;

	editor.enemy_flocks = gfc_list_new();
	gfc_list_append(editor.enemy_flocks, gfc_list_new());

	atexit(editor_close);
}

void editor_ui(void* w_data) {
	SJson* data_array, *entry, *level_prop_data, *obj_data;
	WorldData* world;
	LevelData* level;
	Entity* asset;
	EnemyData* e_data;
	GFC_Rect obj_rect, flock_rect;
	GFC_Vector2D offset;
	GFC_Rect asset_change, asset_delete;
	GFC_Vector4D rect_vec;
	GFC_Color color;
	GFC_List* curr_flock;	//for ENEMY_FLOCK
	char buffer[40];
	int i, j;

	world = (WorldData*) w_data;
	level = get_level_data();

	if (world->notif_flag)
		notif_window(world->notif_type);
	

	if (!editor.in_editor) {
		gf2d_sprite_draw_image(editor.ui->level_prop, gfc_vector2d(0, 0));

		if (!editor.level_type_selected) {
			gf2d_font_draw_line_tag("SELECT LEVEL TYPE", FT_H2, GFC_COLOR_WHITE, editor.ui->text_offset);

			data_array = sj_object_get_value(level->level_base, "level_type");

			// showcase all available level types
			for (i = 0, j = editor.page_offset; i < 3; i++, j++) {
				entry = sj_array_get_nth(data_array, j);

				if (entry) {
					// initialize entry's option button data
					level_prop_data = sj_object_get_value(editor.editor_def, "level_prop");
					obj_data = sj_array_get_nth(sj_object_get_value(level_prop_data, "obj_rect"), i);
					
					sj_value_as_vector2d(sj_object_get_value(obj_data, "text_offset"), &offset);

					sj_value_as_vector4d(sj_object_get_value(obj_data, "rect"), &rect_vec);
					obj_rect = gfc_rect_from_vector4(rect_vec);

					// get entry's data
					strcpy(buffer, sj_object_get_value_as_string(entry, "name"));
					color = sj_object_get_color(entry, "color");

					// draw the button
					gf2d_draw_rect_filled(obj_rect, color);
					gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, offset);

					if (gf2d_mouse_button_released(0) && gf2d_mouse_in_rect(obj_rect)) {
						gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
						strcpy(level->name, buffer);
						level->level_type = j;
						editor.level_type_selected = 1;
						editor.page_offset = 0;
					}
				}
			}
		}
		else if (!editor.obj_type_selected && editor.level_type_selected) {
			gf2d_font_draw_line_tag("SELECT OBJECTIVE TYPE", FT_H2, GFC_COLOR_WHITE, editor.ui->text_offset);

			data_array = sj_object_get_value(level->level_base, "level_obj");

			for (i = 0, j = editor.page_offset; i < 3; i++, j++) {
				entry = sj_array_get_nth(data_array, j);

				if (entry) {
					// initialize entry's option button data
					level_prop_data = sj_object_get_value(editor.editor_def, "level_prop");
					obj_data = sj_array_get_nth(sj_object_get_value(level_prop_data, "obj_rect"), i);

					sj_value_as_vector2d(sj_object_get_value(obj_data, "text_offset"), &offset);

					sj_value_as_vector4d(sj_object_get_value(obj_data, "rect"), &rect_vec);
					obj_rect = gfc_rect_from_vector4(rect_vec);
					
					// get entry's data
					strcpy(buffer, sj_object_get_value_as_string(entry, "obj_desc"));
					color = sj_object_get_color(entry, "color");

					// draw the button
					gf2d_draw_rect_filled(obj_rect, color);
					gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, offset);

					if (gf2d_mouse_button_released(0) && gf2d_mouse_in_rect(obj_rect)) {
						gfc_sound_play(get_sound_data()->confirm, 0, 1, -1, -1);
						level->obj_type = j;
						strcpy(level->level_obj, buffer);
						editor.obj_type_selected = 1;
					}
				}
			}
		}
		else {
			// input level_goal
			if (select_num_effect(level->obj_type)) {
				switch (level->obj_type) {
					case KILL_ENEMY:
						level->enemy_goal = editor.int_buffer;
						sprintf(buffer, level->level_obj, level->enemy_goal);
						strcpy(level->level_obj, buffer);
						break;

					case SURVIVE:
						level->survival_time = editor.int_buffer * 60.0f; // convert to seconds
						sprintf(buffer, level->level_obj, editor.int_buffer);
						strcpy(level->level_obj, buffer);
						break;
				}
				editor.in_editor = 1;
				editor.int_buffer = -1;

				player_assets_init();
				if (world->player_assets_made) {
					world->enemy_start = 0;
					world->player_spawned = 1;
					world->last_state = LEVEL_EDITOR;
					world->current_state = LOADING_SCREEN;
				}

				return;
				//slog("%i", level->enemy_goal);
			}
		}

		// input checks (TODO)
		if (gf2d_mouse_button_released(0)) {
			if (gf2d_mouse_in_rect(editor.ui->next_page)) {

			}			
			else if (gf2d_mouse_in_rect(editor.ui->prev_page)) {

			}
			else if (gf2d_mouse_in_rect(editor.ui->exit_button)) {
				world->last_state = LEVEL_EDITOR;
				world->current_state = START_MENU;
			}
		}
	}
	else { // in editor
		if (editor.track_mode == ENEMY_TRACK)
			curr_flock = gfc_list_nth(editor.enemy_flocks, editor.curr_flock);
		else
			curr_flock = NULL;

		if (editor.toggle_menu) {
			gf2d_sprite_draw_image(editor.ui->editor_menu, gfc_vector2d(0, 0));

			// track mode draws
			switch (editor.track_mode) {
				case ENEMY_TRACK:
					gf2d_draw_rect_filled(editor.ui->trackbar, editor.ui->enemy_color);
					if (!editor.asset_loaded) {
						editor.preview_asset = enemy_spawn(NULL, editor.asset_index, NULL, 0);
						editor.preview_asset->editor_flag = 0;
						editor.asset_loaded = 1;
					}
					
					if (editor.enemy_flocks->count > 1) {
						gfc_rect_copy(flock_rect, editor.ui->trackbar);
						flock_rect.w /= ((float) editor.enemy_flocks->count);
						for (i = 0; i < editor.enemy_flocks->count; i++) {
							if (i == editor.curr_flock) {
								gf2d_draw_rect_filled(flock_rect, gfc_color8(0, 0, 244, 120));
							}
							else
								gf2d_draw_rect_filled(flock_rect, gfc_color8(i * 20, i * 20, i * 20, 120));
							
							sprintf(buffer, "F%i", i+1);
							gf2d_font_draw_text_wrap_tag(buffer, FT_H2, GFC_COLOR_WHITE, flock_rect);
							if (gf2d_mouse_in_rect(flock_rect) && gf2d_mouse_button_released(0)) {
								editor.curr_flock = i;
							}
							flock_rect.x += flock_rect.w;

						}
					}	

					break;
				case HAZARD_TRACK:
					gf2d_draw_rect_filled(editor.ui->trackbar, editor.ui->hazard_color);
					// TODO

					break;
			}
		}

		// display level obj
		gf2d_font_draw_line_tag(level->level_obj, FT_H2, GFC_COLOR_WHITE, gfc_vector2d(10, 10));

		if (editor.asset_changing) 
			select_num_effect(0);

		// input checks (TODO)
		if (gfc_input_command_released("escape")) {
			full_level_reset();
			entity_despawn_all();
			entity_assets_close();
			world->player_spawned = 0;
			world->last_state = LEVEL_EDITOR;
			world->current_state = START_MENU;
			gfc_sound_play(get_sound_data()->cancel, 0, 1, -1, -1);
			return;
		}
		else if (gfc_input_command_released("vortex")) {
			if (editor.toggle_menu) {
				editor.toggle_menu = 0;
				entity_free(editor.preview_asset);
				editor.asset_loaded = 0;
			}
			else
				editor.toggle_menu = 1;
		}
		else if (gf2d_mouse_button_released(2) && editor.toggle_menu) {
			if (gf2d_mouse_in_rect(editor.ui->enemy_select) && editor.track_mode == ENEMY_TRACK) {
				// delete current flock
				if (editor.enemy_flocks->count == 0)
					return;

				gfc_list_foreach(curr_flock, entity_free);
				gfc_list_clear(curr_flock);
				gfc_list_delete_data(editor.enemy_flocks, curr_flock);
				gfc_list_delete(curr_flock);
				editor.curr_flock = 0;
				return;
			}
		}
		else if (gf2d_mouse_button_released(0) && editor.toggle_menu) {
			if (gf2d_mouse_in_rect(editor.ui->enemy_select)) {
				if (editor.track_mode == HAZARD_TRACK) {
					editor.track_mode = ENEMY_TRACK;
					if (!editor.preview_asset->editor_flag)
						entity_free(editor.preview_asset);

					editor.asset_loaded = 0;
				}
				else {
					gfc_list_append(editor.enemy_flocks, gfc_list_new());
				}
			}
			else if (gf2d_mouse_in_rect(editor.ui->hazard_select)) {
				editor.track_mode = HAZARD_TRACK;
				if (!editor.preview_asset->editor_flag)
					entity_free(editor.preview_asset);

				editor.asset_loaded = 0;
			}
			else if (gf2d_mouse_in_rect(editor.ui->new_asset)) {
				//slog("new asset");
				//e_data = editor.curr_asset->data;
				//editor.curr_asset->position = gfc_vector3d_enemy_random_pos(e_data->x_bound, e_data->dist_to_player, e_data->z_bound);
				if (!editor.preview_asset) {
					slog("no preview asset");
					return;
				}
				editor.preview_asset->position.x = 0;
				editor.preview_asset->position.z = 10; 

				e_data = editor.preview_asset->data;

				offset = gfc_3DPos_to_2DPos(editor.preview_asset->position, e_data->x_bound, e_data->z_bound);

				editor.preview_asset->editor_rect.x = offset.x - (editor.preview_asset->editor_rect.w / 2.0f);
				editor.preview_asset->editor_rect.y = offset.y - (editor.preview_asset->editor_rect.h / 2.0f);
				editor.preview_asset->editor_flag = 1;

				gfc_list_append(curr_flock, editor.preview_asset);
				editor.asset_loaded = 0;
			}
			else if (gf2d_mouse_in_rect(editor.ui->next_asset)) {
				if (!editor.preview_asset->editor_flag)
					entity_free(editor.preview_asset);

				//slog("next asset");
				editor.asset_loaded = 0;
				editor.asset_index++;
				if (editor.track_mode == ENEMY_TRACK && editor.asset_index > FENCERS)
					editor.asset_index = 0;
			}
			else if (gf2d_mouse_in_rect(editor.ui->prev_asset)) {
				if (!editor.preview_asset->editor_flag)
					entity_free(editor.preview_asset);

				//slog("prev asset");
				editor.asset_loaded = 0;
				editor.asset_index--;
				if (editor.asset_index < 0) {
					if (editor.track_mode == ENEMY_TRACK)
						editor.asset_index = (int) FENCERS; // change later
					else
						editor.asset_index = 0;	// change later for hazards
				}
			}
			else if (gf2d_mouse_in_rect(editor.ui->level_prop_rect)) {
				world->change_level_prop = 1;
				full_level_reset();
				entity_despawn_all();
				entity_assets_close();
				world->player_spawned = 0;
				world->last_state = LEVEL_EDITOR;
				world->current_state = START_MENU;
				return;
			}
			else if (gf2d_mouse_in_rect(editor.ui->save_level_rect)) {
				if (level->enemy_count - 1 < level->enemy_goal && level->obj_type == KILL_ENEMY) {
					world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
					world->notif_flag = 1;
					world->notif_type = NOT_ENOUGH_ENEMIES;
					return;
				}

				level_save(level);
				full_level_reset();
				entity_despawn_all();
				entity_assets_close();
				world->player_spawned = 0;

				world->notification_time = CURRENT_TIME + NOTIF_TIME_MAX;
				world->notif_flag = 1;
				world->notif_type = LEVEL_SAVED;

				world->last_state = LEVEL_EDITOR;
				world->current_state = START_MENU;
			}
		}
		
		// moving enemy assets
		asset = NULL;
		if (editor.track_mode == ENEMY_TRACK) {
			if (!curr_flock)
				return;

			for (i = 0; i < curr_flock->count; i++) {
				if (curr_flock->count == 0)
					return;

				asset = (Entity*) gfc_list_nth(curr_flock, i);
				if (!asset)
					continue;

				gf2d_draw_rect(asset->editor_rect, GFC_COLOR_WHITE);

				if (gf2d_mouse_button_held(0) && gf2d_mouse_in_rect(asset->editor_rect) &&
					asset->editor_flag && !editor.asset_selected)
				{
					move_asset(asset);
					break;
				}
				else if (gf2d_mouse_button_released(2) && gf2d_mouse_in_rect(asset->editor_rect) &&
					asset->editor_flag && !editor.asset_selected)
				{
					editor.asset_selected = 1;
					editor.asset_prop = 1;
					editor.asset_prop_offset = gf2d_mouse_get_position();
					editor.asset_prop_rect = gfc_rect(editor.asset_prop_offset.x - 20.0f, 
													  editor.asset_prop_offset.y - 20.0f,
													  editor.ui->enemy_prop->frameWidth,
													  editor.ui->enemy_prop->frameHeight);
					editor.selected_asset = asset;
					break;
					//gf2d_draw_rect_filled(gfc_rect(), GFC_COLOR_RED);
				}
			}
			if (asset) {
				if (gf2d_mouse_in_rect(editor.asset_prop_rect) && editor.asset_selected && editor.asset_prop && editor.toggle_menu) {
					gf2d_sprite_draw(editor.ui->enemy_prop, editor.asset_prop_offset, NULL,
						NULL, NULL, NULL, NULL, NULL, NULL);

					gfc_rect_copy(asset_change, editor.asset_prop_rect);
					asset_change.h = editor.ui->prop_option.y;
					
					gfc_rect_copy(asset_delete, editor.asset_prop_rect);
					asset_delete.y += asset_delete.h - editor.ui->prop_option.y;
					asset_delete.h = editor.ui->prop_option.y;
					
					if (gf2d_mouse_in_rect(asset_change) && gf2d_mouse_button_released(0)) {
						editor.asset_changing = 1;
						editor.selected_asset = asset;
					}
					else if (gf2d_mouse_in_rect(asset_delete) && gf2d_mouse_button_released(0)) {
						editor.asset_selected = 0;
						editor.asset_prop = 0;
						gfc_list_delete_data(curr_flock, editor.selected_asset);
						entity_free(editor.selected_asset);
					}
				}
				else {
					editor.asset_selected = 0;
					editor.asset_prop = 0;
				}
			}
		}
		// moving hazard assets (TODO)
	}
}

void move_asset(Entity* asset) {
	EnemyData* e_data;
	GFC_Vector2D offset;

	// move around enemy
	e_data = asset->data;

	asset->position = gfc_2DPos_to_3DPos(gf2d_mouse_get_position(), e_data->x_bound, e_data->z_bound);
	asset->position.y = e_data->dist_to_player;
	editor.asset_selected = 1;

	// update editor_rect offsets
	gfc_vector2d_copy(offset, gf2d_mouse_get_position());
	asset->editor_rect.x = offset.x - (asset->editor_rect.w / 2.0f);
	asset->editor_rect.y = offset.y - (asset->editor_rect.h / 2.0f);
}

int select_num_effect(ObjType obj_type) {
	char buffer[50];
	EnemyData* e_data;

	gf2d_draw_rect_filled(editor.ui->textbox, editor.ui->textbox_color);
	gf2d_draw_rect_filled(editor.ui->textbox_inc, GFC_COLOR_DARKYELLOW);
	gf2d_font_draw_text_wrap_tag("+", FT_Normal, GFC_COLOR_WHITE, editor.ui->textbox_inc);

	gf2d_draw_rect_filled(editor.ui->textbox_dec, GFC_COLOR_RED);
	gf2d_font_draw_text_wrap_tag("-", FT_Normal, GFC_COLOR_WHITE, editor.ui->textbox_dec);

	gf2d_draw_rect_filled(editor.ui->textbox_confirm, GFC_COLOR_BLUE);
	gf2d_font_draw_text_wrap_tag("Confirm", FT_Normal, GFC_COLOR_WHITE, editor.ui->textbox_confirm);

	if (editor.asset_changing) {
		// choosing selected enemy's item
		switch (editor.int_buffer) {
			case -1:
				strcpy(buffer, "Select Item Type: RANDOM");
				break;

			case NONE:
				strcpy(buffer, "Select Item Type: NONE");
				break;

			case SCRAP:
				strcpy(buffer, "Select Item Type: HEALTH_PICKUP");
				break;

			case HEALTH_PICKUP:
				strcpy(buffer, "Select Item Type: HEALTH_PICKUP");
				break;

			case HAPPY_TRIGGER:
				strcpy(buffer, "Select Item Type: HAPPY_TRIGGER");
				break;

			case INVINCIBILITY:
				strcpy(buffer, "Select Item Type: INVINCIBILITY");
				break;
		}
	}
	else {
		// choosing objective goal
		switch (obj_type) {
			case KILL_ENEMY:
				sprintf(buffer, "Select Enemy Goal: %d Enemies", editor.int_buffer);
				break;

			case SURVIVE:
				sprintf(buffer, "Select Survival Time: %d Minutes", editor.int_buffer);
				break;

			case BOSS:
				return 1;
		}
	}

	gf2d_font_draw_text_wrap_tag(buffer, FT_Large, GFC_COLOR_WHITE, editor.ui->textbox);
	if (gf2d_mouse_button_pressed(0)) {
		if (gf2d_mouse_in_rect(editor.ui->textbox_inc)) {
			editor.int_buffer++;
			if (editor.asset_changing && editor.int_buffer > INVINCIBILITY) // selecting enemy item type
				editor.int_buffer = -1;
		}
		else if (gf2d_mouse_in_rect(editor.ui->textbox_dec)) {
			editor.int_buffer--;
			if (editor.int_buffer < 1) {
				if (editor.asset_changing) // selecting enemy item type
					editor.int_buffer = (int) INVINCIBILITY;
				else // selecting objective type
					editor.int_buffer = 1;
			}
		}

		if (editor.int_buffer > 100)
			editor.int_buffer = 100;
	}
	else if (gf2d_mouse_button_released(0) && gf2d_mouse_in_rect(editor.ui->textbox_confirm)) {
		editor.asset_selected = 0;
		editor.asset_prop = 0;
		if (editor.asset_changing) {
			e_data = editor.selected_asset->data;
			e_data->item_type = (ItemType)editor.int_buffer;
		}
		editor.asset_changing = 0;

		return 1;
	}
	return 0;
}

void level_save(LevelData* level) {
	SJson* level_ex, * level_ex_entry, * data_entry, * level_list;
	SJson* level_check, *flock_data, *enemy_spawn, *array, *new_level;
	GFC_List* enemy_flock;
	Entity* asset;
	EnemyData* e_data;
	char buffer[30];
	int i, j, k;


	level_ex = sj_load("levels/level_example.def");
	level_list = sj_load("levels/regular_levels.def");

	level_ex_entry = sj_object_get_value(level_ex, "level");

	data_entry = sj_object_get_value(level_ex_entry, "level_type");
	k = (int) level->level_type;
	data_entry->v.string = sj_new_int(k)->v.string;

	data_entry = sj_object_get_value(level_ex_entry, "obj_type");
	k = (int) level->obj_type;
	data_entry->v.string = sj_new_int(k)->v.string;

	data_entry = sj_object_get_value(level_ex_entry, "level_goal");
	switch (level->obj_type) {
		case KILL_ENEMY:
			data_entry->v.string = sj_new_int(level->enemy_goal)->v.string;
			break;
		case SURVIVE:
			data_entry->v.string = sj_new_float(level->survival_time)->v.string;
			break;
		default:
			data_entry->v.string = sj_new_int(0)->v.string;
	}
	
	data_entry = sj_object_get_value(level_ex_entry, "level_spawns");
	for (i = 0; i < editor.enemy_flocks->count; i++) {
		sprintf(buffer, "flock%d", i);
		enemy_flock = gfc_list_nth(editor.enemy_flocks, i);

		if (!enemy_flock)
			continue;

		flock_data = sj_array_new();
		for (j = 0; j < enemy_flock->count; j++) {
			enemy_spawn = sj_object_new();
			asset = gfc_list_nth(enemy_flock, j);
			e_data = asset->data;
			sj_object_insert(enemy_spawn, "enemy_type", sj_new_int(e_data->enemy_type));

			array = sj_array_new();
			sj_array_append(array, sj_new_float(asset->position.x));
			sj_array_append(array, sj_new_float(asset->position.z));
			sj_object_insert(enemy_spawn, "spawn", array);

			sj_object_insert(enemy_spawn, "item", sj_new_int(e_data->item_type));
			sj_array_append(flock_data, enemy_spawn);
		}
		sj_object_insert(data_entry, buffer, flock_data);
	}

	// bug fix for a dupe entry at the start of the array
	sj_object_delete_key(data_entry, "flock0");
	
	// TODO: add level hazard saving

	for (i = 0; i > -1; i++) {
		sprintf(buffer, "levels/level_list/level%d.def", i);
		//slog("%s", buffer);

		level_check = sj_load(buffer);

		if (!level_check) {
			sj_save(level_ex, buffer);
			new_level = sj_object_new();
			
			sj_object_insert(new_level, "level", sj_new_str(buffer));
			if (!sj_array_get_nth(sj_object_get_value(level_list, "level_list"), i)) {
				sj_array_append(sj_object_get_value(level_list, "level_list"), new_level);
				sj_save(level_list, "levels/regular_levels.def");
			}
			else
				sj_free(new_level);
			break;
		}
		else {
			sj_free(level_check);
			strcpy(buffer, "");
		}
	}

	sj_free(level_ex);
	sj_free(level_list);
}

GFC_Vector2D get_asset_prev_location() {
	return editor.ui->asset_prev_location;
}

void editor_close() {
	sj_free(editor.editor_def);
	gf2d_sprite_free(editor.ui->editor_menu);
	gf2d_sprite_free(editor.ui->level_prop);
	gf2d_sprite_free(editor.ui->enemy_prop);
	gfc_list_foreach(editor.enemy_flocks, gfc_list_delete);
	gfc_list_delete(editor.enemy_flocks);
	gfc_list_clear(editor.hazard_list);
	gfc_list_delete(editor.hazard_list);

	free(editor.ui);

	slog("editor closed");

	if (get_world_data()->_done)
		memset(&editor, 0, sizeof(EditorManager));
}