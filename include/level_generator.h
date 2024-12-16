#ifndef __LEVEL_GENERATOR_H__
#define __LEVEL_GENERATOR_H__

void level_gen_init(SJson* level_def);
void generate_level();
SJson* generate_level_preview();
void level_generate_enemy_flock(void* level_data);
void level_gen_close();
Uint32 get_flock_count();

#endif