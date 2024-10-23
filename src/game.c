#include <SDL.h>            

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"
#include "gfc_string.h"
#include "gfc_actions.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "gf3d_texture.h"
#include "gf3d_draw.h"

#include "entity.h"
#include "player.h"
#include "enemy.h"
#include "shop.h"

extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 33;
static float fps = 0;

void parse_arguments(int argc,char *argv[]);
void game_frame_delay();

void exitGame()
{
    _done = 1;
}

void draw_origin()
{
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(-100,0,0),gfc_vector3d(100,0,0)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(1,0,0,1));
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(0,-100,0),gfc_vector3d(0,100,0)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(0,1,0,0));
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(0,0,-100),gfc_vector3d(0,0,100)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(0,0,1,1));
}

int main(int argc,char *argv[])
{
    //local variables
    Model *sky;
    GFC_Matrix4 skyMat;
    Entity* player, * enemy;
    PlayerData* player_data;

    //initializtion    
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0); //append_mode is 0, meaning the file overwrites the previous
    slog("gf3d begin"); //writes the state to the log
    gfc_config_def_init();

    //gfc init
    gfc_input_init("config/input.cfg");
    gfc_action_init(1024); //setups/inits vulkan graphics subsystems

    //gf3d init
    gf3d_vgraphics_init("config/setup.cfg"); 
    gf3d_materials_init();
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);
    gf3d_draw_init();//3D, draws origin
    gf2d_draw_manager_init(1000);//2D

    //entity init
    entity_system_init(MAX_ENTITY);

    //game init
    srand(SDL_GetTicks()); 
    slog_sync();

    //game setup
    //gf2d_mouse_load("actors/mouse.actor");
    sky = gf3d_model_load("models/sky.model");
    gfc_matrix4_identity(skyMat);
    

    //camera, definitely needs change for player entity
    gf3d_camera_set_scale(gfc_vector3d(1,1,1));
    gf3d_camera_set_position(gfc_vector3d(15,-15,10));
    gf3d_camera_look_at(gfc_vector3d(0,0,0),NULL);
    gf3d_camera_set_move_step(0.6);
    gf3d_camera_set_rotate_step(0.05);
    
    //gf3d_camera_enable_free_look(1);

    //player initialization
    enemy_count = 0;

    player = player_spawn();
    enemy = enemy_spawn(&(player->position), player->data);
    player_data = player->data;
    
    //windows
    //gf2d_draw_rect_filled(gfc_rect(player->position.x, player->position.y, 10, 20), gfc_color(1, 0, 0, 1));
    
   
    // main game loop, constant series of updates  
    while(!_done)
    {
        gfc_input_update(); //look here for SDL stuff
        gf2d_mouse_update();
        gf2d_font_update();
        entity_think_all();
        entity_update_all();

        //camera updates
        gf3d_camera_controls_update(); //arrow and ASDW are registered here
        gf3d_camera_update_view();
        gf3d_camera_get_view_mat4(gf3d_vgraphics_get_view_matrix());

        gf3d_vgraphics_render_start(); // combines all draw commands, then submits
                //3D draws
                gf3d_model_draw_sky(sky,skyMat,GFC_COLOR_WHITE);
                entity_draw_all();
                draw_origin();
             
                //2D draws
                //gf2d_mouse_draw();
                //gf2d_font_draw_line_tag("ALT+F4 to exit",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));
                //gf2d_font_draw_text_wrap_tag("ALT+F4 to exit", FT_Normal, gfc_color(0, 1, 0, 1), gfc_rect(player->position.x, player->position.y, 10, 20));
                
                // game updates
                if (gfc_input_command_pressed("shop")) {
                    if (!player_data->in_shop)
                        player_data->in_shop = 1;
                    else
                        player_data->in_shop = 0;
                }
                if (player_data->in_shop) {
                    gf2d_mouse_draw();
                    shop_hud(player_data);
                }
                else {
                    player_hud(player, player->data);
                }
                if (enemy_count < 2 && !player_data->in_shop)
                    enemy = enemy_spawn(&(player->position), player->data);

        gf3d_vgraphics_render_end();
        if (gfc_input_command_down("exit"))_done = 1; // exit condition


        

        game_frame_delay();
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    exit(0);
    slog_sync();
    return 0;
}

void parse_arguments(int argc,char *argv[])
{
    int a;

    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"--debug") == 0)
        {
            __DEBUG = 1;
        }
    }    
}

void game_frame_delay()
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    then = now;
    slog_sync();// make sure logs get written when we have time to write it
    now = SDL_GetTicks();
    diff = (now - then);
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then,0.001);
//     slog("fps: %f",fps);
}
/*eol@eof*/
