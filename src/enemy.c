#include "simple_logger.h"
#include "gfc_audio.h"
#include "gfc_config.h"
#include "enemy.h"
#include "player.h"
#include "projectile.h"
#include "item.h"
#include "level.h"

#define EMPER_CHARGE_TIME 5.0f
#define ENEMY_HURTBOX gfc_box(400, -150, 200, 1, 1, 1) // make temporary dummy hitbox not accessible to player when enemy is dead

void enemy_think(Entity* self);
void enemy_update(Entity* self);
void emper_think(Entity* self);
void enemy_move(Entity* self);
void enemy_free(Entity* self);
void enemy_update_stats(EnemyData* data);
void enemy_take_damage(Entity* self, EnemyData* data);
void enemy_die(Entity* self, EnemyData* data, int item_type);
void bomber_die(Entity* self, EnemyData* data, GFC_Vector3D position);
int random_item();
EnemyData* enemy_data_init_from_config(EnemyType enemy_type, SJson* object);

// spawn random enemy (OLD, commented out for future reference)
/*
EnemyData* enemy_data_init_random() {
	EnemyData* data;
	EnemyType type;
	LevelData* level;

	data = gfc_allocate_array(sizeof(EnemyData), 1);
	if (!data) return NULL;

	level = get_level_data();
	type = (EnemyType) gfc_random_int(5);

	// only one fencer on-screen
	if (type == FENCERS && !level->fencer_flag)
		level->fencer_flag = 1;
	else if (type == FENCERS && level->fencer_flag)
		type = (EnemyType) gfc_random_int(4);

	// only one emper on-screen
	if (type == EMPERS && !level->emper_flag)
		level->emper_flag = 1;
	else if (type == EMPERS && level->emper_flag)
		type = (EnemyType) gfc_random_int(3);

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
			data->base_damage = 10.0f;
		}
		
		data->move_type = HORIZONTAL;
	}
	else if (data->enemy_type == BOMBERS) {
		data->move_type = HORIZONTAL;
		data->base_damage = 400.0f;
		data->maxHealth = 10.0f;
		data->rigspeed = 0.3f;
	}

	data->currHealth = data->maxHealth;

	return data;
}
*/

EnemyData* enemy_data_init_from_config(EnemyType enemy_type, SJson* object) {
	SJson* enemy_def, *enemy_entry;
	EnemyData* e_data;
	LevelData* level;
	GFC_Vector2D spawn_check;
	int index, i;

	level = get_level_data();
	enemy_def = sj_load("def/enemy.def");

	e_data = gfc_allocate_array(sizeof(EnemyData), 1);
	if (!e_data) return NULL;

	e_data->proj_count = 0;
	e_data->damage_taken = 0;
	e_data->next_single_shot = 0;

	e_data->x_bound = 74; // left is positive, right is negative
	e_data->z_bound = 50;
	e_data->dist_to_player = -65;

	if (object) { //i.e. spawning from given enemy object (REGULAR)
		sj_object_get_value_as_int(object, "enemy_type", &index);
		e_data->enemy_type = (EnemyType) index;

		sj_value_as_vector2d(sj_object_get_value(object, "spawn"), &spawn_check);
		if (spawn_check.x == 0 && spawn_check.y == 0)
			e_data->spawn_pos = gfc_vector3d_enemy_random_pos(e_data->x_bound, e_data->dist_to_player, e_data->z_bound);
		else
			e_data->spawn_pos = gfc_vector3d(spawn_check.x, e_data->dist_to_player, spawn_check.y);
	}
	else { // i.e. spawning random enemy (ENDLESS)
		index = (int) enemy_type;
		e_data->enemy_type = enemy_type;
		e_data->spawn_pos = gfc_vector3d_enemy_random_pos(e_data->x_bound, e_data->dist_to_player, e_data->z_bound);
	}

	enemy_entry = sj_array_get_nth(sj_object_get_value(enemy_def, "enemy_list"), index);

	sj_object_get_value_as_float(enemy_entry, "pea_speed", &e_data->pea_speed);
	sj_object_get_value_as_float(enemy_entry, "maxHealth", &e_data->maxHealth);
	e_data->currHealth = e_data->maxHealth;
	sj_object_get_value_as_float(enemy_entry, "base_damage", &e_data->base_damage);
	sj_object_get_value_as_int(enemy_entry, "move_type", &i);
	e_data->move_type = (EnemyMove) i;
	sj_object_get_value_as_float(enemy_entry, "upspeed", &e_data->upspeed);
	sj_object_get_value_as_float(enemy_entry, "rigspeed", &e_data->rigspeed);

	free(enemy_def);

	return e_data;
}

void enemy_spawn(GFC_Vector3D* player_pos, EnemyType enemy_type, SJson* object) {
	Entity* self;
	EnemyData* data;
	LevelData* level;
	Entity_Models* models;

	self = entity_new();
	if (!self) return NULL;

	data = enemy_data_init_from_config(enemy_type, object);
	if (data) self->data = data;

	if (!data) return;

	level = get_level_data();
	if (level->wave_count > 1) 
		enemy_update_stats(data);

	self->think = enemy_think;
	self->update = enemy_update;
	self->free = enemy_free;
	self->entity_type = ENEMY;

	models = get_models();
	switch (data->enemy_type) {
		case PEAS:
			self->model = models->peas;
			break;

		case CHARGERS:
			self->model = models->chargers;
			break;

		case FENCERS:
			self->model = models->fencer;
			break;

		case EMPERS:
			self->model = models->emper;
			self->think = emper_think;
			data->emper_attack_active = 0;
			data->emper_attack_time = CURRENT_TIME + EMPER_CHARGE_TIME;
			break;

		case BOMBERS:
			self->model = models->bomber;
			break;

	}
	data->player_pos = player_pos;

	self->position = data->spawn_pos;

	update_hurtbox(self);

	level->enemy_count++;
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
	gfc_vector3d_copy_ptr(player_pos, data->player_pos);
	time = CURRENT_TIME;

	if (!player_data->player_no_attack && data->enemy_type != BOMBERS) {
		enemy_proj_spawn(self->position, player_pos, self, time);
	}

	if (data->move_type != STATIONARY)
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
	rand = random_item();
	// bomber deaths are a special case
	if (data->enemy_type != BOMBERS) {
		if (data->currHealth <= 0.0 && !data->enemy_dead) 
			enemy_die(self, data, rand);

		self->rotation.y -= 0.1f;

		if (data->enemy_dead && data->proj_count <= 0)
			entity_free(self);
	}
	else if (data->enemy_type == BOMBERS) {
		if (data->currHealth <= 0.0 && !data->enemy_dead) {
			if (data->damaged_type == CHARGE_SHOT) 
				enemy_die(self, data, rand);
			else {
				bomber_die(self, data, self->position);
			}
		}
		else if (data->enemy_dead && data->proj_count == 0)
			entity_free(self);
	}
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

	time = CURRENT_TIME;
	if (data->emper_attack_time - time <= 2.0f)
		self->model->texture = get_models()->damaged;
	else
		self->model->texture = get_models()->emper_spawn;

	if (time >= data->emper_attack_time && !data->emper_attack_active) {
		player_data->emp_time = CURRENT_TIME + 2.0f;
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
	if (!data) return;

	level = get_level_data();

	if (data->enemy_type == FENCERS)
		level->fencer_flag = 0;
	else if (data->enemy_type == EMPERS)
		level->emper_flag = 0;

	free(data);
	level->enemy_count--;

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

int random_item() {
	PlayerData* player_data;
	int rand;

	rand = 1 + gfc_random_int(4);
	
	player_data = get_player_data();

	if (player_data->active_item == HAPPY_TRIGGER || player_data->active_item == INVINCIBILITY)
		rand = 1 + gfc_random_int(2);
	if (player_data->currHealth >= player_data->maxHealth && rand == HEALTH_PICKUP)
		rand = 1;

	return rand;
}

void enemy_die(Entity* self, EnemyData* data, int item_type) {
	if (!self || !data) return;

	item_spawn(SCRAP, self->position, data->dist_to_player);
	item_spawn(item_type, self->position, data->dist_to_player);
	self->rotation.y = 0;
	self->hurtbox.s.b = ENEMY_HURTBOX;

	if (data->enemy_type == FENCERS)
		get_level_data()->fencer_flag = 0;

	get_level_data()->enemy_killed++;
	data->enemy_dead = 1;
}

void bomber_die(Entity* self, EnemyData* data, GFC_Vector3D position){
	GFC_Vector3D player_pos, spawn_pos;

	if (!self || !data) return;

	self->hurtbox.s.b = ENEMY_HURTBOX;
	self->no_draw = 1;

	gfc_vector3d_copy_ptr(player_pos, data->player_pos);
	gfc_vector3d_copy(spawn_pos, position);

	spawn_pos.x -= 5.0f;
	spawn_pos.z += 5.0f;
	enemy_proj_spawn(spawn_pos, player_pos, self, CURRENT_TIME);

	spawn_pos.x += 10.0f;
	enemy_proj_spawn(spawn_pos, player_pos, self, CURRENT_TIME);

	spawn_pos = self->position;
	spawn_pos.z -= 10.0f;
	enemy_proj_spawn(spawn_pos, player_pos, self, CURRENT_TIME);

	data->enemy_dead = 1;
}

void enemy_update_stats(EnemyData* data) {
	int i;
	
	if (!data) return;

	for (i = get_level_data()->wave_count; i > 1; i--) {
		data->maxHealth *= 1.2f;
		data->currHealth = data->maxHealth;
		data->base_damage *= 1.2f;
	}
	//slog("enemy stats updated %d times", wave_count);
}