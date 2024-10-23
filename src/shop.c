#include "simple_logger.h"
#include "gfc_input.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gfc_vector.h"
#include "gf2d_mouse.h"
#include "shop.h"

void shop_hud(PlayerData* data) {
	gf2d_draw_rect_filled(gfc_rect(10, 20, 400.0, 30), GFC_COLOR_BLACK);
}