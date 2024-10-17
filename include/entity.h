#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gf3d_model.h"

typedef enum {
    PLAYER,
    ENEMY,
    PROJECTILE,
    RETICLE
}Entity_Type;

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
    GFC_Rect        hurtbox;                    // entity's hurtbox
    
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

#endif