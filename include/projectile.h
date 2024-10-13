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

	float			next_shot_time;

}ProjData;



Entity* proj_spawn(GFC_Vector3D position, int curr_mode, float curr_time);
void proj_update(Entity* self);
void proj_free(Entity* self);
int	proj_exist(Entity* self, ProjData* data);
void proj_think_basic(Entity* self);
void proj_think_missile(Entity* self);
void proj_think_spread_shot(Entity* self);
void proj_think_super_nuke(Entity* self);