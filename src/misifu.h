#ifndef MISIFU_H
#define MISIFU_H
#include "allegro/gfx.h"

#define NONE 0
#define M_JUMPING 3
#define M_FALLING 4
#define M_WALKING_LEFT 1
#define M_WALKING_RIGHT 2
#define M_JUMPING_PUSHED 11
#define M_SWIMMING 12
#define M_JUMP_UP  0
#define M_JUMP_RIGHT 1
#define M_JUMP_LEFT 12

#define OFF_RIGHTC1 0
#define OFF_RIGHTC2 1
#define OFF_JRIGHT  2
#define OFF_JUMPING 3
#define OFF_BORED   4
#define OFF_HANGING 5
#define OFF_FALL 6

#define BINCAT_Y 140
#define HIGHER_BIN1_X 190
#define HIGHER_BIN2_X 70
#define BIN_HIGH_Y 128
#define BIN_LOW_Y 138

#define WALK_CYCLE 8  // Cambia el sprite cada 8 píxeles

#define HIGHER_BIN1 2   // bins are from 1 - 5
#define HIGHER_BIN2 4



#define LEVEL_MAX 265
#define LEVEL_MIN 30
#define FALLING_FLOOR 69

#define CAT_IN_ROPE 6
#define CAT_IN_ROPE1 7
#define CAT_IN_ROPE2 8
#define CAT_IN_ROPE3 9
#define CAT_ON_HIGH 10
#define CAT_IN_BIN 3
#define CAT_IN_FENCE 4
#define FLOOR_Y 165

BITMAP * load_misifu_data();
void misifu_process();
void misifu_output();
void destroy_misifu_data();

#endif
