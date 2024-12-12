#ifndef __PERK_H__
#define __PERK_H__

#include <SDL_stdinc.h>
#include "gfc_color.h"

typedef enum PerkType_E {
	NO_PERK,			// (X) default, placeholder value
	DMG_RESIST,			// (X) implement when player takes damage
	PASSIVE_HEALS,		// (X) implement in player_update
	INVINCIBLE_ROLL,	// (X) implement in player_move
	REFLECTOR_SHIELD	// (X) implement in projectile
}PerkType;

typedef struct Perk_S {
	PerkType		type;
	int				uses;
	int				num_effect;
	char			name[50];
	char			desc[100];
	GFC_Color		color;
	Uint8			bought;
	Uint8			updated;
}Perk;

Perk* create_dummy_perk();

Perk* create_perk(PerkType type, Uint8 uses, int num_effect, const char* name, const char* desc, GFC_Color color);

/**
* @brief initialize a gfc_list for holding perks
*/
void perk_list_init();

/**
* @brief close perk gfc_list when entering back into START_MENU
*/
void perk_list_close();

/**
* @brief used to clear all unbought perks when going from SHOP to WAVE_START
*/
void empty_perk_list();

/**
* @brief copies perk data from another
* @note used to actually give a perk to the player
*/
Perk* copy_perk_data(Perk* perk);

/**
* @brief display three potential perks for a player to buy from the shop
*/
void make_random_perk();

/**
* @brief update perk usage once wave ends or remove perk usage is 0
*/
void update_player_perks();

void update_perk_desc(Perk* perk);

GFC_List* get_perk_list();


#endif