//include files from most stable to least stable
//dont include everything, just what you need
#include "simple_logger.h"
#include "gfc_matrix.h"
#include "entity.h"

typedef struct{
    Entity  *entity_list;
    Uint32  entityMax;
    //may need more things later
}EntityManager;

static EntityManager entity_manager = {0}; //C needs 0, C++ doesn't

void entity_system_init(Uint32 maxEnts){
    //sanity check
    if (entity_manager.entity_list){
        slog("entity manager already exists");
        return;
    }

    //another sanity check
    if (!maxEnts){
        slog("cannot allocated 0 entities for the entity manager");
        return;
    }
    entity_manager.entity_list = gfc_allocate_array(sizeof(Entity), maxEnts);
    if (!entity_manager.entity_list){
        slog("failed to allocate %i entities for the entity manager", maxEnts);
        return;
    }
    entity_manager.entityMax = maxEnts; // at this point, big ass entity list is made
    //atexit(entity_system_close);
}
/*
void entity_system_close(){
    int i;
    
    for( i < 0; i < entity_manager.entityMax; i++){
        if (!entity_manager.entity_list[i]._inuse) continue;
        entity_free(&entity_manager.entity_list[i]);
    }

}
*/

void entity_draw(Entity *self){
    GFC_Matrix4 matrix; // not constructors in C
    if (!self) return;

    if (self->draw) self->draw(self);

    gfc_matrix4_from_vectors(
        matrix,
        self->position,
        self->rotation,
        self->scale
    );


    gf3d_model_draw(
        self->model,
        matrix,
        GFC_COLOR_WHITE,
        0
    );
}

void entity_draw_all(){
    int i;
    for (i = 0; i < entity_manager.entityMax; i++) {
        if (entity_manager.entity_list[i]._inuse) continue; // skips ones not set
        entity_draw(&entity_manager.entity_list[i]);
    }
}

void entity_think(Entity *self){
    if (!self) return;
    if (self->think) self->think(self);
}

void entity_think_all(){
    int i;
    for (i = 0; i < entity_manager.entityMax; i++) {
        if (entity_manager.entity_list[i]._inuse) continue; // skips ones not set
        entity_think(&entity_manager.entity_list[i]);
    }
}

void entity_update(Entity *self){
    if (!self) return;
    if (self->update) self->update(self);
}

void entity_update_all(){
    int i;
    for (i = 0; i < entity_manager.entityMax; i++) {
        if (entity_manager.entity_list[i]._inuse) continue; // skips ones not set
        entity_update(&entity_manager.entity_list[i]);
    }
}

Entity *entity_new(){
    int i;
    for (i = 0; i < entity_manager.entityMax; i++)
    {
        if (entity_manager.entity_list[i]._inuse) continue; // skips ones in use
        memset(&entity_manager.entity_list[i], 0, sizeof(Entity)); // clear out in case anything was still there

        // any default values should be set
        entity_manager.entity_list[i]._inuse = 1;
        entity_manager.entity_list[i].scale = gfc_vector3d(1,1,1); // scale of zero means entity doesn't exist

        return &entity_manager.entity_list[i];
    }
    return NULL; // no more entity slots
}

void entity_free(Entity *self){
    // check if pointer is null
    if (!self) return;
    
    if (self->free) self->free(self);

    // free up anything that may have been allocated FOR this
    gf3d_model_free(self->model);
    memset(self, 0, sizeof(Entity));
}