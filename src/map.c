#include "map.h"
#include "utils.h"

Map* map_create(int height, int width) {
    Map* m = (Map*)malloc(sizeof(Map));
    if (m == NULL) {
        printf("Ошибка выделения памяти под Map!\n");
        return NULL;
    }

    m->height = height;
    m->width = width;
    m->floor = 1;
    m->is_vertical = rand() % 2;
    m->wall_active = 1;
    m->under = SYM_FLOOR;

    m->cells = (char**)malloc(height * sizeof(char*));
    if (m->cells == NULL) {
        free(m);
        return NULL;
    }
    for (int i = 0; i < height; i++) {
        m->cells[i] = (char*)malloc(width * sizeof(char));
        if (m->cells[i] == NULL) {
            for (int j = 0; j < i; j++) free(m->cells[j]);
            free(m->cells);
            free(m);
            return NULL;
        }
    }
    return m;
}

void map_free(Map* m) {
    if (m == NULL) return;
    if (m->cells) {
        for (int i = 0; i < m->height; i++) {
            free(m->cells[i]);
        }
        free(m->cells);
    }
    printf("Память структуры по адресу %p освобождена\n", (void*)m);
    free(m);
}

void map_generate_floor(Map* m, int* player_x, int* player_y) {
    for (int y = 0; y < m->height; y++)
        for (int x = 0; x < m->width; x++)
            m->cells[y][x] = SYM_FLOOR;

    for (int y = 0; y < m->height; y++) {
        m->cells[y][0] = SYM_WALL;
        m->cells[y][m->width - 1] = SYM_WALL;
    }
    for (int x = 0; x < m->width; x++) {
        m->cells[0][x] = SYM_WALL;
        m->cells[m->height - 1][x] = SYM_WALL;
    }

    if (m->is_vertical) {
        int mid = m->width / 2;
        for (int y = 2; y < m->height - 2; y++)
            for (int x = 2; x < m->width; x++) {
                int r = rand() % 10;
                if (r == 0)      m->cells[y][x] = SYM_TREE;
                else if (r == 1) m->cells[y][x] = SYM_ROCK;
            }
        for (int y = 0; y < m->height; y++)
            m->cells[y][mid] = SYM_WALL;
    }
    else {
        int mid = m->height / 2;
        for (int y = 2; y < m->height; y++)
            for (int x = 2; x < m->width - 2; x++) {
                int r = rand() % 10;
                if (r == 0)      m->cells[y][x] = SYM_TREE;
                else if (r == 1) m->cells[y][x] = SYM_ROCK;
            }
        for (int x = 0; x < m->width; x++)
            m->cells[mid][x] = SYM_WALL;
    }

    int x, y;
    do {
        x = 1 + rand() % (m->width - 2);
        y = 1 + rand() % (m->height - 2);
    } while (m->cells[y][x] != SYM_FLOOR);
    *player_x = x;
    *player_y = y;
    m->under = SYM_LADDER_DOWN;
    m->cells[y][x] = SYM_PLAYER;

    do {
        x = 1 + rand() % (m->width - 2);
        y = 1 + rand() % (m->height - 2);
    } while (m->cells[y][x] != SYM_FLOOR);
    m->cells[y][x] = SYM_SWITCH;

    do {
        x = 1 + rand() % (m->width - 2);
        y = 1 + rand() % (m->height - 2);
    } while (m->cells[y][x] != SYM_FLOOR);
    m->cells[y][x] = SYM_LADDER_UP;
}

void map_print(const Map* m, const Player* p) {
    if (m == NULL) return;

    printf("\n=== Башня: этаж %d ===\n", m->floor);
    int mid = m->is_vertical ? m->width / 2 : m->height / 2;

    for (int y = 0; y < m->height; y++) {
        for (int x = 0; x < m->width; x++) {
            char c;
            int hidden = m->wall_active &&
                ((m->is_vertical && x > mid) ||
                    (!m->is_vertical && y > mid));

            if (hidden) {
                int mid_x = m->is_vertical ? (m->width - 1 - x) : x;
                int mid_y = m->is_vertical ? y : (m->height - 1 - y);
                c = m->cells[mid_y][mid_x];
            }
            else {
                c = m->cells[y][x];
            }
            printf("%c ", c);
        }
        printf("\n");
    }

    if (p) {
        printf("HP: %d/%d | Выносливость: %d/%d | Золото: %d | Этаж: %d\n",
            p->hp, p->max_hp, p->stamina, p->max_stamina,
            p->gold, m->floor);
    }
    printf("W/A/S/D - ходить, Q - сбежать (только на %c)\n", SYM_LADDER_DOWN


    );
}

static int step_stamina_cost(const Player* p) {
    if (p && p->active_relic.effect == RELIC_GREED)
        return 2;
    return 1;
}

static int floor_stamina_cost(const Player* p) {
    if (p && p->active_relic.effect == RELIC_SPEED)
        return 1;
    return 5;
}

static int floor_gold_reward(const Player* p, int floor) {
    int base = BASE_GOLD_MULTIPLIER * floor;
    if (p && p->active_relic.effect == RELIC_GREED)
        return base * 2;
    return base;
}

static void handle_relic_find(Player* p) {
    if (p == NULL) return;
    if (rand() % 100 >= RELIC_FIND_CHANCE) return;

    Relic found = get_random_relic();
    printf("\n*** Вы нашли реликвию: «%s»! ***\n", found.name);
    printf("Эффект: %s\n", relic_effect_description(found.effect));
    printf("[1] Надеть (старый талисман пропадёт)\n");
    printf("[2] Продать на месте за %d золота\n", RELIC_SELL_PRICE);
    printf("Ваш выбор: ");

    int choice = 0;
    if (!fool_check(scanf("%d", &choice))) {
        printf("Реликвия потеряна в суматохе...\n");
        return;
    }

    if (choice == 1) {
        player_set_relic(p, found);
    }
    else if (choice == 2) {
        p->gold += RELIC_SELL_PRICE;
        printf("Вы продали «%s» за %d золота. Теперь у вас %d золота.\n",
            found.name, RELIC_SELL_PRICE, p->gold);
    }
    else {
        printf("Вы проигнорировали находку.\n");
    }
}

void adventure_run(GameState* gs) {
    if (gs == NULL || gs->player == NULL) {
        printf("Сначала создайте персонажа!\n");
        return;
    }
    Player* p = gs->player;

    if (p->stamina <= 0) {
        printf("Вы слишком устали (выносливость = 0). Отдохните!\n");
        return;
    }
    if (gs->current_hour == NIGHT_HOUR) {
        printf("Уже 02:00 ночи. Фермер должен спать!\n");
        return;
    }

    int height = 7 + (rand() % 6) * 2;
    int width = 7 + (rand() % 6) * 2;
    Map* m = map_create(height, width);
    if (m == NULL) return;
    gs->map = m;

    map_generate_floor(m, &p->x, &p->y);

    int emergency = 0;

    while (1) {
        printf("\n=== Башня: этаж %d ===\n", m->floor);
        int mid = m->is_vertical ? m->width / 2 : m->height / 2;

        for (int y = 0; y < m->height; y++) {
            for (int x = 0; x < m->width; x++) {
                char c;
                int hidden = m->wall_active &&
                    ((m->is_vertical && x > mid) ||
                        (!m->is_vertical && y > mid));
                if (hidden) {
                    int mid_x = m->is_vertical ? (m->width - 1 - x) : x;
                    int mid_y = m->is_vertical ? y : (m->height - 1 - y);
                    c = m->cells[mid_y][mid_x];
                }
                else {
                    c = m->cells[y][x];
                }
                printf("%c ", c);
            }
            printf("\n");
        }

        printf("HP: %d/%d | Выносливость: %d/%d | Золото: %d | Этаж: %d | Время: %02d:00\n",
            p->hp, p->max_hp, p->stamina, p->max_stamina,
            p->gold, m->floor, gs->current_hour);
        if (p->active_relic.effect != RELIC_NONE) {
            printf("Талисман: %s\n", p->active_relic.name);
        }
        printf("W/A/S/D - ходить, Q - сбежать (только на %c)\n", SYM_LADDER_DOWN);

        char move;
        printf("Ваш ход: ");
        if (scanf(" %c", &move) != 1) {
            int ch; while ((ch = getchar()) != '\n' && ch != EOF);
            continue;
        }

        if (move == 'q' || move == 'Q') {
            if (m->under == SYM_LADDER_DOWN) {
                printf("\nВы успешно сбежали домой! Всё заработанное золото при вас.\n");
                break;
            }
            else {
                printf("\nВыход работает только если вы стоите на лестнице вниз (%c)!\n",
                    SYM_LADDER_DOWN);
                continue;
            }
        }

        int new_x = p->x;
        int new_y = p->y;

        if (move == 'w' || move == 'W') new_y--;
        else if (move == 's'


            || move == 'S') new_y++;
        else if (move == 'a' || move == 'A') new_x--;
        else if (move == 'd' || move == 'D') new_x++;
        else {
            printf("Используйте W, A, S, D или Q!\n");
            continue;
        }

        if (new_x < 0 || new_x >= m->width || new_y < 0 || new_y >= m->height) {
            printf("Край карты. Попробуйте другое направление.\n");
            continue;
        }

        char cell = m->cells[new_y][new_x];
        if (cell == SYM_WALL || cell == SYM_TREE || cell == SYM_ROCK) {
            printf("Нельзя пройти сквозь препятствие.\n");
            continue;
        }

        m->cells[p->y][p->x] = m->under;
        p->x = new_x;
        p->y = new_y;
        m->under = m->cells[p->y][p->x];
        m->cells[p->y][p->x] = SYM_PLAYER;

        int cost = step_stamina_cost(p);
        p->stamina -= cost;
        if (p->stamina < 0) p->stamina = 0;

        if (p->active_relic.effect == RELIC_TROLL) {
            p->hp += 1;
            if (p->hp > p->max_hp) p->hp = p->max_hp;
        }

        if (m->under == SYM_SWITCH) {
            printf("\n*ЩЕЛК* Стена посередине исчезла!\n");
            m->wall_active = 0;
            if (m->is_vertical) {
                int mid = m->width / 2;
                for (int y = 1; y < m->height - 1; y++)
                    m->cells[y][mid] = SYM_FLOOR;
            }
            else {
                int mid = m->height / 2;
                for (int x = 1; x < m->width - 1; x++)
                    m->cells[mid][x] = SYM_FLOOR;
            }
            m->under = SYM_FLOOR;
        }

        if (m->under == SYM_LADDER_UP) {
            printf("\nВы поднялись на следующий этаж!\n");

            int fcost = floor_stamina_cost(p);
            p->stamina -= fcost;
            if (p->stamina < 0) p->stamina = 0;

            gs->current_hour++;
            if (gs->current_hour >= HOURS_IN_DAY) {
                gs->current_hour = 0;
                gs->current_day++;
            }

            int reward = floor_gold_reward(p, m->floor);
            p->gold += reward;
            printf("Вы получили %d золота за исследование этажа %d!\n",
                reward, m->floor);

            handle_relic_find(p);

            m->floor++;

            for (int i = 0; i < m->height; i++) free(m->cells[i]);
            free(m->cells);

            m->height = 7 + (rand() % 6) * 2;
            m->width = 7 + (rand() % 6) * 2;
            m->is_vertical = rand() % 2;
            m->wall_active = 1;

            m->cells = (char**)malloc(m->height * sizeof(char*));
            for (int i = 0; i < m->height; i++)
                m->cells[i] = (char*)malloc(m->width * sizeof(char));

            map_generate_floor(m, &p->x, &p->y);
        }

        if (p->stamina <= 0 || gs->current_hour == NIGHT_HOUR) {
            emergency = 1;
            printf("\n*** Вы падаете без сил! ***\n");
            if (gs->current_hour == NIGHT_HOUR)
                printf("Наступила ночь (02:00).\n");
            else
                printf("Выносливость исчерпана.\n");
            break;
        }
    }

    if (emergency) {
        int lost = p->gold / 2;
        p->gold -= lost;
        printf("Спасательная служба доставила вас домой, забрав %d золота за услуги.\n",
            lost);
        printf("Осталось золота: %d\n", p->gold);
    }

    map_free(m);
    gs->map = NULL;
}