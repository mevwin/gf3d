#include "simple_logger.h"
#include "gf3d_draw.h"
#include "gf2d_mouse.h"
#include "enemy.h"
#include "player.h"
#include "projectile.h"

Entity* enemy_spawn(GFC_Vector3D* player_pos) {
	Entity* self;
	EnemyData* data;
	GFC_Vector3D position;
	Enemy_Type type;

	self = entity_new();
	if (!self) return NULL;

	data = gfc_allocate_array(sizeof(EnemyData), 1);
	if (data) self->data = data;

	type = gfc_random_int(4);

	self->model = gf3d_model_load("models/player_ship/test_ship.model");
	self->think = enemy_think;
	self->update = enemy_update;
	self->free = enemy_free;
	self->entity_type = ENEMY;
	data->enemy_type = PEAS;

	data->took_damage = 0;
	data->currHealth = 30;
	data->maxHealth = 30;
	data->player_pos = player_pos;
	data->pea_speed = 2.5;
	data->proj_count = 0;
	

	//data->upspeed = (float)1.2;
	//data->rigspeed = (float)1.2;

	data->x_bound = 74; // left is positive, right is negative
	data->z_bound = 50;
	data->dist_to_player = -65;

	position = gfc_vector3d_random_pos(data->x_bound, data->dist_to_player, data->z_bound);
	self->position = position;

	self->hurtbox = gfc_sphere(self->position.x, self->position.y, self->position.z, self->model->bounds.h / 5);
	enemy_count++;

	return self;
}
void enemy_think(Entity* self) {
	EnemyData* data;
	GFC_Vector3D player_pos;

	if (!self) return;
	data = self->data;
	if (!data) return;

	player_pos.x = data->player_pos->x;
	player_pos.y = data->player_pos->y;
	player_pos.z = data->player_pos->z;

	
	enemy_proj_spawn(self->position, player_pos, self);
	
	
	//slog("X: %f, Y: %f, Z: %f", self->position.x, self->position.y, self->position.z);
}
void enemy_update(Entity* self) {
	EnemyData* data;
	float dist_x, dist_y, z_angle, y_angle;

	if (!self) return;

	data = self->data;

	if (data->took_damage)
		enemy_take_damage(self, data);

	if (data->currHealth <= 0.0)
		enemy_die(self);

	// rotating enemy to player
	dist_x = data->player_pos->x - self->position.x;
	dist_y = data->player_pos->z - self->position.z;

	z_angle = atan(dist_x / (data->dist_to_player + 5));
	self->rotation.z = z_angle;

	y_angle = atan(dist_y / (data->dist_to_player + 5));
	self->rotation.y = -y_angle;
}

void enemy_free(Entity* self) {
	EnemyData* data;

	if (!self) return;

	data = self->data;
	free(data);
	self->data = NULL;
	enemy_count--;
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