#ifndef UTILS_H
#define UTILS_H

#include "player.h"

#define MAX_LINE_LEN  256
#define MAX_ITEMS     10
#define HOURS_IN_DAY  24

/* Состояние игры */
typedef struct {
    Player* player;
    int     current_day;
    int     current_hour;
    char    item_names[MAX_ITEMS][MAX_NAME_LEN];
} GameState;

int  fool_check(int scanf_result);

int  items_catalog(char names[MAX_ITEMS][MAX_NAME_LEN]);

int  find_item_id_by_name(char names[][MAX_NAME_LEN], const char* word);

int  find_inventory_slots(const ItemType inv[], int size, int id, int slots[], int max_slots);

// Дневник
void write_diary_entry(FILE* out_screen, FILE* out_file, const char* player_name, int day, 
                       int hour, const ItemType inv[], int size, char item_names[MAX_ITEMS][MAX_NAME_LEN]);

void save_diary_action(const Player* player, int day, int hour,
                       char item_names[MAX_ITEMS][MAX_NAME_LEN]);

// Квесты
int  process_quests_to_streams(FILE* out_screen, FILE* out_file, const char* filename, int current_day,
                               const ItemType inv[], int inv_size, char item_names[MAX_ITEMS][MAX_NAME_LEN]);

void process_quests_action(int day, const ItemType inv[], char item_names[MAX_ITEMS][MAX_NAME_LEN]);

void print_time(int day, int hour);
int  advance_time(int* day, int* hour, int hours);
void sleep_until_morning(int* day, int* hour, Player* player);

#endif // UTILS_H