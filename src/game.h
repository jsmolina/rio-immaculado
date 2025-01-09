#ifndef GAME_H
#define GAME_H

#include "enem.h"
#include "helpers.h"
#include <allegro/keyboard.h>

#define TRANS makecol(255, 0, 255)

// TODO define maximum vertical distance to allow before two objects can no
// longer interact

extern int exit_game; /* flag we'll set to end the game */
extern unsigned char level;
extern unsigned char sub_level;
extern unsigned char next_level;
extern unsigned char starting_level_counter;
extern unsigned char level_enemies;
extern spritePos player;

extern int counter;
// extern int moving;
// extern BITMAP *player[11];
// extern BITMAP *enemy1[11];
extern BITMAP *bg;
extern char slow_cpu;

// processes user input
extern void input();
// processes state changes, e.g., based on user input
extern void process();
// syncs current state to the screen
extern void output();
// setups next level
extern void increase_level_and_load();
// opens the levels.nfo file which includes actionable parameters for each level
extern void load_levels();
// opens the current playable level
extern void load_level();


#endif

/**
Niveles:
1: clases
2: cours navé
3: sala informática: alleycat piscina
4: parking
5: calle
 */