#include "entity.h"

typedef struct {
	// reticle bounds
	int             x_bound;
	int				y_bound;
	int             z_bound;
}ReticleData;

GFC_Vector3D* reticle_spawn(GFC_Vector3D position);
void reticle_update(Entity* self);
void reticle_free(Entity* self);
int check_recbounds(Entity* self, GFC_Vector3D movement, ReticleData* data);