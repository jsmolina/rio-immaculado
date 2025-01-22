#include "enem.h"
#include "allegro/gfx.h"
#include "game.h"
#include "helpers.h"
#include <stdio.h>

enemyData enemies[MAX_ENEMIES];

void eies_sprite(enemyData *enem, unsigned int variant) {
    char file_buffer[14];
    // load enemy1
    for (int i = 0; i < 9; i++) {
        sprintf(file_buffer, "ENEM%d_%d.PCX", variant, i + 1); 
        enem->sprite[i] = load_pcx(file_buffer, NULL);
        enem->variant = variant;
        if (!enem->sprite[i]) {
            die("Cannot load %s", file_buffer);
        }
    }
}

void unload_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        for (int j = 0; j < 9; j++) {
            if (enemies[i].sprite[j]) {
                destroy_bitmap(enemies[i].sprite[j]);
            }
        }
    }
}

void init_level_enemies(int total_enemies, int maxX, char first_load) {
    int i;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (i < total_enemies) {
            enemies[i].is_active = TRUE;
            enemies[i].x = maxX - 60 - i * 10; // it should vary per level
            enemies[i].y = 150; // it should vary per enemy
            enemies[i].targetX = 0;
            enemies[i].targetY = 0;
            enemies[i].curr_sprite = 0;
            enemies[i].is_hit = FALSE;
            enemies[i].is_floor = FALSE;
            enemies[i].is_punching = FALSE;
            enemies[i].received_hits = 0;
            enemies[i].is_active = FALSE;
        } else {
            enemies[i].is_active = FALSE;
            if (first_load == TRUE) {
                eies_sprite(&enemies[i], i % 2 + 1);
            }
        }
    }
}

void enemy_animation(enemyData *enem) {

    if (enem->is_floor != FALSE) {
        return;
    }

    if (enem->is_hit > 0) {
        enem->curr_sprite = ANIM_HITTED;
        enem->is_hit--;
        return;
    }
    if (enem->is_punching > 0) {
        enem->is_punching--;
    }

    if (enem->moving == MOVING_RIGHT || enem->moving == MOVING_LEFT) {
        if (enem->curr_sprite == ANIM_WALK1) {
            enem->curr_sprite = ANIM_WALK2;
        } else {
            enem->curr_sprite = ANIM_WALK1;
        }
    } else if (enem->moving == PUNCH_RIGHT || enem->moving == PUNCH_LEFT) {
        if (enem->curr_sprite == ANIM_PUNCH) {
            enem->curr_sprite = ANIM_PUNCH2;
        } else {
            enem->curr_sprite = ANIM_PUNCH;
        }
    } else {
        enem->curr_sprite = 0;
    }
}

inline void enemy_decision(enemyData *enem, spritePos *playr) {
    int distance;
    int x_distance;
    int y_distance;
    // TODO return;
    // do not take decisions: you are hitted
    if (enem->is_hit > 0) {
        return;
    }

    // no decisions in floor, sir
    if (enem->is_floor != FALSE) {
        return;
    }

    x_distance = point_distance(playr->x, enem->x);
    y_distance = point_distance(playr->y, enem->y);
    // check hits
    if (x_distance <= 24 && y_distance <= 2) {
        if (playr->moving == PUNCH_LEFT && enem->x <= playr->x) {
            enem->is_hit = HIT_DURATION;
            enem->received_hits++;
        }
        if (playr->moving == PUNCH_RIGHT && player.x <= enem->x) {
            enem->is_hit = HIT_DURATION;
            enem->received_hits++;
        }

        if (enem->received_hits == 6) {
            enem->is_floor = FLOOR_DURATION;
            enem->moving = MOVING_RIGHT;
            enem->floor_times++;
            enem->received_hits = 0;
        }
    }

    if (enem->is_punching > 0) {
        if (x_distance > 10) {
            enem->is_punching = FALSE;
        }
        return;
    }


    int random_choice = rand() % 10;
    char enem_has_moved = FALSE;

    // check movements
    if ((enem->variant == 1 && (enem->targetX < playr->x || enem->targetX > playr->x + 25)) || (enem->variant == 2 && (enem->targetX > playr->x || enem->targetX < playr->x - 25))) {    
        if ((counter % 30) == 0) {
            if (enem->variant == 1) {
                enem->targetX = playr->x + 25;
            } else {
                if (playr->x > 25) {
                    enem->targetX = playr->x - 25;
                } else {
                    enem->targetX = playr->x;
                }
            }   
        }
    } else if ((enem->variant == 1 && (enem->x < playr->x || enem->x > playr->x + 25)) || (enem->variant == 2 && (enem->x > playr->x || enem->x < playr->x - 25))) {
        if (enem->targetX) {
            if (enem->x > enem->targetX && enem->x > 0) {
                enem->x--;
                enem_has_moved = TRUE;
                enem->moving = MOVING_LEFT;
            } else if (enem->x < enem->targetX && enem->x < 320) {                
                enem->x++;
                enem_has_moved = TRUE;
                enem->moving = MOVING_RIGHT;
            } else {
                enem->moving = STOP_RIGHT;
            }
        } 
    } else {
        if (point_distance(playr->y, enem->y) >= 2 && (counter % 2) == 0) {
            if (enem->y > playr->y) {
                enem->y_moving = MOVING_UP;
                enem->y--;
            } else {
                enem->y_moving = MOVING_DOWN;
                enem->y++;
            }
            enem_has_moved = TRUE;
        } else {
            enem->y_moving = STOPPOS;

            if (enem->moving == MOVING_LEFT || enem->moving == PUNCH_LEFT) {
                enem->moving = STOP_LEFT;
                enem->targetX = FALSE;
            } else if (enem->moving == MOVING_RIGHT ||
                       enem->moving == PUNCH_RIGHT) {
                enem->moving = STOP_RIGHT;
                enem->targetX = FALSE;
            }
            if (counter % 20 == 0 && playr->is_floor == FALSE) {
                if (enem->moving == STOP_LEFT || enem->moving == STOP_RIGHT) {
                    // TODO think on punch
                    if (enem->x > playr->x) {
                        enem->moving = PUNCH_LEFT;
                    } else {
                        enem->moving = PUNCH_RIGHT;
                    }
                    enem->is_punching = HIT_DURATION;
                }

                if (enem->moving == PUNCH_LEFT && player.x <= enem->x) {
                    playr->is_hit = HIT_DURATION;
                    playr->received_hits++;
                }
                if (enem->moving == PUNCH_RIGHT && player.x >= enem->x) {
                    playr->is_hit = HIT_DURATION;
                    playr->received_hits++;
                }

                if (player.received_hits == 5) {
                    playr->is_floor = FLOOR_DURATION;
                    playr->floor_times++;
                    playr->received_hits = 0;
                }
            }
        }
    }
    if (!enem_has_moved && (enem->moving == MOVING_LEFT || enem->moving == MOVING_RIGHT)) {
        enem->moving = STOP_LEFT;
    }
}

inline void draw_enemy(enemyData *enem) {
    if (enem->is_floor != FALSE) {
        if (enem->moving & 1) {
            rotate_sprite(screen, enem->sprite[enem->curr_sprite], enem->x,
                            enem->y + 10, itofix(1 * 64));
        } else {
            rotate_sprite_v_flip(screen, enem->sprite[enem->curr_sprite], enem->x,
                            enem->y + 10, itofix(1 * 64));
        }

    } else {
        // redraw pair or impair?
        if (enem->moving & 1) {
            draw_sprite_h_flip(screen, enem->sprite[enem->curr_sprite], enem->x,
                            enem->y);
        } else {
            draw_sprite(screen, enem->sprite[enem->curr_sprite], enem->x, enem->y);
        }
    }
}

void all_enemy_animations() {
    for (int i = 0; i < level_enemies; i++) {
        enemy_animation(&enemies[i]);
    }
}

void all_enemy_decisions(spritePos *playr) {
    for (int i = 0; i < level_enemies; i++) {
        enemy_decision(&enemies[i], playr);
    }
}

void all_draw_enemies() {
    for (int i = 0; i < level_enemies; i++) {
        draw_enemy(&enemies[i]);
    }
}

void redraw_bg_enemy_positions() {
    for (int i = 0; i < level_enemies; i++) {
        blit(bg, screen, enemies[i].x, 120, enemies[i].x, 120, 40, 80);
    }
}

char player_over_all_enemies(int player_y) {
    char player_under_enemies = FALSE;
    for (int i = 0; i < level_enemies; i++) {
        if (player_y < enemies[i].y) {
            player_under_enemies = TRUE;
        }
    }
    return player_under_enemies;
}

char enemy_on_path(int new_player_x, int play_y) {
    for (int i = 0; i < level_enemies; i++) {
        if (enemies[i].is_floor != FALSE) {
            continue;
        }
        int x_distance = point_distance(new_player_x, enemies[i].x);        
        int y_distance = point_distance(play_y, enemies[i].y);

        if (y_distance < 2 && x_distance < 8) {
            return TRUE;
        }
    }
    return FALSE;
}