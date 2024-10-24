#include <math.h>
#include "simple_logger.h"
#include "gf3d_vgraphics.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gfc_vector.h"
#include "gf2d_mouse.h"
#include "ui.h"

static ShopData* shop_data;

void shop_init() {
    GFC_Vector2D res;

    shop_data = gfc_allocate_array(sizeof(ShopData), 1);
    if (!shop_data) return;

    res = gf3d_vgraphics_get_resolution();

    shop_data->shields_block = gfc_rect(res.x/4, res.y/4, 200.0, 100.0);

    atexit(shop_free);
}

void shop_free() {
    free(shop_data);
    shop_data = NULL;
}

void shop_hud_draw(PlayerData* data) {
    GFC_Vector2D res;
    
    res = gf3d_vgraphics_get_resolution();
    gf2d_draw_rect_filled(gfc_rect(0, 0, res.x, res.y), data->shop_color);

    if (data->shop_color_hue + 0.5 > 360.0)
        data->shop_color_hue = 0.0;
    else {
        data->shop_color_hue += 0.5;
        data->shop_color_hue = roundf(10 * data->shop_color_hue) / 10;
    }

    gfc_color_set_hue(data->shop_color_hue, &(data->shop_color));
    gf2d_draw_rect(gfc_rect(0, 0, res.x, res.y), GFC_COLOR_BLACK);

    gf2d_font_draw_line_tag("SHOP", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(605.0, 100.0));

    //gf2d_font_draw_line_tag("SHOP", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(605.0, 100.0));
    
    gf2d_draw_rect_filled(shop_data->shields_block, GFC_COLOR_BLACK);
    gf2d_font_draw_text_wrap_tag("SHIELDS", FT_H2, GFC_COLOR_WHITE, shop_data->shields_block);
}

void shop_think(PlayerData* data) {
    if (!data) return;

    // shield upgrade
    if (gf2d_mouse_button_pressed(0) && gfc_point_in_rect(gf2d_mouse_get_position(), shop_data->shields_block)) {
        if (data->shields_check >= 10) {
            slog("max amount of upgrades for shields");
            return;
        }

        
        if (data->currScrap >= 5) {
            data->maxShield += 50.0;
            data->currScrap -= 5;
            data->shields_check++;
            slog("more shields");
        }
        else
            slog("not enough scrap");
    }
}

void player_hud(PlayerData* data) {
    float start, scrap, maxscrap, maxshield, shield;
    int scrap_line_count, new_x, i;

    if (!data) return;
    if (data->player_dead || data->in_shop) return;


    shield = data->currShield;
    maxshield = data->maxShield;

    scrap = (float) data->currScrap;
    maxscrap = (float) data->maxScrap;

    // health bar draws
    gf2d_draw_rect_filled(gfc_rect(10, 20, 400.0, 30), GFC_COLOR_BLACK);

        // current health
    gf2d_draw_rect_filled(gfc_rect(10, 20,(float) 400.0 * (data->currHealth / data->total_health_bar), 30), GFC_COLOR_RED);

        // current shield
    gf2d_draw_rect_filled(gfc_rect(10 + (400.0 * (data->currHealth / data->total_health_bar)), 20, (float)  400.0 * (data->currShield / data->total_health_bar), 30), GFC_COLOR_DARKBLUE);

        // bar outline
    gf2d_draw_rect(gfc_rect(10, 20, 400, 30), GFC_COLOR_WHITE);
    gf2d_font_draw_line_tag("HEALTH", FT_H5, GFC_COLOR_WHITE, gfc_vector2d(15, 23));

    // scrap bar draws
        // current scrap
    gf2d_draw_rect_filled(gfc_rect(10, 60, 400.0, 30), GFC_COLOR_BLACK);
    gf2d_draw_rect_filled(gfc_rect(10, 60, 400.0 * (scrap / maxscrap), 30), GFC_COLOR_GREY);

        // bat outline
    scrap_line_count = (int)data->maxScrap / data->max_missile;
    start = 10.0;
    if (data->maxScrap % data->max_missile != 0) scrap_line_count++;
    for (i = scrap_line_count; i > 0; i--) {
        new_x = (int) 400.0 / scrap_line_count;
        gf2d_draw_rect(gfc_rect(start, 60, 400.0 / scrap_line_count, 30), GFC_COLOR_WHITE);
        start += new_x;
    }
    gf2d_draw_rect(gfc_rect(10, 60, 400, 30), GFC_COLOR_WHITE);
    gf2d_font_draw_line_tag("SCRAP", FT_H5, GFC_COLOR_WHITE, gfc_vector2d(15, 63));
}

void enemy_hud(EnemyData* data, GFC_Vector3D position) {
    float health, maxhealth;
    PlayerData* player_data;
    GFC_Vector2D bar_position;

    if (!data) return;
    
    player_data = data->player_data;
    if (player_data->player_dead || player_data->in_shop || data->currHealth <= 0.0) return;

    health = data->currHealth;
    maxhealth = data->maxHealth;

    bar_position = gfc_3DPos_to_2DPos(position, data->x_bound, data->z_bound);
    
    // bar offset (maybe testing the scaling for other res???)
    bar_position.x += 100.0;
    bar_position.y += 50.0;
    bar_position.x *= 0.8;
    bar_position.y *= 0.8;

    gf2d_draw_rect_filled(gfc_rect(bar_position.x, bar_position.y, 100.0, 20), GFC_COLOR_BLACK);
    gf2d_draw_rect_filled(gfc_rect(bar_position.x, bar_position.y, 100.0 * (health / maxhealth), 20), GFC_COLOR_DARKBLUE);
    gf2d_draw_rect(gfc_rect(bar_position.x, bar_position.y, 100.0, 20), GFC_COLOR_WHITE);
}

void enemy_hud_all() {
    Entity* entityList, *enemy;
    int i;

    entityList = get_entityList();
    for (i = 0; i < MAX_ENTITY; i++) {
        enemy = &entityList[i];

        if (enemy->entity_type != ENEMY)
            continue;

        enemy_hud(enemy->data, enemy->position);
    }
}

void pause_menu(PlayerData* data) {
    GFC_Vector2D res;

    res = gf3d_vgraphics_get_resolution();
    gf2d_draw_rect_filled(gfc_rect(0, 0, res.x, res.y), gfc_color(65, 65, 65, 0.4));

    gf2d_font_draw_line_tag("GAME PAUSED", FT_H1, GFC_COLOR_WHITE, gfc_vector2d(res.x/2 - 90.0, res.y / 2 - 20.0));
}

void player_death_screen(PlayerData* data) {

}