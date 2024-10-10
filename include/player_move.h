#ifndef __PLAYER_MOVE_H_
#define __PLAYER_MOVE_H_

#include "entity.h"

/**
 * @brief reads player input and turns it into movement
 */
void player_movement(Entity* self);

/**
 * @brief undoes rotation on ship when player isn't pressing a button
 */
void undo_anim(Entity* self);

/**
 * @brief sets player's camera
 */
void player_cam(Entity* self);

#endif