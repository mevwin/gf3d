#include "simple_logger.h"
#include "gf3d_draw.h"
#include "enemy.h"
#include "player.h"

Entity* enemy_spawn() {
	Entity* self;
	EnemyData* data;
	GFC_Vector3D position;

	self = entity_new();
	if (!self) return NULL;

	self->model = gf3d_model_load("models/dino.model");
	self->think = enemy_think;
	self->update = enemy_update;
	self->free = enemy_free;
	self->entity_type = ENEMY;

	data = gfc_allocate_array(sizeof(EnemyData), 1);
	if (data) self->data = data;

	data->took_damage = 0;
	data->currHealth = 10;
	data->maxHealth = 10;

	//data->upspeed = (float)1.2;
	//data->rigspeed = (float)1.2;

	data->x_bound = 72; // left is positive, right is negative
	data->z_bound = 48;

	position = gfc_vector3d_random_pos(data->x_bound, -50, data->z_bound);
	self->position = position;

	self->hurtbox = gfc_sphere(self->position.x, self->position.y, self->position.z, self->model->bounds.h / 10);

	enemy_count += 1;


	return self;
}
void enemy_think(Entity* self) {
	//slog("X: %f, Y: %f, Z: %f", self->position.x, self->position.y, self->position.z);
}
void enemy_update(Entity* self) {
	EnemyData* data;

	if (!self) return;

	data = self->data;

	if (data->took_damage)
		enemy_take_damage(self, data);

	if (data->currHealth <= 0.0)
		enemy_die(self);

}
void enemy_free(Entity* self) {
	EnemyData* data;

	if (!self) return;

	data = self->data;
	free(data);
	self->data = NULL;
	enemy_count -= 1;
}

void enemy_take_damage(Entity* self, EnemyData* data) {
	if (!data) return;

	data->currHealth -= data->damage_taken;
	data->took_damage = 0;
	data->damage_taken = 0;
}

void enemy_die(Entity* self) {
	entity_free(self);
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