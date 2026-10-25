#include <stdio.h>

#define INVENTORY_SIZE     10
#define HOURS_IN_DAY       24
#define INITIAL_DAY        1
#define INITIAL_HOUR       8

void not_fool(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

typedef enum {
    ITEM_EMPTY,
    ITEM_WOOD,
    ITEM_STONE,
    ITEM_SEEDS,
    ITEM_IRON,
    ITEM_WATER,
    ITEM_TORCH,
    ITEM_GOLD,
    ITEM_ROPE,
    ITEM_SCYTHE,
} ItemType;

const char* get_item_name(ItemType item) {
    switch (item) {
    case ITEM_EMPTY:  return "Пусто";
    case ITEM_WOOD:   return "Дерево";
    case ITEM_STONE:  return "Камень";
    case ITEM_SEEDS:  return "Семена";
    case ITEM_IRON:   return "Железо";
    case ITEM_WATER:  return "Вода";
    case ITEM_TORCH:  return "Факел";
    case ITEM_GOLD:   return "Золото";
    case ITEM_ROPE:   return "Верёвка";
    case ITEM_SCYTHE: return "Коса";
    default:          return "Неизвестный предмет";
    }
}

void inventory(const ItemType inv[], int size) {
    for (int i = 0; i < size; i++) {
        printf("Слот %d: [%d] (%s)\n", i, (int)inv[i], get_item_name(inv[i]));
    }
}

int main(void) {
    setlocale(0, ""); // чтоб умная консоль выводила человеческий русский
    int current_day = 1;
    int current_hour = 8;
    ItemType inventory[INVENTORY_SIZE] {
        ITEM_EMPTY, ITEM_WOOD, ITEM_STONE, ITEM_SEEDS, ITEM_IRON,
        ITEM_WATER, ITEM_TORCH, ITEM_GOLD, ITEM_ROPE, ITEM_SCYTHE
    }
    int choice = -1;
    
    while (choice != 0) {
        printf("\n============= Меню =============\n");
        printf("[0] Выход\n");
        printf("[1] Посмотреть на часы\n");
        printf("[2] Промотать время (Поработать)\n");
        printf("[3] Посмотреть инвентарь\n");
        printf("[4] Положить предмет в слот\n");
        printf("[5] Выбросить предмет\n");
        printf("[6] Очистить инвентарь от мусора\n");
        printf("==================================\n");
        printf("Что хотите сделать?: ");

        choice = read_int("Выберите пункт меню: ", 0, 6);

        while ((c = getchar()) != '\n' && c != EOF) {}

        switch (choice) {
            
        }

}