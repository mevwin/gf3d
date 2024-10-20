#ifndef __PLAYER_MOVE_H_
#define __PLAYER_MOVE_H_

#include "player.h"

typedef enum {
	UP,
	RIGHT,
	DOWN,
	LEFT
}Roll_Type;

/**
 * @brief reads player input and turns it into movement
 */
void player_movement(Entity* self, PlayerData* data);

/**
 * @brief sets player's camera offset
 */
void player_cam(Entity* self, PlayerData* data);

/**
 * @brief do a barrel roll!
 */
void barrel_roll(Entity* self, PlayerData* data);

/**
* @brief make sure player is in camera view
*/
Uint8 check_movebounds(Entity* self, GFC_Vector3D movement, PlayerData* data);

#endif