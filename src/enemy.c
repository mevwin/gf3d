#include "simple_logger.h"
#include "gf3d_draw.h"
#include "SDL_scancode.h"
#include "gf2d_mouse.h"
#include "enemy.h"
#include "player.h"
#include "projectile.h"
#include "item.h"

EnemyData* enemy_data_init() {
	EnemyData* data;
	Enemy_Type type;

	data = gfc_allocate_array(sizeof(EnemyData), 1);
	if (!data) return NULL;

	type = (Enemy_Type) gfc_random_int(3);

	if (type == FENCERS && fencer_count >= FENCER_MAX)
		type = (Enemy_Type) gfc_random_int(2);

	data->enemy_type = type;

	data->maxHealth = 1500.0;
	data->currHealth = 1500.0;

	data->base_damage = 100.0;
	data->pea_speed = 1.5;

	data->upspeed = (float)1.2;
	data->rigspeed = (float)0.6;

	data->proj_count = 0;
	data->damage_taken = 0.0;
	data->next_single_shot = 0.0;

	data->x_bound = 74; // left is positive, right is negative
	data->z_bound = 50;
	data->dist_to_player = -65;

	return data;
}

Entity* enemy_spawn(GFC_Vector3D* player_pos) {
	Entity* self;
	EnemyData* data;
	GFC_Vector3D position;

	self = entity_new();
	if (!self) return NULL;

	data = enemy_data_init();
	if (data) self->data = data;

	if (!data) return;

	if (wave_count > 0) enemy_update_stats(data);

	if (data->enemy_type == PEAS) 
		self->model = get_models()->peas;
	else if (data->enemy_type == CHARGERS)
		self->model = get_models()->chargers;
	else if (data->enemy_type == FENCERS) {
		self->model = get_models()->fencer;
	}
	
	self->think = enemy_think;
	self->update = enemy_update;
	self->free = enemy_free;
	self->entity_type = ENEMY;

	data->player_pos = player_pos;

	position = gfc_vector3d_enemy_random_pos(data->x_bound, data->dist_to_player, data->z_bound);
	self->position = position;
	data->spawn_pos = position;

	self->hurtbox = gfc_box(self->position.x - (self->model->bounds.w / 2),
							self->position.y - (self->model->bounds.h / 2),
							self->position.z - (self->model->bounds.d / 2),
							self->model->bounds.w,
							self->model->bounds.h,
							self->model->bounds.d);

	enemy_count++;

	return self;
}
void enemy_think(Entity* self) {
	EnemyData* data;
	PlayerData* player_data;
	GFC_Vector3D player_pos;
	float time;

	if (!self) return;

	data = self->data;
	if (!data) return;

	player_data = get_player_data();

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
	
	enemy_move(self);
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

	player_data = get_player_data();

	if (player_data->in_shop || player_data->paused || !player_data)
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
		if (player_data->currMode != MISSILE)
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
	if (data->currHealth <= 0.0 && !data->enemy_dead) {
		rand = 1 + gfc_random_int(4);

		if (player_data->active_item == HAPPY_TRIGGER || player_data->active_item == INVINCIBILITY)
			rand = 1 + gfc_random_int(2);
		if (player_data->currHealth >= player_data->maxHealth && rand == HEALTH_PICKUP)
			rand = 1.0;

		enemy_die(self, data, rand);
		data->enemy_dead = 1;
	}

	self->rotation.y -= 0.1;

	if (data->enemy_dead && data->proj_count <= 0)
		entity_free(self);
}

void enemy_move(Entity* self) {
	EnemyData* data;

	if (!self) return;

	data = self->data;
	if (!data) return;

	if (self->position.x > data->x_bound)
		data->rigspeed = -data->rigspeed;
	else if (self->position.x < -data->x_bound)
		data->rigspeed = -data->rigspeed;

	self->position.x += data->rigspeed;
}

void enemy_free(Entity* self) {
	EnemyData* data;

	if (!self) return;

	data = self->data;

	if (data->enemy_type == FENCERS)
		fencer_count--;

	free(data);
	enemy_count--;
	enemy_killed++;
	//slog("enemy_killed: %d", enemy_killed);
}

void enemy_take_damage(Entity* self, EnemyData* data) {
	if (!data) return;

	data->currHealth -= data->damage_taken;
	data->took_damage = 0;
	data->damage_taken = 0.0;
}

void enemy_die(Entity* self, EnemyData* data, int item_type) {
	if (!data) return;

	item_spawn(SCRAP, self->position, data->dist_to_player);
	item_spawn(item_type, self->position, data->dist_to_player);
	self->rotation.y = 0;
	self->hurtbox = gfc_box(400.0, -150.0, 200.0, 1.0, 1.0, 1.0);	// make dummy hitbox not accessible to player
}

void enemy_update_stats(EnemyData* data) {
	int i;
	
	if (!data) return;

	for (i = wave_count; i > 0; i--) {
		data->maxHealth *= 1.1;
		data->currHealth = data->maxHealth;
		data->base_damage *= 1.1;
	}
	//slog("enemy stats updated %d times", wave_count);
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