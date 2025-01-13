#include "game.h"
#include "allegro/inline/draw.inl"
#include "allegro/keyboard.h"
#include "allegro/text.h"
#include "dat_manager.h"
#include "enem.h"
#include "tiles.h"

#include "helpers.h"
#include <stdlib.h>
#include <stdio.h>

int exit_game;
spritePos player;

unsigned char level = 0;
unsigned char next_level = FALSE;
unsigned char level_enemies = 0;
unsigned char starting_level_counter = FALSE;
int counter = 0;
char space_was_pressed = FALSE;
// BITMAP *player[11];
// BITMAP *enemy1[11];
BITMAP *bg;
BITMAP *tiles;
char slow_cpu;
LevelData levels[TOTAL_LEVELS];

void input() {
    // readkey();
    // end_game = 1;

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

    if (player.is_hit > 0 || player.is_floor > 0) {
        return;
    }
    if (!key[KEY_SPACE] && player.is_punching > 0) {
        player.is_punching = 0;
    }

    // now check again keys
    if (key[KEY_SPACE]) {
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

// returns true if player is >= or <= a margin on a door
inline char is_on_door(int door_x) {
    if (door_x == 0) {
        return FALSE;
    }
    return (player.x >= door_x && player.x <= (door_x + 50));
}

inline unsigned char move_to_level_if_needed() {
    if (level == 0) {
        return FALSE;
    }
    LevelData curr_leveldata = levels[level];
    if (player.y < 142) { 
        // for doors stays on same level but with subdoors
        if (is_on_door(curr_leveldata.door1Pos)) {
            next_level = curr_leveldata.door1;
            
            return TRUE;
        } else if (is_on_door(curr_leveldata.door2Pos)) {
            next_level = curr_leveldata.door2;
            return TRUE;
        } 
    }
    // right and left sides
    if (curr_leveldata.right != 0 && player.x >= 318) {
        next_level = curr_leveldata.right;
        return TRUE;
    } else if (curr_leveldata.left != 0 && player.x <= 20) {
        next_level = curr_leveldata.left;
        return TRUE;
    }
    return FALSE;    
}


void increase_level_and_load() {
    if (level >= 1) {
        return;
    }
    clear_to_color(screen, 0);
    textout_centre_ex(screen, font, "Loading Level...", SCREEN_W / 2,
                      SCREEN_H / 2, makecol(255, 255, 255), -1);

    next_level = 1;
    load_level();
}

void process() {
    // https://code.tutsplus.com/building-a-beat-em-up-in-game-maker-part-1-player-movement-attacks-and-basic-enemies--cms-26147t
    // https://code.tutsplus.com/building-a-beat-em-up-in-game-maker-part-2-combat-and-basic-enemy-ai--cms-26148t
    // https://code.tutsplus.com/building-a-beat-em-up-in-game-makercombo-attacks-more-ai-and-health-pickups--cms-26471t
    // delete sprite
    LevelData curr_leveldata = levels[level];
    if (player.moving == MOVING_RIGHT && player.x < curr_leveldata.maxX) {
        if (!enemy_on_path(player.x + 1, player.y)) {
            player.x++;
        }
    } else if (player.moving == MOVING_LEFT && player.x > curr_leveldata.minX) {
        if (!enemy_on_path(player.x - 1, player.y)) {
            player.x--;
        }
    }

    if (player.y_moving == MOVING_DOWN && player.y < 151) {
        player.y++;
    } 
    
    if (player.y_moving == MOVING_UP && player.y > 140) {
        player.y--;
    }

    // check door opening or side moving
    if (key[KEY_RCONTROL] || key[KEY_LCONTROL]) {
        if (move_to_level_if_needed()) {
            load_level();
        }
    }

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
        } else {
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
    // clean
    blit(bg, screen, player.x, 120, player.x, 120, 40, 80);
    redraw_bg_enemy_positions();

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
   for (alpha=0; alpha<256; alpha+=8) {
      set_trans_blender(0, 0, 0, alpha);
      draw_trans_sprite(buffer, bg,
			(SCREEN_W-bg->w)/2, (SCREEN_H-bg->h)/2);
      vsync();
      blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);      
   }

   blit(bg, screen, 0, 0, (SCREEN_W-bg->w)/2, (SCREEN_H-bg->h)/2,
	bg->w, bg->h);

   destroy_bitmap(buffer);
}

void init_level_1(unsigned int initialX, unsigned int initialY) {
    player.x = initialX;
    player.y = initialY;
    level_enemies = 1;
    //player.x = 16;
    //player.y = 130;
    player.moving = STOP_RIGHT;
    player.y_moving = 0;
    player.curr_sprite = 0;
    player.is_hit = FALSE;
    player.is_floor = FALSE;
    player.received_hits = 0; // TODO remove
    player.lives = 3; // TODO remove
    player.floor_times = 0;
    starting_level_counter = 20; // TODO depends on level
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
    if (next_level == 0) {
        bg = load_pcx("bege.pcx", NULL);
        level_enemies = 0;
    } else if (next_level >= 1) {
        load_tiles();
        
        char level_filename[17];
        sprintf(level_filename, "bg%d.tmx", next_level);
        bg = load_background(level_filename);
        LevelData curr_leveldata = levels[next_level];
        unsigned int initialX, initialY;
        
        if (next_level < level) { // coming back
            initialX = levels[next_level].door1Pos;
            initialY = curr_leveldata.initialY;
        } else {
            initialX = curr_leveldata.initialX;
            initialY = curr_leveldata.initialY;
        }
        level = next_level;
        init_level_1(initialX, initialY);
    }
    if (!bg) {
        die("Cannot load graphic");        
    }
    if (slow_cpu) {
        blit(bg, screen, 0, 0, 0, 0, 320, 200);
    } else {
        show_bg();
    }
    //

    init_level_enemies(level_enemies, FALSE);
}

/**
blit
– Stands for “BLock Transfer”
– Most important graphics function
– Takes a rectangular block of one bitmap and draws it onto
another
**/
