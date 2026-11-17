#ifndef MAP_H
#define MAP_H

#include "player.h"

// Символы для карты
#define SYM_WALL           '#'
#define SYM_FLOOR          '.'
#define SYM_PLAYER         '@'
#define SYM_LADDER_DOWN    'D'
#define SYM_LADDER_UP      'U'
#define SYM_SWITCH         'P'
#define SYM_TREE           'T'
#define SYM_ROCK           'R'

// Структура карты
typedef struct {
    char** cells;
    int         width;
    int         height;
    int         floor;
    int         is_vertical;
    int         wall_active;
    char        under;
} Map;

// Состояние игры 
typedef struct {
    Player* player;
    Map* map;
    int         current_day;
    int         current_hour;
    char        item_names[MAX_ITEMS][MAX_NAME_LEN];
} GameState;

// Прототипы 
Map* map_create(int height, int width);
void  map_free(Map* m);
void  map_generate_floor(Map* m, int* player_x, int* player_y);
void  map_print(const Map* m, const Player* p);
void  adventure_run(GameState* gs);

#endif /* MAP_H */