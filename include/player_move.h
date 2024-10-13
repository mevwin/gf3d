#ifndef __PLAYER_MOVE_H_
#define __PLAYER_MOVE_H_

#include "entity.h"
#include "player.h"

enum roll_type {
	UP,
	RIGHT,
	DOWN,
	LEFT
};

/**
 * @brief reads player input and turns it into movement
 */
void player_movement(Entity* self, PlayerData* data);

/**
 * @brief sets player's camera
 */
void player_cam(Entity* self, PlayerData* data);

/**
 * @brief do a barrel roll!
 */
void barrel_roll(Entity* self, PlayerData* data);

int check_movebounds(Entity* self, GFC_Vector3D movement, PlayerData* data);

void mousepos_to_gamepos(GFC_Vector2D* cursor_pos, PlayerData* data);

void ease_anim(Entity* self, PlayerData* data);

#endif