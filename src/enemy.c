#include "simple_logger.h"
#include "enemy.h"

Entity* enemy_spawn(GFC_Vector3D position) {
	Entity* self;
	EnemyData* data;

	self = entity_new();
	if (!self) return NULL;

	self->model = gf3d_model_load("models/dino.model");
	self->think = enemy_think;
	self->update = enemy_update;

	self->position = position;
	self->free = enemy_free;
	self->entity_type = ENEMY;

	data = gfc_allocate_array(sizeof(EnemyData), 1);
	if (data) self->data = data;

	data->upspeed = (float)1.2;
	data->rigspeed = (float)1.2;

	data->x_bound = 72; // left is positive, right is negative
	data->z_bound = 48;

	return self;
}
void enemy_think(Entity* self) {

}
void enemy_update(Entity* self) {

}
void enemy_free(Entity* self) {
	EnemyData* data;

	if (!self) return;

	data = (EnemyData*) self->data;
	free(data);
	self->data = NULL;
}
/**
* define enemy/AI behavior as a FSA
* define states of the enemy through enumerations
* Ex:
* typedef enum{
*	IDLE,
*	ATTACK,
*	IN_PAIN,
*	DIE,
*	DEATH
* }enemy_states;
* 
* have think functions for each state
* decide in each state what to do
* 
* make .def for defining monsters
* entity_load_from_def(entityDef)
*/