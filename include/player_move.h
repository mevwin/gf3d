#ifndef __PLAYER_MOVE_H_
#define __PLAYER_MOVE_H_

#include "entity.h"

enum roll_type {
	UP,
	UPRIGHT,
	RIGHT,
	DOWNRIGHT,
	DOWN,
	DOWNLEFT,
	LEFT,
	UPLEFT
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

#endif