#ifndef GAME_H
#define GAME_H

#include <allegro.h>
#include <allegro/gfx.h>
#include <allegro/keyboard.h>

#define TRANS makecol(255, 0, 255)
#define STOPPOS 100
#define STOP_LEFT 101
#define STOP_RIGHT 102
#define MOVING_LEFT 1
#define MOVING_RIGHT 2
#define CROUCH 50
#define JUMPING 52
#define JUMPING_LEFT 3
#define JUMPING_RIGHT 4
#define PUNCH_LEFT 5
#define PUNCH_RIGHT 6

// enemy states
#define ST_NONE 0
#define ST_IDLE 1
#define ST_POSITION_FRONT 2
#define ST_POSITION_BACK 3

#define ANIM_WALK1 4
#define ANIM_WALK2 5
#define ANIM_PUNCH 1
// TODO define maximum vertical distance to allow before two objects can no
// longer interact

struct spritePos {
    unsigned int x;
    unsigned int y;
    BITMAP *sprite[11];
    unsigned int moving;
    unsigned int curr_sprite;
    unsigned char is_hit;
};

struct enemyData {
    unsigned int x;
    unsigned int y;
    BITMAP *sprite[11];
    unsigned int moving;
    unsigned targetX;
    unsigned targetY;
    unsigned int curr_sprite;
    unsigned char is_hit;
};

extern int exit_game; /* flag we'll set to end the game */
extern struct spritePos player;
extern struct enemyData enem1;
extern int counter;
// extern int moving;
// extern BITMAP *player[11];
// extern BITMAP *enemy1[11];
extern BITMAP *bg;

extern void input();
extern void process();
extern void output();

#endif
