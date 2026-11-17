#include "player.h"
#include "utils.h"

Player* player_create(void) {
    Player* p = (Player*)malloc(sizeof(Player));
    if (p == NULL) {
        printf("Ошибка выделения памяти под Player!\n");
        return NULL;
    }

    printf("Введите имя персонажа (до %d символов): ", MAX_PLAYER_NAME - 1);
    if (fgets(p->name, MAX_PLAYER_NAME, stdin) == NULL) {
        strcpy(p->name, "Фермер");
    }
    p->name[strcspn(p->name, "\r\n")] = '\0';
    if (p->name[0] == '\0') {
        strcpy(p->name, "Фермер");
    }

    p->max_hp = 100;
    p->hp = 100;
    p->max_stamina = 50;
    p->stamina = 50;
    p->gold = 0;
    p->x = 0;
    p->y = 0;

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        p->inventory[i] = ITEM_EMPTY;
    }
    p->inventory[0] = ITEM_WOOD;
    p->inventory[1] = ITEM_STONE;
    p->inventory[2] = ITEM_SEEDS;

    p->active_relic.effect = RELIC_NONE;
    strcpy(p->active_relic.name, "Нет");

    printf("Персонаж «%s» создан! HP=%d, Стамина=%d\n",
        p->name, p->hp, p->stamina);
    return p;
}

void player_free(Player* p) {
    if (p == NULL) return;
    printf("Память структуры по адресу %p освобождена\n", (void*)p);
    free(p);
}

void player_print_card(const Player* p, int day, int hour,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    if (p == NULL) {
        printf("Персонаж ещё не создан!\n");
        return;
    }

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    printf("\n========== Карточка персонажа ==========\n");
    printf("Имя:           %s\n", p->name);
    printf("Здоровье:      %d / %d\n", p->hp, p->max_hp);
    printf("Выносливость:  %d / %d\n", p->stamina, p->max_stamina);
    printf("Золото:        %d\n", p->gold);
    printf("Координаты:    (%d, %d)\n", p->x, p->y);
    printf("Игровое время: День %d, %02d:00\n", day, hour);
    printf("Системное:     %02d.%02d.%04d %02d:%02d\n",
        t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
        t->tm_hour, t->tm_min);

    player_print_relic(p);

    printf("\n--- Инвентарь ---\n");
    player_print_inventory(p, item_names);
    printf("========================================\n");
}

void player_print_inventory(const Player* p,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    if (p == NULL) return;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        int id = (int)p->inventory[i];
        const char* name = (id >= 0 && id < MAX_ITEMS)
            ? item_names[id] : "Неизвестный";
        printf("  Слот %d: [%d] - %s\n", i, id, name);
    }
}

void player_put_item(Player* p, int slot, int id,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    if (p == NULL) return;
    if (slot < 0 || slot >= INVENTORY_SIZE) {
        printf("Индекс слота должен быть от 0 до %d.\n", INVENTORY_SIZE - 1);
        return;
    }
    if (id < 0 || id >= MAX_ITEMS) {
        printf("ID предмета должен быть от 0 до %d.\n", MAX_ITEMS - 1);
        return;
    }
    p->inventory[slot] = (ItemType)id;
    printf("Предмет «%s» помещён в слот %d.\n", item_names[id], slot);
}

void player_drop_item(Player* p, int slot) {
    if (p == NULL) return;
    if (slot < 0 || slot >= INVENTORY_SIZE) {
        printf("Индекс слота должен быть от 0 до %d.\n", INVENTORY_SIZE - 1);
        return;
    }
    p->inventory[slot] = ITEM_EMPTY;
    printf("Слот %d очищен.\n", slot);
}

void player_clear_item_id(Player* p, int id,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    if (p == NULL) return;
    if (id < 1 || id >= MAX_ITEMS) {
        printf("ID должен быть от 1 до %d.\n", MAX_ITEMS - 1);
        return;
    }
    int count = 0;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if ((int)p->inventory[i] == id) {
            p->inventory[i] = ITEM_EMPTY;
            count++;
        }
    }
    if (count == 0) {
        printf("Предметов с ID %d в инвентаре не найдено.\n", id);
    }
    else {
        printf("О


            чищено слотов : % d(предмет ID % d->« % s»).\n",
            count, id, item_names[id]);
    }
}

void player_find_item_by_name(Player* p, const char* word,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    if (p == NULL) return;

    int id = find_item_id_by_name(item_names, word);
    if (id < 0) {
        printf("Предмет «%s» не найден в каталоге.\n", word);
        return;
    }

    int slots[INVENTORY_SIZE];
    int count = find_inventory_slots(p->inventory, INVENTORY_SIZE,
        id, slots, INVENTORY_SIZE);
    if (count == 0) {
        printf("Предмет «%s» (ID %d) отсутствует в рюкзаке.\n",
            item_names[id], id);
    }
    else {
        printf("Предмет «%s» (ID %d) найден в слотах: ", item_names[id], id);
        for (int i = 0; i < count; i++) {
            printf("%d%s", slots[i], (i < count - 1) ? ", " : "\n");
        }
    }
}

void player_set_relic(Player* p, Relic relic) {
    if (p == NULL) return;
    p->active_relic = relic;
    printf("Вы надели «%s»!\n", relic.name);
}

void player_clear_relic(Player* p) {
    if (p == NULL) return;
    p->active_relic.effect = RELIC_NONE;
    strcpy(p->active_relic.name, "Нет");
}

void player_print_relic(const Player* p) {
    if (p == NULL) return;
    if (p->active_relic.effect == RELIC_NONE) {
        printf("Талисман:      нет\n");
    }
    else {
        printf("Талисман:      %s (%s)\n",
            p->active_relic.name,
            relic_effect_description(p->active_relic.effect));
    }
}