#include "simple_logger.h"
#include "gfc_types.h"
#include "gfc_config.h"
#include "player_move.h"
#include "ui.h"
#include "world.h"

//include perk.h is in player.h

static GFC_List* perk_list;

void perk_list_init() {
	perk_list = gfc_list_new_size(3);

	atexit(perk_list_close);
}

void perk_list_close() {
	empty_perk_list();
	gfc_list_delete(perk_list);
}

void empty_perk_list() {
	gfc_list_foreach(perk_list, free);
	gfc_list_clear(perk_list);
}

Perk* copy_perk_data(Perk* perk) {
	Perk* player_perk;

	player_perk = gfc_allocate_array(sizeof(Perk), 1);

	player_perk->type = perk->type;
	player_perk->uses = perk->uses;
	player_perk->num_effect = perk->num_effect;
	strcpy(player_perk->name, perk->name);
	strcpy(player_perk->desc, perk->desc);
	player_perk->color = perk->color;

	perk->bought = 1;

	return player_perk;
}

Perk* create_dummy_perk() {
	Perk* perk;

	perk = gfc_allocate_array(sizeof(Perk), 1);

	perk->type = NO_PERK;

	perk->bought = 1;

	perk->updated = 0;

	return perk;
}

Perk* create_perk(PerkType type, Uint8 uses, int num_effect, const char* name, const char* desc, GFC_Color color){
	Perk* perk;
	WorldData* world;

	world = get_world_data();

	perk = gfc_allocate_array(sizeof(Perk), 1);
	if (!perk) {
		slog("failed to allocate perk");
		return;
	}

	perk->type = type;
	
	if (perk->type == NO_PERK || !name || !desc)
		return perk;

	perk->uses = uses;
	perk->num_effect = num_effect;

	if (world->current_state == PREV_PREVIEW) {
		strcpy(perk->name, name);
		strcpy(perk->desc, desc);
	}
	else {
		sprintf(perk->name, name);
		if (type == INVINCIBLE_ROLL)
			sprintf(perk->desc, desc, uses);
		else
			sprintf(perk->desc, desc, num_effect, uses);

	}

	perk->color = color;

	perk->bought = 0;

	return perk;
}

void make_random_perk() {
	SJson* data, *perk, *perk_array;
	UIData* ui;
	PerkType type;
	const char* name;
	const char* desc;
	int total_perks, uses, num_effect, low, high, index;
	GFC_Vector2D rolls;
	GFC_Color color;

	ui = get_UI_data();

	data = sj_load("def/perks.def");
	perk_array = sj_object_get_value(data, "perks");

	total_perks = perk_array->v.array->count;

	type = (PerkType) (1 + gfc_random_int(total_perks));

	index = (int) type;

	perk = sj_array_get_nth(perk_array, index - 1);

	if (!perk) {
		sj_free(data);
		return;
	}

	name = sj_object_get_value_as_string(perk, "name");
	desc = sj_object_get_value_as_string(perk, "desc");

	color = sj_value_as_color(sj_object_get_value(perk, "color"));

	sj_value_as_vector2d(sj_object_get_value(perk, "num_effect"), &rolls);
	low = (int) rolls.x;
	high = (int) rolls.y;

	num_effect = low + gfc_random_int(high - low);

	sj_value_as_vector2d(sj_object_get_value(perk, "uses"), &rolls);
	low = (int) rolls.x;
	high = (int) rolls.y;

	uses = low + gfc_random_int(high - low);

	sj_free(data);

	gfc_list_append(perk_list, create_perk(type, uses, num_effect, name, desc, color));

}

void update_perk_desc(Perk* perk) {
	SJson* data, *perk_data,*perk_array;
	char buffer[100];
	const char* desc;
	int index;

	data = sj_load("def/perks.def");
	perk_array = sj_object_get_value(data, "perks");

	index = (int) perk->type;
	perk_data = sj_array_get_nth(perk_array, index-1);

	desc = sj_object_get_value_as_string(perk_data, "desc");

	perk->uses--;

	if (perk->type == INVINCIBLE_ROLL)
		sprintf(buffer, desc, perk->uses);
	else
		sprintf(buffer, desc, perk->num_effect, perk->uses);


	strcpy(perk->desc, buffer);

	perk->updated = 1;

	//slog("perk uses updated");

	free(perk_data);
}

void update_player_perks() {
	Perk* perk1, * perk2, * dummy;
	PlayerData* p_data;
	WorldData* world;

	world = get_world_data();

	p_data = get_player_data();
	perk1 = p_data->perk1;
	perk2 = p_data->perk2;

	// update perk uses
	if (world->current_state == WAVE_COMPLETED) {
		if ((perk1->type == DMG_RESIST || perk1->type == PASSIVE_HEALS) && !perk1->updated) {
			update_perk_desc(perk1);
		}
		if ((perk2->type == DMG_RESIST || perk2->type == PASSIVE_HEALS) && !perk2->updated) {
			update_perk_desc(perk2);
		}
	}
	else if (world->current_state == IN_GAME) {
		if ((perk1->type == INVINCIBLE_ROLL || perk1->type == REFLECTOR_SHIELD) && !perk1->updated) {
			if (p_data->mid_roll && (p_data->roll == RIGHT || p_data->roll == LEFT) && perk1->type == INVINCIBLE_ROLL) {
				update_perk_desc(perk1);
			}
			else if (p_data->vortex_flag && perk1->type == REFLECTOR_SHIELD) {
				update_perk_desc(perk1);
			}
		}
		else {
			if (!p_data->mid_roll && perk1->type == INVINCIBLE_ROLL)
				perk1->updated = 0;
			else if (p_data->vortex_flag && perk1->type == REFLECTOR_SHIELD)
				perk1->updated = 0;
		}

		if ((perk2->type == INVINCIBLE_ROLL || perk2->type == REFLECTOR_SHIELD) && !perk2->updated) {
			if (p_data->mid_roll && (p_data->roll == RIGHT || p_data->roll == LEFT) && perk2->type == INVINCIBLE_ROLL) {
				update_perk_desc(perk2);
			}
			else if (p_data->vortex_flag && perk2->type == REFLECTOR_SHIELD) {
				update_perk_desc(perk2);
			}
		}
		else {
			if (!p_data->mid_roll && perk2->type == INVINCIBLE_ROLL)
				perk2->updated = 0;
			else if (p_data->vortex_flag && perk2->type == REFLECTOR_SHIELD)
				perk2->updated = 0;
		}
	}
	
	// check if perk usage is 0
	if (world->current_state == IN_GAME || world->current_state == WAVE_COMPLETED) {
		if (perk1->uses == 0 && perk1->type != NO_PERK) {
			dummy = p_data->perk1;
			p_data->perk1 = create_dummy_perk();
			free(dummy);

			//slog("perk ran out");
		}
		if (perk2->uses == 0 && perk2->type != NO_PERK) {
			dummy = p_data->perk2;
			p_data->perk2 = create_dummy_perk();
			free(dummy);

			//slog("perk ran out");
		}
	}
}

GFC_List* get_perk_list() {
	return perk_list;
}