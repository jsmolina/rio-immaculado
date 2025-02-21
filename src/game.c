#include "game.h"
#include "allegro/gfx.h"
#include "allegro/inline/draw.inl"
#include "allegro/keyboard.h"
#include "allegro/text.h"
#include "dat_manager.h"
#include "enem.h"
#include "tiles.h"
#include "helpers.h"
#include "levels.h"
#include "misifu.h"

#include <stdlib.h>
#include <stdio.h>

int exit_game;
spritePos player;

unsigned char level = 0;
unsigned char next_level = FALSE;
unsigned char level_enemies = 0;
unsigned char starting_level_counter = 0;
char coursnave_completed = FALSE;
char yellow_key = FALSE;
char locked_elevator;
char urinated;
MIDI *music;
int counter = 0;
char space_was_pressed = FALSE;
// BITMAP *player[11];
// BITMAP *enemy1[11];
BITMAP *bg;
BITMAP *tiles;
BITMAP *player_head;
BITMAP *girl;
BITMAP *key_sprite;
BITMAP *player_lifebar;
char slow_cpu;
LevelData levels[TOTAL_LEVELS];

void input() {
    // readkey();
    // end_game = 1;
    if (level == 12 || (level == 8 && !coursnave_completed)) {
        return;
    }
    if (player.moving != LOOKING_WALL) {
        if (player.moving < STOPPOS) {
            if (player.moving == MOVING_LEFT || player.moving == PUNCH_LEFT) {
                player.moving = STOP_LEFT;
            } else {
                player.moving = STOP_RIGHT;
            }
        }

        if (player.y_moving < STOPPOS) {
            player.y_moving = STOPPOS;
        }
    }

    if (player.is_hit > 0 || player.is_floor > 0) {
        return;
    }
    unsigned char pressing_control = key[KEY_RCONTROL] || key[KEY_LCONTROL];
    if (!pressing_control && player.is_punching > 0) {
        player.is_punching = 0;
    }

    // now check again keys
    if (pressing_control) {
        player.is_punching++;
        if (player.is_punching > 20) {
            return;
        }

        if (player.moving == STOP_LEFT) {
            player.moving = PUNCH_LEFT;
        } else {
            player.moving = PUNCH_RIGHT;
        }
    }

    if (key[KEY_LEFT]) {
        player.moving = MOVING_LEFT;
    } else if (key[KEY_RIGHT]) {
        player.moving = MOVING_RIGHT;
    }
    
    if (key[KEY_UP]) {
        player.y_moving = MOVING_UP;
    } else if (key[KEY_DOWN]) {
        player.y_moving = MOVING_DOWN;
    }
    // TODO ALT/ALTGR for kicks
}


void increase_level_and_load() {
    if (level >= 1) {
        return;
    }
    clear_to_color(screen, 0);
    textout_centre_ex(screen, font, "Loading Level...", SCREEN_W / 2,
                      SCREEN_H / 2, makecol(255, 255, 255), -1);
    locked_elevator = TRUE;
    urinated = FALSE;
    beep_count = -1;
    missed_beeps = 0;
    beep_side = IZQUIERDA;
    coursnave_completed = FALSE;
    yellow_key = FALSE;
    next_level = 15;
    load_level();
}
// draws current player lives
void draw_lives() {
    //blit(bg, screen, 20, 20, 20, 20, 50, 20);
    rectfill(screen, 20, SCREEN_H - 30, 60, SCREEN_H - 20, makecol(40, 40, 40));
    unsigned int i;
    for (i = 0; i < player.lives; i++) {
        draw_sprite(screen, player_head, 20 + i * 10, SCREEN_H - 30);
    }
}

void game_over() {
    textout_centre_ex(screen, font, "GAME OVER", 120, SCREEN_H - 34, makecol(255, 255, 255), makecol(10, 10, 10));
}

void draw_lifebar() {
    rectfill(screen, 70, SCREEN_H - 30, 90, SCREEN_H - 20, makecol(40, 40, 40));
    blit(player_lifebar, screen, 0, 0, 70, SCREEN_H -30, 2 * player.lifebar, 14);
}

void process() {
    if (level == MISIFU_ALLEY || level ==  MISIFU_CHEESE) {
        misifu_process();
        return;
    }

    move_with_level_limits();

    // check door opening or side moving
    if (move_to_level_if_needed()) {
        load_level();
        return;
    }
    if (key[KEY_0]) {
        next_level = GAME_OVER;
        level = GAME_OVER;
        player.is_floor = FLOOR_DURATION;
        game_over();
        return;
    }

    if (player.received_hits == 5) {
        player.is_floor = FLOOR_DURATION;
        player.received_hits = 0;
    }
    if (player.lifebar == 0) {
        player.lives--;
        player.is_floor = FLOOR_DURATION;
        player.lifebar = LIFEBAR;
        draw_lives();
    }

    if (player.lives == 0) {
        next_level = GAME_OVER;
        level = GAME_OVER;        
        player.is_floor = FLOOR_DURATION;
        game_over();
        
        return;
    }

    level_processing();

    if (player.is_hit > 0) {
        player.curr_sprite = ANIM_HITTED;
    }
    

    if ((counter % 10) == 0) {
        if (player.is_floor > 0) {
            player.is_floor--;
            return;
        }

        if (player.is_hit > 0) {
            player.is_hit--;
            return;
        }

        if (player.moving == MOVING_RIGHT || player.moving == MOVING_LEFT ||
            player.y_moving == MOVING_UP || player.y_moving == MOVING_DOWN) {
            if (player.curr_sprite == ANIM_WALK1) {
                player.curr_sprite = ANIM_WALK2;
            } else {
                player.curr_sprite = ANIM_WALK1;
            }
        } else if (player.moving == PUNCH_RIGHT ||
                   player.moving == PUNCH_LEFT) {

            if (player.curr_sprite == ANIM_PUNCH) {
                player.curr_sprite = ANIM_PUNCH2;
            } else {
                player.curr_sprite = ANIM_PUNCH;
            }
            // TODO 15/10: sprite de hit
        } else if (player.moving != LOOKING_WALL) {
            player.curr_sprite = 0;
        }
    }
    ///// ENEMY
    all_enemy_decisions(&player);

    if ((counter % 10) == 0) {
        all_enemy_animations();
    }
}

void draw_player() {
    // redraw pair or impair?
    if (player.is_floor != FALSE) {
        if (player.moving & 1) {
            rotate_sprite(screen, player.sprite[0], player.x, player.y + 10,
                          itofix(1 * 64));
        } else {
            rotate_sprite_v_flip(screen, player.sprite[0], player.x,
                                 player.y + 10, itofix(1 * 64));
        }

    } else {
        if (player.moving & LOOKING_LEFT) {
            draw_sprite_h_flip(screen, player.sprite[player.curr_sprite],
                               player.x, player.y);
        } else {
            draw_sprite(screen, player.sprite[player.curr_sprite], player.x,
                        player.y);
        }
    }
}


void output() {
    counter++;
    if (level == MISIFU_ALLEY || level ==  MISIFU_CHEESE) {
        return;
    }

    if (level == 12) {
        return;
    }
    // clean
    blit(bg, screen, player.x-5, player.y - 10, player.x-5, player.y - 10, 45, 55);
    redraw_bg_enemy_positions();

    if (level == 9 || yellow_key == TRUE) {
        unsigned char y_pos = 105;
        if (yellow_key == TRUE) {
            y_pos = SCREEN_H - 18;
        }
        draw_sprite(screen, key_sprite, 34, y_pos);
    }

    // draw in order depending on Y
    if (player_over_all_enemies(player.y)) {
        draw_player();
        all_draw_enemies();
    } else {
        all_draw_enemies();
        draw_player();
    }

    if (counter > 319) {
        counter = 0;
    }
}


int show_bg() {
   BITMAP *bmp, *buffer;
   PALETTE pal;
   int alpha;
   
   if (!bg)
      return -1;

   buffer = create_bitmap(SCREEN_W, SCREEN_H);
   blit(screen, buffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

   set_palette(pal);

   /* fade it in on top of the previous picture */
   for (alpha=100; alpha<256; alpha+=8) {
      set_trans_blender(0, 0, 0, alpha);
      draw_trans_sprite(buffer, bg,
			(SCREEN_W-bg->w)/2, (SCREEN_H-bg->h)/2);
      vsync();
      blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);      
   }

   blit(bg, screen, 0, 0, (SCREEN_W-bg->w)/2, (SCREEN_H-bg->h)/2,
	bg->w, bg->h);
    //rectfill(screen, 0, 200, 320, 240, makecol(0, 0, 0));


   destroy_bitmap(buffer);
}

void init_level_variables(unsigned int initialX, unsigned int initialY) {
    player.x = initialX;
    player.y = initialY;
    player.moving = STOP_RIGHT;
    player.y_moving = 0;
    player.curr_sprite = 0;
    player.is_hit = FALSE;
    player.is_floor = FALSE;
    player.received_hits = 0; // TODO remove
    player.floor_times = 0;
    if (initialY == 130) {
        starting_level_counter = 20; // simulate leaving elevator
    } else {
        starting_level_counter = 5;
    }
    player.curr_sprite = ANIM_WALK1;
    
}

/**
Loads the whole set of levels
*/
void load_levels() {
    FILE* archivo = fopen("levels.csv", "r");
    if (archivo == NULL) {
        die("Cannot open 'levels.csv'\n");
    }
    // discard first line
    char buffer[150];
    fgets(buffer, 72, archivo);
    /*level,door1Pos,door1,door2Pos,door2,left,right,enemies,initialX,initialY,elevatorPos,elevator, minX,maxX */
    int total_levels = 0;
    int level_index = 1;

    while (fscanf(archivo, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
                  &levels[level_index].level,
                  &levels[level_index].door1Pos,
                  &levels[level_index].door1,
                  &levels[level_index].door2Pos,
                  &levels[level_index].door2,
                  &levels[level_index].left,
                  &levels[level_index].right,
                  &levels[level_index].total_enemies, 
                  &levels[level_index].initialX,
                  &levels[level_index].initialY, 
                  &levels[level_index].elevatorPos,
                  &levels[level_index].elevator,
                  &levels[level_index].minX, 
                  &levels[level_index].maxX) > 2) {
        total_levels++;
        level_index++;
        if (total_levels >= TOTAL_LEVELS) break;
    }

    fclose(archivo);
}



void load_level() {

    if (next_level == 6 && level == 5) {
        BITMAP * bg2;

        bg2 = load_level_background(6);
        for (unsigned int i = 0; i < 200; i+=4) {
            blit(bg2, screen, 0, 0, 0, 200 - i, 320, i);
            blit(bg, screen, 0, i, 0, 0, 320, 200 - i);
            vsync();
            // TODO check speed
            rest(100);
        }
        destroy_bitmap(bg2);
    }
    if (bg) {
        // TODO might crash
        destroy_bitmap(bg);
    }

    if (next_level == 0) {
        bg = load_pcx("bege.pcx", NULL);
        level_enemies = 0;
        player.lives = 3;
        player.lifebar = 10;
        player.floor_times = 0;
    } else if (next_level == 12) {
        bg = load_level_background(next_level);
        level = 12;
    } else if (next_level == MISIFU_ALLEY) {
        bg = load_misifu_data();
        level = next_level;
    } else if (next_level >= 1) {
        bg = load_level_background(next_level);
        LevelData curr_leveldata = levels[next_level];
        unsigned int initialX, initialY;
        
        if (next_level < level) { // coming back
            if (is_on_door(levels[level].door1Pos)) {
                initialX = curr_leveldata.door1Pos;
            } else if (is_on_door(levels[level].door2Pos)) {
                initialX = curr_leveldata.door2Pos;
            } else {
                if (point_distance(player.x, levels[level].minX) < 20) {
                    initialX = curr_leveldata.maxX - 15;
                } else {
                    initialX = curr_leveldata.minX + 15;
                }
            }
            initialY = curr_leveldata.initialY;
        } else {
            initialX = curr_leveldata.initialX;
            initialY = curr_leveldata.initialY;
        }
        level = next_level;
        player.x = initialX;
        player.y = initialY;
        player.moving = STOP_RIGHT;
        player.y_moving = 0;
        player.curr_sprite = 0;
        player.is_hit = FALSE;
        player.is_floor = FALSE;
        player.received_hits = 0; // TODO remove
        player.floor_times = 0;
        if (initialY == 130) {
            starting_level_counter = 20; // simulate leaving elevator
        } else {
            starting_level_counter = 5;
        }
        player.curr_sprite = ANIM_WALK1;

        level_enemies = curr_leveldata.total_enemies;
        init_level_enemies(level_enemies, curr_leveldata.maxX, FALSE);
    }
    if (!bg) {
        die("Cannot load graphic");        
    }
    //blit(bg, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    if (slow_cpu) {
        blit(bg, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    } else {
        if (!(next_level == 6 && level == 5)) {
            show_bg();
        }
    }

    //rectfill(screen, 0, 200, 320, 240, makecol(0, 0, 0));
    if (level != 0) {
        draw_lives();
        draw_lifebar();
    }
}

/**
blit
– Stands for “BLock Transfer”
– Most important graphics function
– Takes a rectangular block of one bitmap and draws it onto
another
**/
