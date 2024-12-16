#include "simple_logger.h"
#include "gfc_config.h"
#include "gf2d_font.h"
#include "gf2d_sprite.h"
#include "world.h"
#include "player.h"
#include "notifications.h"

typedef struct NotifData_S {
    /*notifications*/
    SJson*      notif_data;
    Sprite*     notif_block;
    Sprite*     notif_dur;
}NotifManager;

static NotifManager notif_manager = {0};

void notif_close() {
    sj_free(notif_manager.notif_data);
    gf2d_sprite_free(notif_manager.notif_block);
    gf2d_sprite_free(notif_manager.notif_dur);
    memset(&notif_manager, 0, sizeof(NotifManager));
}

void notif_init() {
    notif_manager.notif_data = sj_load("def/notifications.def");
    notif_manager.notif_block = gf2d_sprite_load_image("images/UI/notifications/notif_bar.png");
    notif_manager.notif_dur = gf2d_sprite_load_image("images/UI/notifications/notif_duration.png");
    atexit(notif_close);
}

void notif_window(NotifType notif_type) {
    SJson* notif, * data;
    PlayerData* p_data;
    WorldData* world;
    char buffer[30];
    int index;
    float width;
    GFC_Color color;
    GFC_Vector2D scale, offset;

    world = get_world_data();

    if (!world->notif_flag)
        return;

    if (!world->notif_init) {
        notif_init();
        world->notif_init = 1;
    }

    data = sj_object_get_value(notif_manager.notif_data, "notifs");
    index = (int) notif_type;

    notif = sj_array_get_nth(data, index);

    if (!notif) {
        slog("nope");
        return;
    }

    p_data = get_player_data();

    strcpy(buffer, sj_object_get_value_as_string(notif, "text"));

    sj_value_as_vector2d(sj_object_get_value(notif, "notif_rect_offset"), &offset);
    color = sj_object_get_color(notif, "color");

    gf2d_sprite_draw_image(notif_manager.notif_block, offset);
    gf2d_font_draw_line_tag(buffer, FT_Large, GFC_COLOR_WHITE, offset);

    // duration meter
    sj_value_as_vector2d(sj_object_get_value(notif, "notif_dur_offset"), &offset);

    if (notif_type == HAPPYTRIG_POWERUP || notif_type == INVINCE_POWERUP) {
        width = notif_type == HAPPYTRIG_POWERUP ?
            (p_data->item_duration - CURRENT_TIME) / p_data->powerup_dur :
            (p_data->item_duration - CURRENT_TIME) / p_data->powerup_dur;
    }
    else if (notif_type == NO_SCRAP || notif_type == DUPE_PERKS || 
        notif_type == NO_RUNS || notif_type == LEVEL_SAVED || notif_type == NOT_ENOUGH_ENEMIES)
        width = (world->notification_time - CURRENT_TIME) / NOTIF_TIME_MAX;
    else
        width = 0;

    scale = gfc_vector2d(width, 1.0f);
    if (width > 0) {
        gf2d_sprite_draw(notif_manager.notif_dur, offset, &scale,
                        NULL, NULL, NULL, NULL, NULL, NULL);
    }
    else 
        world->notif_flag = 0;
}