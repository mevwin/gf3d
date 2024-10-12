#include "entity.h"

#define MAX_PROJ 20

enum ProjType {
	SINGLE_SHOT,
	CHARGE_SHOT,
	MISSILE,
	SPREAD_SHOT,
	SUPER_NUKE
};

typedef struct {
	GFC_Vector3D    og_pos;
	GFC_Rect		hitbox;

	float           upspeed;
	float           rigspeed;
	float           forspeed;

	int				y_bound;
	int				curr_mode;
}ProjData;



Entity* proj_spawn(GFC_Vector3D position, int curr_mode);
void proj_think(Entity* self);
void proj_update(Entity* self);
void proj_free(Entity* self);
int	proj_exist(Entity* self, ProjData* data);