#ifndef LEVELS_H
#define LEVELS_H


extern void level2_processing();
extern char* load_level_background();
extern unsigned char move_to_level_if_needed();
extern inline unsigned char is_on_door(int door_x);
#endif
