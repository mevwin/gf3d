#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_primitives.h"
#include "gf3d_model.h"

#define MAX_ENTITY 500
#define CURRENT_TIME (SDL_GetTicks() / 1000.0f)

typedef enum {
    PLAYER,
    ENEMY,
    PROJECTILE,
    RETICLE,
    ITEM, 
    ASTEROID
}Entity_Type;

typedef struct {
    Model*      player;
    Texture*    single_shot;
    Texture*    charge_shot;
    Texture*    damaged;
    Texture*    dead;

    Model*      single_proj;
    Model*      charge_proj;
    Model*      super_nuke;
    Model*      reticle;

    Model*      peas;
    Model*      peas_shot;
    Model*      chargers;
    Model*      chargers_shot;
    Model*      fencer;
    Model*      fencer_attack;
    Model*      emper;
    Texture*    emper_spawn;
    Model*      bomber;

    Model*      scrap;
    Model*      health_pickup;
    Model*      happy_trigger;
    Model*      invincibility;
}Entity_Models;

typedef struct Entity_S{
    Uint8           _inuse;     // flag for memory management
    GFC_TextLine    name;       // name of entity
    GFC_Vector3D    position;   // where I am in space
    GFC_Vector3D    rotation;   // how I should rotate
    GFC_Vector3D    scale;      // stretching
    Model           *model;     // graphics
    
    //behavior
    void (*think)   (struct Entity_S *self);    // called every frame for the entity to decide things
    void (*update)  (struct Entity_S *self);    // called every frame for the entity to update its state

    void (*free)    (struct Entity_S *self);    // called when the entity is cleaned up
    void (*draw)    (struct Entity_S* self);    // for custom draw calls
    void            *data;                      // entity data

    Entity_Type     entity_type;                // type of entity
    GFC_Primitive   hurtbox;                    // entity's hurtbox
    Uint8           no_draw;
    
}Entity;
/**
 * always comment in header files about certain shit
 */

/**
 * @brief initialize the entity manager subsystem
 * @param maxEnts how many entities can exist at the same time
 */
void entity_system_init(Uint32 maxEnts);


/**
 * @brief close the entity subsystem when game is closed
 */
void entity_system_close();

/**
 * @brief draw all active entities
 */
void entity_draw_all();


/**
 * @brief let all active entities think
 */
void entity_think_all();

/**
 * @brief let all active entities update
 */
void entity_update_all();

/**
 * @brief allocated a blank entity for use
 * @return NULL on failure (out of memory) or a pointer to the initialized entity
 */
Entity *entity_new();

/**
 * @brief return the memory of a previously allocated entity back to the pool
 * @param self the entity to free
 */
void entity_free(Entity* self);

/**
 * @brief returns the big ass list of entities
 */
Entity* get_entityList();

Entity_Models* get_models();

void update_hurtbox(Entity* self);

void entity_despawn_all();
void entity_reset();
void enemy_reset();

#endif
