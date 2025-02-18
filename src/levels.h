#ifndef LEVELS_H
#define LEVELS_H
#include <allegro.h>

#define VELOCIDAD_BASE 0
#define VELOCIDAD_MAX 5
#define DECAY_RATE 1
#define DERECHA 1
#define IZQUIERDA -1


extern char beep_count;
extern unsigned char missed_beeps;
extern char beep_side;

void level_processing();
BITMAP * load_level_background(unsigned char lvl);
unsigned char move_to_level_if_needed();
inline unsigned char is_on_door(int door_x);
void move_with_level_limits();
#endif
