#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "player.h"
#include "utils.h"
#include "map.h"

#define HOURS_IN_DAY       24

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
    printf("[11] Отправиться в приключение\n");
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

        case 11:
            adventure_run(&gs);
            break;

        default:
            printf("Неверный пункт меню.\n");
            break;
        }
    }

    if (gs.map) {
        map_free(gs.map);
        gs.map = NULL;
    }
    if (gs.player) {
        player_free(gs.player);
        gs.player = NULL;
    }

    return 0;
}