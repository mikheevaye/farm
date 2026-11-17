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

static void print_main_menu(const char* name) {
    printf("\n============= Меню (%s) =============\n",
        name ? name : "нет персонажа");
    printf("[0]  Выход\n");
    printf("[1]  Создать / пересоздать персонажа\n");
    printf("[2]  Карточка персонажа\n");
    printf("[3]  Управление временем\n");
    printf("[4]  Посмотреть инвентарь\n");
    printf("[5]  Положить предмет в слот\n");
    printf("[6]  Выбросить предмет\n");
    printf("[7]  Очистить инвентарь от мусора\n");
    printf("[8]  Поиск предмета в рюкзаке по названию\n");
    printf("[9]  Записать состояние в дневник (diary.txt)\n");
    printf("[10] Расшифровать старые записи (квесты)\n");
    printf("=======================================\n");
    printf("Что хотите сделать?: ");
}

static void time_submenu(GameState* gs) {
    int sub = -1;
    while (sub != 0) {
        printf("\n--- Управление временем ---\n");
        printf("[0] Назад в главное меню\n");
        printf("[1] Посмотреть на часы\n");
        printf("[2] Пропустить время (Ждать)\n");
        printf("[3] Отдых (Лечь спать)\n");
        printf("Выбор: ");

        if (!fool_check(scanf("%d", &sub))) continue;

        switch (sub) {
        case 0: break;
        case 1:
            print_time(gs->current_day, gs->current_hour);
            break;
        case 2: {
            int hours;
            printf("Сколько часов хотите промотать (от 1)? ");
            if (!fool_check(scanf("%d", &hours))) break;
            int advanced = advance_time(&gs->current_day, &gs->current_hour, hours);
            if (advanced > 0) {
                printf("Прошло %d ч. ", advanced);
                print_time(gs->current_day, gs->current_hour);
            }
            break;
        }
        case 3:
            sleep_until_morning(&gs->current_day, &gs->current_hour, gs->player);
            break;
        default:
            printf("Неверный пункт.\n");
            break;
        }
    }
}

static void create_or_recreate_player(GameState* gs) {
    if (gs->player != NULL) {
        printf("Персонаж «%s» уже существует. Удалить и создать нового? (1=да, 0=нет): ",
            gs->player->name);
        int conf = 0;
        if (!fool_check(scanf("%d", &conf)) || conf != 1) {
            printf("Отменено.\n");
            return;
        }
        player_free(gs->player);
        gs->player = NULL;
    }
    gs->player = player_create();
}

int main(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    srand((unsigned int)time(NULL));

    GameState gs;
    gs.player = NULL;
    gs.map = NULL;
    gs.current_day = 1;
    gs.current_hour = MORNING_HOUR;
    items_catalog(gs.item_names);

    printf("Добро пожаловать в «Весёлый фермер» (Лабораторная №5, вариант 5 — Реликвии)!\n");
    printf("Сначала создайте персонажа (пункт меню 1).\n");

    int choice = -1;
    while (choice != 0) {
        const char* pname = (gs.player) ? gs.player->name : NULL;
        print_main_menu(pname);

        if (!fool_check(scanf("%d", &choice))) continue;

        int needs_player = (choice >= 2 && choice <= 11 && choice != 3);
        if (needs_player && gs.player == NULL) {
            printf("Сначала создайте персонажа (пункт 1)!\n");
            continue;
        }

        switch (choice) {
        case 0:
            printf("Вы вышли из игры. Увидимся!\n");
            break;

        case 1:
            create_or_recreate_player(&gs);
            break;

        case 2:
            player_print_card(gs.player, gs.current_day, gs.current_hour,
                gs.item_names);
            break;

        case 3:
            time_submenu(&gs);
            break;

        case 4:
            player_print_inventory(gs.player, gs.item_names);
            break;

        case 5: {
            int slot, id;
            printf("Введите индекс слота (0-%d): "


                , INVENTORY_SIZE - 1);
            if (!fool_check(scanf("%d", &slot))) break;
            printf("Введите ID предмета (0-%d): ", MAX_ITEMS - 1);
            if (!fool_check(scanf("%d", &id))) break;
            player_put_item(gs.player, slot, id, gs.item_names);
            break;
        }

        case 6: {
            int slot;
            printf("Введите индекс слота (0-%d): ", INVENTORY_SIZE - 1);
            if (!fool_check(scanf("%d", &slot))) break;
            player_drop_item(gs.player, slot);
            break;
        }

        case 7: {
            int id;
            printf("Введите ID предмета (1-%d) для очистки: ", MAX_ITEMS - 1);
            if (!fool_check(scanf("%d", &id))) break;
            player_clear_item_id(gs.player, id, gs.item_names);
            break;
        }

        case 8: {
            char word[MAX_NAME_LEN];
            printf("Введите название предмета: ");
            int ch; while ((ch = getchar()) != '\n' && ch != EOF);
            if (fgets(word, sizeof(word), stdin) == NULL) break;
            word[strcspn(word, "\r\n")] = '\0';
            if (word[0] == '\0') {
                printf("Название не может быть пустым.\n");
                break;
            }
            player_find_item_by_name(gs.player, word, gs.item_names);
            break;
        }

        case 9:
            save_diary_action(gs.player, gs.current_day, gs.current_hour,
                gs.item_names);
            break;

        case 10:
            process_quests_action(gs.current_day, gs.player->inventory,
                gs.item_names);
            break;

        default:
            printf("Неверный пункт меню.\n");
            break;
        }
    }
}