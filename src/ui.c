#include "simple_logger.h"
#include "SDL_scancode.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gfc_vector.h"
#include "gf2d_mouse.h"
#include "ui.h"

void shop_hud(PlayerData* data) {
	gf2d_draw_rect_filled(gfc_rect(10, 20, 400.0, 30), GFC_COLOR_BLACK);
}

void player_hud(PlayerData* data) {
    float health, maxhealth, start;
    int scrap_line_count, new_x, i, scrap, maxscrap;

    if (!data) return;
    if (data->player_dead || data->in_shop) return;

    health = data->currHealth;
    maxhealth = data->maxHealth;

    scrap = data->currScrap;
    maxscrap = data->maxScrap;

    // health bar draws
        // current health
    gf2d_draw_rect_filled(gfc_rect(10, 20, 400.0, 30), GFC_COLOR_BLACK);
    gf2d_draw_rect_filled(gfc_rect(10, 20, 400.0 * (health / maxhealth), 30), GFC_COLOR_DARKBLUE);

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
    GFC_Vector2D bar_position, res;
    const Uint8* keys;

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

    // TODO: remove this debug tool later
    keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_V])
        slog("bar_pos.x: %f, bar_pos.y: %f", bar_position.x, bar_position.y);

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