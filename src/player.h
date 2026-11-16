#ifndef PLAYER_H
#define PLAYER_H

#define MAX_LINE_LEN        256
#define MAX_NAME_LEN        32
#define MAX_PLAYER_NAME     32
#define INVENTORY_SIZE      10
#define MAX_ITEMS           10
#define BASE_GOLD_MULTIPLIER 15
#define NIGHT_HOUR          2
#define MORNING_HOUR        8
#define RELIC_SELL_PRICE    25
#define RELIC_FIND_CHANCE   20

/* ===== Перечисления ===== */
typedef enum {
    ITEM_EMPTY = 0,
    ITEM_WOOD,
    ITEM_STONE,
    ITEM_SEEDS,
    ITEM_IRON,
    ITEM_WATER,
    ITEM_TORCH,
    ITEM_GOLD,
    ITEM_ROPE,
    ITEM_SCYTHE
} ItemType;

typedef enum {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_QUIT
} Direction;

typedef enum {
    RELIC_NONE = 0,
    RELIC_GREED,      // Кольцо жадности
    RELIC_SPEED,      // Сапоги-скороходы
    RELIC_TROLL,      // Камень тролля
    RELIC_COUNT
} RelicEffect;

// Структуры
typedef struct {
    RelicEffect effect;
    char        name[MAX_NAME_LEN];
} Relic;

typedef struct {
    char        name[MAX_PLAYER_NAME];
    int         hp;
    int         max_hp;
    int         stamina;
    int         max_stamina;
    int         gold;
    int         x, y;
    ItemType    inventory[INVENTORY_SIZE];
    Relic       active_relic;
} Player;

// Прототипы
Player* player_create(void);
void    player_free(Player* p);

void    player_print_card(const Player* p, int day, int hour,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void    player_print_inventory(const Player* p,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]);

void    player_put_item(Player* p, int slot, int id,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void    player_drop_item(Player* p, int slot);
void    player_clear_item_id(Player* p, int id,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void    player_find_item_by_name(Player* p, const char* word,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]);

void    player_set_relic(Player* p, Relic relic);
void    player_clear_relic(Player* p);
void    player_print_relic(const Player* p);

#endif // PLAYER_H