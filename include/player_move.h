#ifndef __PLAYER_MOVE_H_
#define __PLAYER_MOVE_H_

#include "entity.h"

enum roll_type {
	UP,
	RIGHT,
	DOWN,
	LEFT
};

/**
 * @brief reads player input and turns it into movement
 */
void player_movement(Entity* self);

/**
 * @brief sets player's camera
 */
void player_cam(Entity* self);

/**
 * @brief do a barrel roll!
 */
void barrel_roll(Entity* self);

int check_bounds(Entity* self, GFC_Vector3D movement);
#endif