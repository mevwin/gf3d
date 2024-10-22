#include "entity.h"

typedef struct {
	Uint8			locked_on;
	GFC_Vector3D*	enemy_pos;
	void*			player_data;

	// reticle bounds
	int             x_bound;
	int				y_bound;
	int             z_bound;
}ReticleData;

Entity* reticle_spawn(GFC_Vector3D position, void* player_data);
void reticle_update(Entity* self);
void reticle_free(Entity* self);
Uint8 check_recbounds(Entity* self, GFC_Vector3D movement, ReticleData* data);