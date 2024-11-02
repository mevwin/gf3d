#include "simple_logger.h"
#include "gf3d_draw.h"
#include "SDL_scancode.h"
#include "gf2d_mouse.h"
#include "enemy.h"
#include "player.h"
#include "projectile.h"
#include "item.h"
#include "level.h"
#include "gfc_audio.h"

#define EMPER_CHARGE_TIME 5.0f;
#define ENEMY_HURTBOX (gfc_box(400, -150, 200, 1, 1, 1))	// make temporary dummy hitbox not accessible to player when enemy is dead

EnemyData* enemy_data_init() {
	EnemyData* data;
	Enemy_Type type;
	LevelData* level;

	data = gfc_allocate_array(sizeof(EnemyData), 1);
	if (!data) return NULL;

	level = get_level_data();
	type = (Enemy_Type) gfc_random_int(4);

	if (type == FENCERS && !level->fencer_flag)
		level->fencer_flag = 1;
	else if (type == FENCERS && level->fencer_flag)
		type = (Enemy_Type) gfc_random_int(3);

	if (type == EMPERS && !level->emper_flag)
		level->emper_flag = 1;
	else if (type == EMPERS && level->emper_flag)
		type = (Enemy_Type)gfc_random_int(2);

	data->enemy_type = type;

	data->pea_speed = 1.5f;

	data->proj_count = 0;
	data->damage_taken = 0;
	data->next_single_shot = 0;

	data->x_bound = 74; // left is positive, right is negative
	data->z_bound = 50;
	data->dist_to_player = -65;

	if (data->enemy_type == PEAS || data->enemy_type == CHARGERS) {
		if (data->enemy_type == CHARGERS) 
			data->maxHealth = 2000.0f;
		else 
			data->maxHealth = 1500.0f;

		data->base_damage = 100.0f;
		data->move_type = DVD_LOGO;
		data->upspeed = 0.3f;
		data->rigspeed = 0.5f;
	}
	else if (data->enemy_type == FENCERS || data->enemy_type == EMPERS) {
		if (data->enemy_type == EMPERS) {
			data->rigspeed = 1.2f;
			data->maxHealth = 1200.0f;
			data->emper_attack_active = 0;
		}
		else {
			data->rigspeed = 0.6f;
			data->maxHealth = 1800.0f;
		}
		
		data->move_type = HORIZONTAL;
	}
	else if (data->enemy_type == BOMBERS)
		data->move_type = STATIONARY;

	data->currHealth = data->maxHealth;

	return data;
}

Entity* enemy_spawn(GFC_Vector3D* player_pos) {
	Entity* self;
	EnemyData* data;
	LevelData* level;
	Entity_Models* models;
	GFC_Vector3D position;

	self = entity_new();
	if (!self) return NULL;

	data = enemy_data_init();
	if (data) self->data = data;

	if (!data) return;

	level = get_level_data();
	if (level->wave_count > 0) 
		enemy_update_stats(data);

	self->think = enemy_think;
	self->update = enemy_update;
	self->free = enemy_free;
	self->entity_type = ENEMY;

	models = get_models();
	if (data->enemy_type == PEAS) 
		self->model = models->peas;
	else if (data->enemy_type == CHARGERS) 
		self->model = models->chargers;
	else if (data->enemy_type == FENCERS)
		self->model = models->fencer;
	else if (data->enemy_type == EMPERS) {
		self->model = models->emper;
		self->think = emper_think;
		data->emper_attack_time = CURRENT_TIME + EMPER_CHARGE_TIME;
	}
	
	data->player_pos = player_pos;

	position = gfc_vector3d_enemy_random_pos(data->x_bound, data->dist_to_player, data->z_bound);
	self->position = position;
	data->spawn_pos = position;

	update_hurtbox(self);

	level->enemy_count++;

	return self;
}

void enemy_think(Entity* self) {
	EnemyData* data;
	PlayerData* player_data;
	LevelData* level;
	GFC_Vector3D player_pos;
	float time;

	if (!self) return;

	data = self->data;
	if (!data) return;

	player_data = get_player_data();
	level = get_level_data();

	// don't do anything if player is dead
	if (player_data->player_dead || data->currHealth <= 0 || 
		level->in_shop || level->paused ||
		player_data->nuke_flag
		) return;
	
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

	time = CURRENT_TIME;

	if (!player_data->player_no_attack) {
		enemy_proj_spawn(self->position, player_pos, self, time);
	}

	if (data->move_type != STATIONARY)
		enemy_move(self);

	//slog("X: %f, Y: %f, Z: %f", self->position.x, self->position.y, self->position.z);
}

void enemy_update(Entity* self) {
	EnemyData* data;
	PlayerData* player_data;
	LevelData* level;
	float dist_x, dist_y, z_angle, y_angle;
	int rand;

	if (!self) return;

	data = self->data;
	if (!data) return;

	player_data = get_player_data();
	level = get_level_data();

	if (level->in_shop || level->paused || !player_data)
		return;

	// dont do anything or find new player
	if (player_data->player_dead) {
		self->rotation.z = 0;
		self->rotation.x = 0;
		return;
	}

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
		update_hurtbox(self);

		if (data->took_damage)
			enemy_take_damage(self, data);

		return;
	}

	if (data->currHealth <= 0.0 && !data->enemy_dead) {
		rand = 1 + gfc_random_int(4);

		if (player_data->active_item == HAPPY_TRIGGER || player_data->active_item == INVINCIBILITY)
			rand = 1 + gfc_random_int(2);
		if (player_data->currHealth >= player_data->maxHealth && rand == HEALTH_PICKUP)
			rand = 1;

		enemy_die(self, data, rand);
		data->enemy_dead = 1;
	}

	self->rotation.y -= 0.1f;

	if (data->enemy_dead && data->proj_count <= 0)
		entity_free(self);
}

void enemy_move(Entity* self) {
	EnemyData* data;

	if (!self) return;

	data = self->data;
	if (!data) return;

	// for both DVD_LOGO and HORIZONTAL
	if (self->position.x > data->x_bound || self->position.x < -data->x_bound)
		data->rigspeed = -data->rigspeed;

	self->position.x += data->rigspeed;

	if (data->move_type == DVD_LOGO) {
		if (self->position.z > (data->z_bound - 6.0f) || self->position.z < -data->z_bound)
			data->upspeed = -data->upspeed;

		self->position.z += data->upspeed;
	}
}

void emper_think(Entity* self) {
	float time;
	EnemyData* data;
	PlayerData* player_data;
	LevelData* level;
	
	if (!self) return;

	data = self->data;
	if (!data) return;

	player_data = get_player_data();
	level = get_level_data();

	if (level->in_shop || level->paused || player_data->player_dead)
		return;

	time = CURRENT_TIME;
	if (data->emper_attack_time - time <= 2.0f)
		self->model->texture = get_models()->damaged;
	else
		self->model->texture = get_models()->emper_spawn;

	if (time >= data->emper_attack_time && !data->emper_attack_active) {
		player_data->emp_time = CURRENT_TIME + 2.0f;
		level->enemy_killed--;
		gfc_sound_play(get_sound_data()->player_damaged, 0, 0.3f, -1, -1);
		data->emper_attack_active = 1;
	}

	if (data->emper_attack_active && time >= player_data->emp_time)
		entity_free(self);

	enemy_move(self);
}

void enemy_free(Entity* self) {
	EnemyData* data;
	LevelData* level;

	if (!self) return;

	data = self->data;
	level = get_level_data();

	if (data->enemy_type == FENCERS)
		level->fencer_flag = 0;
	else if (data->enemy_type == EMPERS)
		level->emper_flag = 0;

	free(data);
	level->enemy_count--;
	level->enemy_killed++;
	//slog("enemy_killed: %d", enemy_killed);
}

void enemy_take_damage(Entity* self, EnemyData* data) {
	if (!data) return;

	data->currHealth -= data->damage_taken;
	if (data->currHealth < 0)
		data->currHealth = 0;

	data->took_damage = 0;
	data->damage_taken = 0;
}

void enemy_die(Entity* self, EnemyData* data, int item_type) {
	if (!data) return;

	item_spawn(SCRAP, self->position, data->dist_to_player);
	item_spawn(item_type, self->position, data->dist_to_player);
	self->rotation.y = 0;
	self->hurtbox.s.b = ENEMY_HURTBOX;

	if (data->enemy_type == FENCERS)
		get_level_data()->fencer_flag = 0;
}

void enemy_update_stats(EnemyData* data) {
	int i;
	
	if (!data) return;

	for (i = get_level_data()->wave_count; i > 0; i--) {
		data->maxHealth *= 1.2f;
		data->currHealth = data->maxHealth;
		data->base_damage *= 1.2f;
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