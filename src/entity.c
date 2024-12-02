//include files from most stable to least stable
//dont include everything, just what you need
#include "simple_logger.h"
#include "gfc_matrix.h"
#include "entity.h"
#include "world.h"

typedef struct EntityManager_S{
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

    atexit(entity_system_close);
}

void entity_system_close(){
    int i;

    for( i = 0; i < entity_manager.entityMax; i++){
        if (!entity_manager.entity_list[i]._inuse) continue;
        entity_free(&entity_manager.entity_list[i]);
    }

    free(entity_manager.entity_list);
    memset(&entity_manager, 0, sizeof(EntityManager));

    entity_assets_close();
    free(models);
}

// initialize all entity models and textures
void player_assets_init(){
    int i;

    i = 0;

    // player
    models->player = gf3d_model_load("models/player_ship/player_ship_single.model");
    if (!models->player)
        slog("no player model");
    else
        i++;

    models->single_shot = gf3d_texture_load("models/player_ship/color_77.png");
    if (!models->single_shot)
        slog("no player single shot texture");
    else
        i++;


    models->charge_shot = gf3d_texture_load("models/player_ship/color_44.png");
    if (!models->charge_shot)
        slog("no player charge shot texture");
    else
        i++;

    models->damaged = gf3d_texture_load("models/player_ship/color_EE.png");
    if (!models->damaged)
        slog("no player damaged texture");
    else
        i++;

    models->dead = gf3d_texture_load("models/player_ship/color_AA.png");
    if (!models->dead)
        slog("no player dead texture");
    else
        i++;

    models->single_proj = gf3d_model_load("models/projectiles/single_shot.model");
    if (!models->single_proj)
        slog("no player single shot model");
    else
        i++;

    models->charge_proj = gf3d_model_load("models/projectiles/charge_shot.model");
    if (!models->charge_proj)
        slog("no player charge shot model");
    else
        i++;

    models->super_nuke = gf3d_model_load("models/projectiles/super_nuke.model");
    if (!models->super_nuke)
        slog("no player super nuke model");
    else
        i++;

    models->reticle = gf3d_model_load("models/reticle/reticle.model");
    if (!models->reticle)
        slog("no player reticle model");
    else
        i++;

    if (i == 9)
        get_world_data()->player_assets_made = 1;
    else {
        slog("not every player asset has been init");
        free(models);
        get_world_data()->_done = 1;
        return;
    }
}

void enemy_assets_init() {
    Uint8 i;

    i = 0;

    // enemy
    models->peas = gf3d_model_load("models/enemy/peas.model");
    if (!models->peas)
        slog("no enemy peas model");
    else
        i++;

    models->peas_shot = gf3d_model_load("models/projectiles/single_shot_enem.model");
    if (!models->peas_shot)
        slog("no enemy peas shot model");
    else
        i++;

    models->chargers = gf3d_model_load("models/enemy/chargers.model");
    if (!models->chargers)
        slog("no enemy chargers model");
    else
        i++;

    models->chargers_shot = gf3d_model_load("models/projectiles/charge_shot_enem.model");
    if (!models->chargers_shot)
        slog("no enemy chargers shot model");
    else
        i++;

    models->fencer = gf3d_model_load("models/enemy/fencer.model");
    if (!models->fencer)
        slog("no enemy fencer model");
    else
        i++;

    models->fencer_attack = gf3d_model_load("models/projectiles/fencer_attack.model");
    if (!models->fencer_attack)
        slog("no enemy fencer attack model");
    else
        i++;

    models->emper = gf3d_model_load("models/enemy/emper.model");
    if (!models->emper)
        slog("no enemy emper model");
    else
        i++;

    models->emper_spawn = gf3d_texture_load("models/enemy/color_55.png");
    if (!models->emper_spawn)
        slog("no enemy emper spawn texture");
    else
        i++;

    models->bomber = gf3d_model_load("models/enemy/bomber.model");
    if (!models->bomber)
        slog("no enemy bomber model");
    else
        i++;

    if (i == 9)
        get_world_data()->enemy_assets_made = 1;
    else {
        slog("not every enemy asset has been init");
        free(models);
        get_world_data()->_done = 1;
        return;
    }
}

void item_assets_init() {
    Uint8 i;

    i = 0;

    // item
    models->scrap = gf3d_model_load("models/item/enemy_scrap.model");
    if (!models->scrap)
        slog("no item scrap model");
    else
        i++;
    
    models->health_pickup = gf3d_model_load("models/item/health_pickup.model");
    if (!models->health_pickup)
        slog("no item health pickup model");
    else
        i++;
    
    models->happy_trigger = gf3d_model_load("models/item/happy_trigger.model");
    if (!models->happy_trigger)
        slog("no item happy trigger model");
    else
        i++;
    
    models->invincibility = gf3d_model_load("models/item/invincibility.model");
    if (!models->invincibility)
        slog("no item invicibility model");
    else
        i++;
    
    if (i == 4)
        get_world_data()->item_assets_made = 1;
    else {
        slog("not every item asset has been init");
        free(models);
        get_world_data()->_done = 1;
        return;
    }
}

void entity_assets_close() {
    WorldData* world;

    world = get_world_data();

    world->player_assets_made = 0;
    world->enemy_assets_made = 0;
    world->item_assets_made = 0;

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
    gf3d_model_free(models->emper);
    gf3d_texture_free(models->emper_spawn);
    gf3d_model_free(models->bomber);

    gf3d_model_free(models->scrap);
    gf3d_model_free(models->health_pickup);
    gf3d_model_free(models->happy_trigger);
    gf3d_model_free(models->invincibility);

}

void entity_draw(Entity *self){
    GFC_Matrix4 matrix; // not constructors in C
    if (!self || self->no_draw ||!self->model) return;

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
        entity_manager.entity_list[i].no_draw = 0;

        return &entity_manager.entity_list[i];
    }
    slog("no more entity slots");
    return NULL; // no more entity slots
}

void entity_free(Entity *self){
    // check if pointer is null
    if (!self) return;
   
    if (self->free) self->free(self);

    // free up anything that may have been allocated FOR this
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
    
    //asteroid_free();

    // despawn all projectiles next
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

        if (target->entity_type == PLAYER || target->entity_type == ENEMY || target->entity_type == ASTEROID)
            continue;

        entity_free(target);
    }

    // despawn everything except player and reticle
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