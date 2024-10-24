#include "simple_logger.h"
#include "gf3d_draw.h"
#include "SDL_scancode.h"
#include "gf2d_mouse.h"
#include "enemy.h"
#include "player.h"
#include "projectile.h"
#include "item.h"

Entity* enemy_spawn(GFC_Vector3D* player_pos, void* p_data) {
	Entity* self;
	EnemyData* data;
	PlayerData* player_data;
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

	data->enemy_type = gfc_random_int(1);
	data->maxHealth = 1500.0;
	data->currHealth = 1500.0;

	data->base_damage = 100.0;
	data->pea_speed = 1.5;

	//data->upspeed = (float)1.2;
	//data->rigspeed = (float)1.2;

	data->proj_count = 0;
	data->damage_taken = 0.0;
	data->next_single_shot = 0.0;

	data->player_pos = player_pos;
	data->player_data = (PlayerData*) p_data;

	data->x_bound = 74; // left is positive, right is negative
	data->z_bound = 50;
	data->dist_to_player = -65;

	position = gfc_vector3d_random_pos(data->x_bound, data->dist_to_player, data->z_bound);
	self->position = position;
	data->spawn_pos = position;


	self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
							self->position.y - (self->model->bounds.h / 2),
							self->position.z - (self->model->bounds.d / 2),
							self->model->bounds.w,
							self->model->bounds.h,
							self->model->bounds.d);
	//check_rand_position(self);

	enemy_count++;

	return self;
}
void enemy_think(Entity* self) {
	EnemyData* data;
	PlayerData* player_data;
	GFC_Vector3D player_pos;
	const Uint8* keys;
	float time;

	if (!self) return;

	data = self->data;
	if (!data) return;

	player_data = data->player_data;

	// don't do anything if player is dead
	if (player_data->player_dead || data->currHealth <= 0.0 || player_data->in_shop || player_data->paused) return;
	
	// TODO: remove this debug tool later
	/*
	keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_I])
		self->position.z += 1.0;	
	if (keys[SDL_SCANCODE_K])
		self->position.z -= 1.0;	
	if (keys[SDL_SCANCODE_J])
		self->position.x += 1.0;	
	if (keys[SDL_SCANCODE_L])
		self->position.x -= 1.0;
		*/

	player_pos.x = data->player_pos->x;
	player_pos.y = data->player_pos->y;
	player_pos.z = data->player_pos->z;

	time = SDL_GetTicks() / 1000.0;

	if (!player_data->player_no_attack)
		enemy_proj_spawn(self->position, player_pos, self, time);
	
	//slog("X: %f, Y: %f, Z: %f", self->position.x, self->position.y, self->position.z);
}

void enemy_update(Entity* self) {
	EnemyData* data;
	PlayerData* player_data;
	float dist_x, dist_y, z_angle, y_angle;
	int rand;

	if (!self) return;

	data = self->data;
	if (!data) return;

	player_data = data->player_data;

	if (player_data->in_shop || player_data->paused)
		return;

	// dont do anything or find new player
	if (player_data->player_dead) {
		self->rotation.z = 0;
		self->rotation.x = 0;
		return;
	}

	// rounding floats to nearest tenth
	data->currHealth = roundf(10 * data->currHealth) / 10;

	if (data->currHealth > 0.0) {
		if (!player_data->player_no_attack) {
			// rotating enemy to player
			dist_x = data->player_pos->x - self->position.x;
			dist_y = data->player_pos->z - self->position.z;

			z_angle = atan(dist_x / (data->dist_to_player + 5));
			self->rotation.z = z_angle;

			y_angle = atan(dist_y / (data->dist_to_player + 5));
			self->rotation.y = -y_angle;
		}
		else {
			self->rotation.z = 0;
			self->rotation.x = 0;
		}

		// undo missile_targeted if player is no longer in that mode
		if (player_data->curr_mode != MISSILE)
			data->missile_targeted = 0;

		// update hurtbox
		self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
						self->position.y - (self->model->bounds.h / 2),
						self->position.z - (self->model->bounds.d / 2),
						self->model->bounds.w,
						self->model->bounds.h,
						self->model->bounds.d);

		if (data->took_damage)
			enemy_take_damage(self, data);

		return;
	}
	rand = gfc_random_int(3);
	if (data->currHealth <= 0.0 && !data->enemy_dead) {
		data->enemy_dead = 1;
		slog("rand: %d", rand);
		enemy_die(self, data, rand);
	}

	self->rotation.y -= 0.2;

	if (data->item_taken && data->enemy_dead && data->proj_count == 0)
		entity_free(self);
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
	data->damage_taken = 0.0;
}

void enemy_die(Entity* self, EnemyData* data, int item_type) {
	item_spawn(item_type, self->position, data);
	self->rotation.y = 0;
	self->hurtbox.x = 200.0;	// move hurtbox outside player view
}

/* 
void check_rand_position(Entity* self) {
	Entity* entityList, * entity;
	int i;

	entityList = get_entityList();

	for (i = 0; i < MAX_ENTITY; i++) {
		entity = &entityList[i];

		if (entity->entity_type != ENEMY)
			continue;

		// spawn collision check
		while (gfc_box_overlap(self->hurtbox, entity->hurtbox)) {
			self->position = gfc_vector3d_random_pos(74, self->position.y, 50);
			self->hurtbox = self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
				self->position.y - (self->model->bounds.h / 2),
				self->position.z - (self->model->bounds.d / 2),
				self->model->bounds.w,
				self->model->bounds.h,
				self->model->bounds.d);
		}
	}
}
*/
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