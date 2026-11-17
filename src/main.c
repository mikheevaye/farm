#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <windows.h>

#include "player.h"
#include "utils.h"

#define HOURS_IN_DAY       24

// Символы для карты
#define SYM_WALL           '#'
#define SYM_FLOOR          '.'
#define SYM_PLAYER         '@'
#define SYM_LADDER_DOWN    'D'
#define SYM_LADDER_UP      'U'
#define SYM_SWITCH         'P'
#define SYM_TREE           'T'
#define SYM_ROCK           'R'

int fool_check(int scanf_result) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    if (scanf_result != 1) {
        printf("Ошибка ввода. Введите целое число.\n");
        return 0;
    }
    return 1;
}

void get_player_name(char player_name[], int max_len) { 
    printf("Вы вошли в игру 'Веселый фермер'! Введите ваше имя (до 32 символов): ");
    fgets(player_name, max_len, stdin);
    player_name[strcspn(player_name, "\r\n")] = '\0'; 

    if (player_name[0] == '\0') {
        strcpy(player_name, "Name");  // если игрок нажал enter, то деф.имя
    }
}

const char* DEFAULT_ITEM_NAMES[MAX_ITEMS] = {
    "Пусто", 
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

void print_inventory(const ItemType inv[], int size, char item_names[][MAX_NAME_LEN]) {
        for (int i = 0; i < size; i++) {
            int id = (int)inv[i];
            const char* name = "Неизвестный предмет";

            if (id >= 0 && id < MAX_ITEMS) {
                name = item_names[id]; // подстановка названия по ID
            }

            printf("Слот %d: [%d] - %s\n", i, id, name);
        }
}

// Выделение памяти
char** create_map(int height, int width) { 
    char** map = (char**)malloc(height * sizeof(char*)); // (sizeof(char*) размер одного указателя
    for (int i = 0; i < height; i++) {
        map[i] = (char*)malloc(width * sizeof(char));
    }
    return map;
}

// Освобождение памяти
void free_map(char** map, int height) {
    for (int i = 0; i < height; i++) {
        free(map[i]);
    }
    free(map);
}

// Отрисовка карты
void print_map(char** map, int height, int width, int floor,
    int is_vertical, int wall_active)
{
    printf("\n=== Башня: этаж %d ===\n", floor);
    int mid = is_vertical ? width / 2 : height / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            char c;
            int hidden = wall_active && ((is_vertical && x > mid) || (!is_vertical && y > mid));

            if (hidden) {
                int mid_x = is_vertical ? (width - 1 - x) : x;
                int mid_y = is_vertical ? y : (height - 1 - y);
                c = map[mid_y][mid_x];
            }
            else {
                c = map[y][x];
            }
            printf("%c ", c);
        }

        printf("\n");
    }
    printf("W/A/S/D - походить на 1 клетку, \n Q - сбежать домой (возможно только на %c)\n",
        SYM_LADDER_DOWN);
}

// Генерация этажа
void make_floor(char** map, int height, int width, int is_vertical,
    int* player_start_x, int* player_start_y, char* under) {

    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            map[y][x] = SYM_FLOOR;

    // Внешние стены
    for (int y = 0; y < height; y++) {
        map[y][0] = SYM_WALL;
        map[y][width - 1] = SYM_WALL;
    }
    for (int x = 0; x < width; x++) {
        map[0][x] = SYM_WALL;
        map[height - 1][x] = SYM_WALL;
    }

    if (is_vertical) {
        int mid = width / 2;
        for (int y = 2; y < height - 2; y++)
            for (int x = 2; x < width; x++) {
                int r = rand() % 10;
                if (r == 0)      map[y][x] = SYM_TREE;
                else if (r == 1) map[y][x] = SYM_ROCK;
            }

        // Стена посередине
        for (int y = 0; y < height; y++)
            map[y][mid] = SYM_WALL;
    }
    else {
        int mid = height / 2;
        for (int y = 2; y < height; y++)
            for (int x = 2; x < width - 2; x++) {
                int r = rand() % 10;
                if (r == 0)      map[y][x] = SYM_TREE;
                else if (r == 1) map[y][x] = SYM_ROCK;
            }

        // Стена посередине
        for (int x = 0; x < width; x++)
            map[mid][x] = SYM_WALL;

    }

    int x, y;

    // Игрок + D под ним
    do {
        x = 1 + rand() % (width - 2);
        y = 1 + rand() % (height - 2);
    } while (map[y][x] != SYM_FLOOR);
    *player_start_x = x;
    *player_start_y = y;
    *under = SYM_LADDER_DOWN;
    map[y][x] = SYM_PLAYER;

    // Переключатель
    do {
        x = 1 + rand() % (width - 2);
        y = 1 + rand() % (height - 2);
    } while (map[y][x] != SYM_FLOOR);
    map[y][x] = SYM_SWITCH;

    // Лестница вверх
    do {
        x = 1 + rand() % (width - 2);
        y = 1 + rand() % (height - 2);
    } while (map[y][x] != SYM_FLOOR);
    map[y][x] = SYM_LADDER_UP;

}
// [10] - Отправиться в приключение
void adventure_action(void) {
    int floor = 1;
    int height = 7 + (rand() % 6) * 2;   // всегда нечётное
    int width = 7 + (rand() % 6) * 2;   // всегда нечётное
    int is_vertical = rand() % 2;
    int wall_active = 1;                 // маскировка вкл

    char** map = create_map(height, width);
    int player_start_x, player_start_y;
    char under;

    make_floor(map, height, width, is_vertical,
        &player_start_x, &player_start_y, &under);

    while (1) {
        print_map(map, height, width, floor, is_vertical, wall_active);

        char move;
        printf("Ваш ход: ");
        scanf(" %c", &move);

        if (move == 'q' || move == 'Q') {
            if (under == SYM_LADDER_DOWN) {
                printf("\nВы успешно сбежали домой!\n");
                break;
            }
            else {
                printf("\nВыход работает только если вы стоите на лестнице вниз (%c)!\n",
                    SYM_LADDER_DOWN);
                continue;
            }
        }

        int new_x = player_start_x;
        int new_y = player_start_y;

        if (move == 'w' || move == 'W') new_y++;
        else if (move == 's' || move == 'S') new_y--;
        else if (move == 'a' || move == 'A') new_x--;
        else if (move == 'd' || move == 'D') new_x++;
        else {
            printf("\nИспользуйте клавиши W, A, S, D или Q!\n");
            continue;
        }

        // Проверка границ
        if (new_x < 0 || new_x >= width || new_y < 0 || new_y >= height) {
            printf("\nВы не можете пройти дальше, это край карты... Попробуйте другое направление.\n");
                continue;
        }

        // Проверка проходимости
        if (map[new_y][new_x] == SYM_WALL ||
            map[new_y][new_x] == SYM_TREE ||
            map[new_y][new_x] == SYM_ROCK) {
            printf("\nВы не можете пройти сквозь препятствие...\n");
            continue;
        }

        // Ход
        map[player_start_y][player_start_x] = under;
        player_start_x = new_x;
        player_start_y = new_y;
        under = map[player_start_y][player_start_x];
        map[player_start_y][player_start_x] = SYM_PLAYER;

        if (under == SYM_SWITCH) {
            printf("\n*ЩЕЛК* Стена посередине исчезла!\n");
            wall_active = 0; // маскировка выключена

            if (is_vertical) {
                int mid = width / 2;
                for (int y = 1; y < height - 1; y++) map[y][mid] = SYM_FLOOR;
            }
            else {
                int mid = height / 2;
                for (int x = 1; x < width - 1; x++) map[mid][x] = SYM_FLOOR;
            }
            under = SYM_FLOOR;
        }

        if (under == SYM_LADDER_UP) {
            printf("\nВы поднялись на следующий этаж!\n");
            floor++;

            free_map(map, height);

            height = 7 + (rand() % 6) * 2;
            width = 7 + (rand() % 6) * 2;
            is_vertical = rand() % 2;
            wall_active = 1; // ← новый этаж — маскировка включ

            map = create_map(height, width);
            make_floor(map, height, width, is_vertical,
                &player_start_x, &player_start_y, &under);
        }
    }

    free_map(map, height);
}

int main(void) {
    SetConsoleOutputCP(CP_UTF8); // чтоб умная консоль выводила человеческий русский
    SetConsoleCP(CP_UTF8);

    char item_names[MAX_ITEMS][MAX_NAME_LEN];
    items_catalog(item_names);
    char player_name[MAX_PLAYER_NAME];
    get_player_name(player_name, MAX_PLAYER_NAME);
    srand((unsigned int)time(NULL));

    int current_day = 1;
    int current_hour = 8;
    ItemType inventory[INVENTORY_SIZE] = {
        ITEM_EMPTY, ITEM_WOOD, ITEM_STONE, ITEM_SEEDS, ITEM_IRON,
        ITEM_WATER, ITEM_TORCH, ITEM_GOLD, ITEM_ROPE, ITEM_SCYTHE
    };
    int choice = -1;

    printf("Привет, %s! Добро пожаловать на ферму!\n", player_name);

    while (choice != 0) {
        printf("\n============= Меню (%s) =============\n", player_name);
        printf("[0] Выход\n");
        printf("[1] Посмотреть на часы\n");
        printf("[2] Промотать время (Поработать)\n");
        printf("[3] Посмотреть инвентарь\n");
        printf("[4] Положить предмет в слот\n");
        printf("[5] Выбросить предмет\n");
        printf("[6] Очистить инвентарь от мусора\n");
        printf("[7] Поиск предмета в рюкзаке по названию\n");
        printf("[8] Записать состояние в дневник фермера (diary.txt)\n");
        printf("[9] Расшифровать старые записи\n");
        printf("[10] Отправиться в приключение\n");
        printf("=======================================\n");
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
            print_inventory(inventory, INVENTORY_SIZE, item_names);
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
            printf("Предмет '%s' помещён в слот %d.\n", item_names[id], slot);
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
                    count, id, item_names[id]);
            }
            break;
        }

        case 7: {
            char word[MAX_NAME_LEN];

            printf("Введите название предмета для поиска: ");

            if (fgets(word, sizeof(word), stdin) == NULL) {
                break;
            }

            word[strcspn(word, "\r\n")] = '\0';

            if (word[0] == '\0') {
                printf("Название предмета не может быть пустым.\n");
                break;
            }

            int id = find_item_id_by_name(item_names, word);

            if (id < 0) {
                printf("Предмет с названием '%s' не найден в каталоге.\n", word);
                break;
            }

            int slots[INVENTORY_SIZE];
            int count = find_inventory_slots(
                inventory, INVENTORY_SIZE, id, slots, INVENTORY_SIZE
            );

            if (count == 0) {
                printf("Предмет '%s' (ID %d) отсутствует в рюкзаке.\n",
                    item_names[id], id);
            }
            else {
                printf("Предмет '%s' (ID %d) найден в слотах: ",
                    item_names[id], id);

                for (int i = 0; i < count; i++) {
                    printf("%d", slots[i]);

                    if (i != count - 1) {
                        printf(", ");
                    }
                }

                printf("\n");
            }

            break;
        }

        case 8:
            save_diary_action(player_name, current_day, current_hour, inventory, item_names);
            break;

        case 9:
            process_quests_action(current_day, inventory, item_names);
            break;
        
        case 10:
            adventure_action();
            break;

        default:
            printf("Неверный пункт меню.\n");
            break;
        }
    }

    return 0;
}