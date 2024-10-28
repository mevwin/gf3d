#include "simple_logger.h"
#include "gfc_list.h"
#include "lights.h"

typedef struct {
    GFC_List*  lightList;
}LightManager;

static LightManager light_manager = { 0 };

void lights_init() {
    light_manager.lightList = gfc_list_new();
    if (!light_manager.lightList)
        slog("failed to allocate lightList");
    
    atexit(lights_close);
}

void lights_close() {
    if (light_manager.lightList) {
        gfc_list_foreach(light_manager.lightList, (gfc_work_func*)free);
        gfc_list_delete(light_manager.lightList);
    }
}

void light_free(Light* light) {
    if (!light) return;

    gfc_list_delete_data(light_manager.lightList, light);
    free(light);
}

Light* light_new() {
    Light* light;
    light = gfc_allocate_array(sizeof(Light), 1);

    if (!light) return NULL;

    gfc_list_append(light_manager.lightList, light);
    return light;
}