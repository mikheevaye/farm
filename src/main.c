#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN       256
#define MAX_NAME_LEN       32
#define MAX_PLAYER_NAME    32
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

void get_player_name(char player_name[], int max_len) { // массив символов, и max len (берется в main во время вызова)
    printf("Вы вошли в игру 'Веселый фермер'! Введите ваше имя (до 32 символов): ");
    fgets(player_name, max_len, stdin); //(запишет ввод в player name, смотрит максимум сколько можно, stdin это для компилятора "жди ввод с клавиатуры")
    player_name[strcspn(player_name, "\r\n")] = '\0'; // strcspn бегает по строке и ищет энтер, находит - записывает перенос строки вместо него

    if (player_name[0] == '\0') {
        strcpy(player_name, "Фермер");  
    } // '\0' - enter (ничего), strcpy копирует в массив деф.имя
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

const char* DEFAULT_ITEM_NAMES[MAX_ITEMS] = { // создает список неизменяемых текстовых названий для предметов, где каждому предмету соответствует свой порядковый номер (индекс).
    "Пусто", // [MAX_ITEMS] размер списка
    "Дерево",
    "Камень",
    "Семена",
    "Железо",
    "Вода",
    "Факел",
    "Золото",
    "Верёвка",
    "Коса"
};

int items_catalog(char names[MAX_ITEMS][MAX_NAME_LEN]) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        strncpy(names[i], DEFAULT_ITEM_NAMES[i], MAX_NAME_LEN - 1); // Сначала заполняем массив значениями по умолчанию
        names[i][MAX_NAME_LEN - 1] = '\0';
    }

    FILE* file = fopen("items.txt", "r"); // Пытаемся открыть файл
    if (file == NULL) {
        printf("Предупреждение: Файл %s не найден! Загружен каталог по умолчанию.\n", "items.txt");
        return 0;   // файл не найден — массив уже заполнен дефолтами
    }

    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), file) != NULL) { // Чтение строк в формате "ID Название". каждая строчка из файла по очереди помещается в переменную line
        int id;
        char name[MAX_NAME_LEN];

        char format[20];
        sprintf(format, "%%d %%%d[^\n]", MAX_NAME_LEN - 1); // Получится "%d %31[^\n]"

        if (sscanf(line, format, &id, name) == 2) { // sscanf берёт прочитанную строчку line и распиливает её по шаблону format (выше).
            // число кладётся в переменную id, а название предмета (текст) — в переменную name
            // если =2, то значит достали ровно 2 элемента из строки
            if (id >= 0 && id < MAX_ITEMS) {
                trim_whitespace(name); // очищает от пробелов

                if (name[0] != '\0') {
                    strncpy(names[id], name, MAX_NAME_LEN - 1);
                    names[id][MAX_NAME_LEN - 1] = '\0'; // название -> в итоговый каталог под тем номером id, под которым оно шло в файле. Старое дефолтное имя заменяем новым из файла
                } 
            }
        }
    }

    fclose(file);
    return 1;
}

void print_inventory(const ItemType inv[], int size) {
    for (int i = 0; i < size; i++) {
        printf("Слот %d: [%d] (%s)\n", i, (int)inv[i], get_item_name(inv[i])); 
    } 
    // inv[] адрес начала массива, size задается в main (case 3), inv[i] передается в get_item_name
}

int main(void) {
    SetConsoleOutputCP(CP_UTF8); // чтоб умная консоль выводила человеческий русский

    char item_names[MAX_ITEMS][MAX_NAME_LEN];
    char player_name[MAX_PLAYER_NAME];
    get_player_name(player_name, MAX_PLAYER_NAME);

    int current_day = 1;
    int current_hour = 8;
    ItemType inventory[INVENTORY_SIZE] = {
        ITEM_EMPTY, ITEM_WOOD, ITEM_STONE, ITEM_SEEDS, ITEM_IRON,
        ITEM_WATER, ITEM_TORCH, ITEM_GOLD, ITEM_ROPE, ITEM_SCYTHE
    };
    int choice = -1;

    while (choice != 0) {
        printf("Привет, %s! Добро пожаловать на ферму!\n", player_name);
        printf("\n============= Меню (%s) =============\n");
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