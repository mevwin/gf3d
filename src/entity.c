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
static Entity_Models* models;

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
    
    models = gfc_allocate_array(sizeof(Entity_Models), 1);
    if (!models) {
        slog("failed to allocate resources for models struct");
        return;
    }

    // initialize all entity models and textures
    models->player = gf3d_model_load("models/player_ship/player_ship_single.model");
    models->single_shot = gf3d_texture_load("models/player_ship/color_77.png");
    models->charge_shot = gf3d_texture_load("models/player_ship/color_44.png");
    models->damaged = gf3d_texture_load("models/player_ship/color_EE.png");
    models->dead = gf3d_texture_load("models/player_ship/color_AA.png");
    models->single_proj = gf3d_model_load("models/projectiles/single_shot.model");
    models->charge_proj = gf3d_model_load("models/projectiles/charge_shot.model");
    models->super_nuke = gf3d_model_load("models/projectiles/super_nuke.model");
    models->reticle = gf3d_model_load("models/reticle/reticle.model");

    models->peas = gf3d_model_load("models/enemy/peas.model");
    models->peas_shot = gf3d_model_load("models/projectiles/single_shot_enem.model");
    models->chargers = gf3d_model_load("models/enemy/chargers.model");
    models->chargers_shot = gf3d_model_load("models/projectiles/charge_shot_enem.model");
    models->fencer = gf3d_model_load("models/enemy/fencer.model");
    models->fencer_attack = gf3d_model_load("models/projectiles/fencer_attack.model");

    models->scrap = gf3d_model_load("models/item/enemy_scrap.model");
    models->health_pickup = gf3d_model_load("models/item/health_pickup.model");
    models->happy_trigger = gf3d_model_load("models/item/happy_trigger.model");
    models->invincibility = gf3d_model_load("models/item/invincibility.model");

    atexit(entity_system_close);
}

void entity_system_close(){
    int i;

    for( i = 0; i < entity_manager.entityMax; i++){
        if (!entity_manager.entity_list[i]._inuse) continue;
        entity_free(&entity_manager.entity_list[i]);
    }

    gf3d_model_free(models->player);
    gf3d_texture_free(models->single_shot);
    gf3d_texture_free(models->charge_shot);
    gf3d_texture_free(models->damaged);
    gf3d_texture_free(models->dead);
    gf3d_model_free(models->single_proj);
    gf3d_model_free(models->charge_proj);
    gf3d_model_free(models->super_nuke);
    gf3d_model_free(models->reticle);

    gf3d_model_free(models->peas);
    gf3d_model_free(models->peas_shot);
    gf3d_model_free(models->chargers);
    gf3d_model_free(models->chargers_shot);
    gf3d_model_free(models->fencer);
    gf3d_model_free(models->fencer_attack);

    gf3d_model_free(models->scrap);
    gf3d_model_free(models->health_pickup);
    gf3d_model_free(models->happy_trigger);
    gf3d_model_free(models->invincibility);

    free(entity_manager.entity_list);
    memset(&entity_manager, 0, sizeof(EntityManager));
    free(models);
}

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
        NULL,
        0
    );
}

void entity_draw_all(){
    int i;
    for (i = 0; i < entity_manager.entityMax; i++) {
        if (!entity_manager.entity_list[i]._inuse) continue; // skips ones not inuse
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
        if (!entity_manager.entity_list[i]._inuse) continue; // skips ones not inuse
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
        if (!entity_manager.entity_list[i]._inuse) continue; // skips ones not inuse
        entity_update(&entity_manager.entity_list[i]);
    }
}

Entity *entity_new(){
    int i;
    for (i = 0; i < entity_manager.entityMax; i++)
    {
        if (entity_manager.entity_list[i]._inuse) continue; // skips ones inuse
        memset(&entity_manager.entity_list[i], 0, sizeof(Entity)); // clear out in case anything was still there

        // any default values should be set
        entity_manager.entity_list[i]._inuse = 1;
        entity_manager.entity_list[i].scale = gfc_vector3d(1,1,1); // scale of zero means entity doesn't exist

        return &entity_manager.entity_list[i];
    }
    slog("no more entity slots");
    return NULL; // no more entity slots
}

void entity_free(Entity *self){
    // check if pointer is null
    if (!self) return;
    
    //self->_inuse = 0;

    if (self->free) self->free(self);

    // free up anything that may have been allocated FOR this
    //gf3d_model_free(self->model);
    //self->model = NULL;
    memset(self, 0, sizeof(Entity));
}

Entity* get_entityList() {
    return entity_manager.entity_list;
}

Entity_Models* get_models() {
    return models;
}

void update_hurtbox(Entity* self) {
    if (!self) return;

    self->hurtbox.s.p = self->position;

    if (self->entity_type == PLAYER) {
        self->hurtbox.s.b = gfc_box(
            self->position.x - (self->model->bounds.w / 4.0),
            self->position.y - (self->model->bounds.h / 4.0),
            self->position.z - (self->model->bounds.d / 4.0),
            self->model->bounds.w / 2.0,
            self->model->bounds.h / 2.0,
            self->model->bounds.d / 2.0);

        self->hurtbox.s.s = gfc_sphere(
            self->position.x,
            self->position.y,
            self->position.z,
            self->model->bounds.w / 2.0
        );
    }
    else if (self->entity_type == ITEM) {
        self->hurtbox.s.s = gfc_sphere(
            self->position.x,
            self->position.y,
            self->position.z,
            self->model->bounds.w / 2.0
        );
    }
    else if (self->entity_type == RETICLE) {
        self->hurtbox.s.b = gfc_box(
            self->position.x - (self->model->bounds.w / 2.0),
            self->position.y - (self->model->bounds.h / 2.0),
            self->position.z - (self->model->bounds.d / 2.0),
            self->model->bounds.w,
            self->model->bounds.h,
            self->model->bounds.d);
    }
    else if (self->entity_type == PROJECTILE) {
        self->hurtbox.s.b = gfc_box(
            self->position.x - (self->model->bounds.w / 2),
            self->position.y - (self->model->bounds.h / 2),
            self->position.z - (self->model->bounds.d / 2),
            self->model->bounds.w,
            self->model->bounds.h,
            self->model->bounds.d);

        self->hurtbox.s.s = gfc_sphere(
            self->position.x,
            self->position.y,
            self->position.z,
            self->model->bounds.w / 2.0
        );
    }
    else if (self->entity_type == ENEMY) {
        self->hurtbox.s.b = 
            gfc_box(self->position.x - (self->model->bounds.w / 2),
            self->position.y - (self->model->bounds.h / 2),
            self->position.z - (self->model->bounds.d / 2),
            self->model->bounds.w,
            self->model->bounds.h,
            self->model->bounds.d);
    }
}

void entity_despawn_all() {
    Entity* entityList, * target;
    int i;

    entityList = get_entityList();
    
    // despawn all projectiles first
    for (i = 0; i < MAX_ENTITY; i++) {
        target = &entityList[i];

        if (target->entity_type != PROJECTILE)
            continue;

        entity_free(target);
    }

    for (i = 0; i < MAX_ENTITY; i++) {
        target = &entityList[i];

        entity_free(target);
    }
}

void entity_reset() {
    Entity* entityList, * target;
    int i;

    entityList = get_entityList();

    // despawn all projectiles, items, and reticles first
    for (i = 0; i < MAX_ENTITY; i++) {
        target = &entityList[i];

        if (target->entity_type == PLAYER || target->entity_type == ENEMY)
            continue;

        entity_free(target);
    }

    // despawn everything except player
    for (i = 0; i < MAX_ENTITY; i++) {
        target = &entityList[i];

        if (target->entity_type != ENEMY) continue;

        entity_free(target);
    }
}

void enemy_reset() {
    Entity* entityList, * target;
    int i;

    entityList = get_entityList();

    // despawn all projectiles first
    for (i = 0; i < MAX_ENTITY; i++) {
        target = &entityList[i];

        if (target->entity_type != PROJECTILE)
            continue;

        entity_free(target);
    }

    for (i = 0; i < MAX_ENTITY; i++) {
        target = &entityList[i];

        if (target->entity_type != ENEMY)
            continue;

        entity_free(target);
    }
}