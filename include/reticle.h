#include "entity.h"

typedef struct {
	float           upspeed;
	float           rigspeed;

	int             x_bound;
	int             z_bound;

	GFC_Vector2D	mouse_move;
}ReticleData;

Entity* reticle_spawn(GFC_Vector3D position);
void reticle_think(Entity* self);
void reticle_update(Entity* self);
void reticle_free(Entity* self);
int check_recbounds(Entity* self, GFC_Vector3D movement, ReticleData* data);