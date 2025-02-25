/*
 * Copyright (C) 2016, Michiel Sikma <michiel@sikma.org>
 * MIT License
 */
#include <time.h>
#include <allegro.h>
#include <stdio.h>
#include "allegro/alcompat.h"
#include "allegro/color.h"
#include "allegro/datafile.h"
#include "allegro/digi.h"
#include "allegro/gfx.h"
#include "allegro/keyboard.h"
#include "allegro/system.h"
#include "game.h"
#include "helpers.h"
#include "tiles.h"
#include "dat_manager.h"

static volatile int update_count, frame_count, fps = 0;

void gfx_timer_proc(void) { update_count = 1; }
END_OF_FUNCTION(gfx_timer_proc)

static void gfx_fps_proc(void) {
    fps = frame_count;
    frame_count = 0;
}
END_OF_FUNCTION(gfx_fps_proc)

void gfx_init_timer() {
    LOCK_VARIABLE(update_count);
    LOCK_VARIABLE(frame_count);
    LOCK_VARIABLE(fps);
    LOCK_FUNCTION(gfx_timer_proc);
    LOCK_FUNCTION(gfx_fps_proc);
    install_int_ex(gfx_timer_proc, BPS_TO_TIMER(70));
    install_int_ex(gfx_fps_proc, BPS_TO_TIMER(1));
}
static volatile long speed_counter = 0;

void increment_speed_counter()
{
    speed_counter++;
}
END_OF_FUNCTION(increment_speed_counter);


/**
 * Allegro example script. Switches to graphics mode to print "hello world",
 * then waits for a keypress and exits the program.
 * Taken from <https://wiki.allegro.cc/index.php?title=Example_ExHello>.
 * http://www.glost.eclipse.co.uk/gfoot/vivace/vivace.html
 */
/* timer callback for measuring the frames per second */
static void fps_proc(void) {
    fps = frame_count;
    frame_count = 0;
}

END_OF_STATIC_FUNCTION(fps_proc);


int main(int argc, const char **argv) {
    char file_buffer[14];
    BITMAP *bmp;
    
    // Initializes the Allegro library.
    if (allegro_init() != 0) {
        return 1;
    }

    // Installs the Allegro keyboard interrupt handler.
    install_keyboard();
    install_timer();

    //bmp = create_bitmap(640, 480);
    set_color_conversion(COLORCONV_KEEP_ALPHA);
    // Switch to graphics mode, 320x200.
    set_color_depth(8);

    if (set_gfx_mode(GFX_MODEX, 320, 200, 0, 0) != 0) {
        die("Cannot set graphics mode");
    }

    install_int(fps_proc, 1000);    
    // Print a single line of "hello world" on a white screen.
    //set_palette(desktop_palette);

    //set_color_depth(desktop_color_depth());
    slow_cpu = is_cpu_slow();
    clear_to_color(screen, 0);
    textout_centre_ex(screen, font, "Loading Instituto Rio Immaculado...", SCREEN_W / 2, SCREEN_H / 2, makecol(255,255,255), -1);
    extract_data(); // todo mover despues de textout

    load_levels();
    set_color_depth(15);
    //try GFX_MODEX
    if(set_gfx_mode(GFX_AUTODETECT, 320, 240, 0, 0) != 0) {
        die("error setting 320x240 16bpp: %s", allegro_error);
    }

    music = load_midi("ROGERR.MID");
    alleytheme = load_wav("alleytheme.wav");
    hit = load_wav("hit.wav");
    dog_theme = load_wav("dog.wav");

    if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) != 0) {
        die("Error: inicializando sistema de sonido\n%s\n", allegro_error);
    }
    set_volume(230, 150);

    srand(time(NULL));
    // load tilemap
    load_tiles();
    set_pallete(palette);

    for (int i = 0; i < 12; i++) {
        sprintf(file_buffer, "MAIN%d.PCX", i + 1);
        player.sprite[i] = load_pcx( file_buffer, NULL );
        if(!player.sprite[i]) {
            die("Cannot load %s", file_buffer);
        }
    }
    player_head = load_pcx( "LIVES.PCX", NULL );
    player_lifebar = load_pcx("LIFEBAR.PCX", NULL);
    girl = load_pcx("GIRL1.PCX", NULL);
    key_sprite = load_pcx( "KEY.PCX", NULL );

    // pre load enemies sprites
    init_level_enemies(0, 300, TRUE);
    // will load menu
    next_level = 0;
    load_level();
    

    exit_game = 0;               /* reset flag */
    player.x = 16;
    player.y = 130;
    player.lifebar = LIFEBAR;
    player.moving = STOP_RIGHT;
    player.y_moving = 0;
    player.curr_sprite = 0;
    player.is_hit = FALSE;
    player.is_floor = FALSE;
    player.received_hits = 0;
    player.lives = 3;
    player.floor_times = 0;

    gfx_init_timer();

    do {
        /*while (speed_counter > 0) {
            rest(0);
            if (key[KEY_ESC]) {
                exit_game = 1;
                break;
            }
            speed_counter--;
        }*/

        while (0 == update_count) {
            rest(0);
            if (key[KEY_ESC]) {
                exit_game = 1;
                break;
            }
            vsync();
        }
        update_count = 0;
        frame_count++;
        
        if (level == MENU) {
            if (key[KEY_SPACE]) {
                increase_level_and_load();
                if (play_looped_midi(music, 0, -1) != 0) {
                    die("Cant play music");
                }
            }
        } else if (level == GAME_OVER) {
            if (key[KEY_SPACE]) {
                level = 0;
                next_level = 0;
                load_level();
            }
        } else {
            if (starting_level_counter == FALSE) {
                input();   /* get input */
                process(); /* process it */
            } else {
                if ((counter % 2) == 0) {
                    player.y++;
                    if (counter % 10 == 0) {
                        player.curr_sprite ^= 1; // varies last digit 0/1, 1/0
                    }
                    starting_level_counter--;
                }
            }
            if (level != MENU) {
                output();  /* give output */
            }
            vsync();
        }

        if (key[KEY_ESC]) {
            exit_game = 1;
        }

    } while (exit_game == 0); /* until the flag is set */

    destroy_bitmap(bg);
    for (int i = 0; i < 12; i++) {
        destroy_bitmap(player.sprite[i]); 
    }
    destroy_bitmap(player_head);
    destroy_bitmap(player_lifebar);
    destroy_bitmap(girl);
    destroy_bitmap(key_sprite);
    destroy_sample(alleytheme);
    destroy_sample(hit);
    unload_enemies();
    destroy_tiles();
    cleanup_data();
    destroy_midi(music);
    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
  
    return 0;
}

END_OF_MAIN()
