#include <stdio.h>
#include <locale.h>

#define HOURS_IN_DAY       24
#define INVENTORY_SIZE     10
#define MAX_ITEMS          10

int fool_check(int scanf_result) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    if (scanf_result != 1) {
        printf("Ошибка ввода. Введите целое число.\n");
        return 0;
    }
    return 1;
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

void print_inventory(const ItemType inv[], int size) {
    for (int i = 0; i < size; i++) {
        printf("Слот %d: [%d] (%s)\n", i, (int)inv[i], get_item_name(inv[i])); 
    } 
    // inv[] адрес начала массива, size задается в main (case 3), inv[i] передается в get_item_name
}

int main(void) {
    setlocale(LC_ALL, ""); // чтоб умная консоль выводила человеческий русский
    int current_day = 1;
    int current_hour = 8;
    ItemType inventory[INVENTORY_SIZE] = {
        ITEM_EMPTY, ITEM_WOOD, ITEM_STONE, ITEM_SEEDS, ITEM_IRON,
        ITEM_WATER, ITEM_TORCH, ITEM_GOLD, ITEM_ROPE, ITEM_SCYTHE
    };
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

        if (!fool_check(scanf("%d", &choice))) {
            continue;
        }

        switch (choice) {
        case 0:
            printf("Вы вышли из игры. Увидимся в следующий раз!\n");
            break;

        case 1:
            printf("Текущее время: День %d, %02d:00\n",
                current_day, current_hour);
            break;

        case 2: {
            int hours;
            printf("Сколько часов хотите поработать? ");
            if (!fool_check(scanf("%d", &hours))) break;
            if (hours < 0) {
                printf("Вы не можете порботать отрицательное количество часов.\n");
                break;
            }
            current_hour += hours;
            current_day += current_hour / HOURS_IN_DAY;
            current_hour = current_hour % HOURS_IN_DAY;

            printf("Прошло %d ч. Текущее время: День %d, %02d:00\n", hours, current_day, current_hour);
            break;
        }

        case 3:
            print_inventory(inventory, INVENTORY_SIZE);
            break;

        case 4: {
            int slot, id;
            printf("Введите индекс слота (0-9): ");
            if (!fool_check(scanf("%d", &slot))) break;
            if (slot < 0 || slot >= INVENTORY_SIZE) {
                printf("Индекс должен быть от 0 до 9.\n");
                break;
            }

            printf("Введите ID предмета (0-9): ");
            if (!fool_check(scanf("%d", &id))) break;
            if (id < 0 || id >= MAX_ITEMS) {
                printf("ID должен быть от 0 до 9.\n");
                break;
            }

            inventory[slot] = (ItemType)id;
            printf("Предмет '%s' помещён в слот %d.\n", get_item_name((ItemType)id), slot);
            break;
        }

        case 5: {
            int slot;
            printf("Введите индекс слота, из которого хотите удалить предмет (0-9): ");
            if (!fool_check(scanf("%d", &slot))) break;
            if (slot < 0 || slot >= INVENTORY_SIZE) {
                printf("Индекс должен быть от 0 до 9.\n");
                break;
            }
            inventory[slot] = ITEM_EMPTY;
            printf("Слот %d очищен.\n", slot);
            break;
        }

        case 6: {
            int id, count = 0;
            printf("Введите ID предмета (1-9) для очистки: ");
            if (!fool_check(scanf("%d", &id))) break;
            if (id < 1 || id >= MAX_ITEMS) {
                printf("ID должен быть от 1 до 9.\n");
                break;
            }

            for (int i = 0; i < INVENTORY_SIZE; i++) {
                if (inventory[i] != ITEM_EMPTY && (int)inventory[i] == id) {
                    inventory[i] = ITEM_EMPTY;
                    count++;
                }
            }

            if (count == 0) {
                printf("Предметов с ID %d в инвентаре не найдено, очистка не произведена.\n", id);
            }
            else {
                printf("Очищено слотов: %d (предмет ID %d -> '%s').\n",
                    count, id, get_item_name((ItemType)id));
            }
            break;
        }
    }
    }

    return 0;
}