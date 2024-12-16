#ifndef __LEVEL_EDITOR_H__
#define __LEVEL_EDITOR_H__


/*
typedef struct EditorUI_S {
	Sprite*			editor_menu;
	Sprite*			level_prop;

	// sidebar
	GFC_Rect		new_asset;
	GFC_Rect		prev_asset;
	GFC_Rect		next_asset;
	GFC_Rect		level_prop_rect;
	GFC_Rect		save_level_rect;

	// trackbar
	GFC_Rect		trackbar;
	GFC_Rect		enemy_select;
	GFC_Color		enemy_color;
	GFC_Rect		hazard_select;
	GFC_Color		hazard_color;

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
*/
void editor_init();
void editor_ui(void* w_data);
void editor_close();
GFC_Vector2D get_asset_prev_location();

#endif