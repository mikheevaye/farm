#include "utils.h"

int fool_check(int scanf_result) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    if (scanf_result != 1) {
        printf("Ошибка ввода. Введите целое число.\n");
        return 0;
    }
    return 1;
}

static const char* DEFAULT_ITEM_NAMES[MAX_ITEMS] = {
    "Пусто", "Дерево", "Камень", "Семена", "Железо",
    "Вода", "Факел", "Золото", "Верёвка", "Коса"
};

int items_catalog(char names[MAX_ITEMS][MAX_NAME_LEN]) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        strncpy(names[i], DEFAULT_ITEM_NAMES[i], MAX_NAME_LEN - 1);
        names[i][MAX_NAME_LEN - 1] = '\0';
    }

    FILE* file = fopen("items.txt", "r");
    if (file == NULL) file = fopen("data/items.txt", "r");
    if (file == NULL) {
        printf("Предупреждение: файл items.txt не найден! Каталог по умолчанию.\n");
        return 0;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file) != NULL) {
        int id;
        char name[MAX_NAME_LEN];
        char format[32];
        sprintf(format, "%%d %%%d[^\n]", MAX_NAME_LEN - 1);
        if (sscanf(line, format, &id, name) == 2) {
            if (id >= 0 && id < MAX_ITEMS && name[0] != '\0') {
                strncpy(names[id], name, MAX_NAME_LEN - 1);
                names[id][MAX_NAME_LEN - 1] = '\0';
            }
        }
    }
    fclose(file);
    return 1;
}

int find_item_id_by_name(char names[][MAX_NAME_LEN], const char* word) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(names[i], word) == 0) return i;
    }
    return -1;
}

int find_inventory_slots(const ItemType inv[], int size, int id,
    int slots[], int max_slots) {
    int count = 0;
    for (int i = 0; i < size && count < max_slots; i++) {
        if ((int)inv[i] == id) slots[count++] = i;
    }
    return count;
}

void write_diary_entry(FILE* out_screen, FILE* out_file,
    const char* player_name, int day, int hour,
    const ItemType inv[], int size,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE* streams[2] = { out_screen, out_file };
    for (int s = 0; s < 2; s++) {
        FILE* out = streams[s];
        if (out == NULL) continue;
        fprintf(out, "===== Дневник фермера =====\n");
        fprintf(out, "Имя: %s\n", player_name);
        fprintf(out, "День: %d, Время: %02d:00\n", day, hour);
        fprintf(out, "Инвентарь:\n");
        for (int i = 0; i < size; i++) {
            int id = (int)inv[i];
            const char* name = (id >= 0 && id < MAX_ITEMS)
                ? item_names[id] : "Неизвестный";
            fprintf(out, "  Слот %d: [%d] - %s\n", i, id, name);
        }
        fprintf(out, "==========================\n\n");
    }
}

void save_diary_action(const Player* player, int day, int hour,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    if (player == NULL) {
        printf("Сначала создайте персонажа!\n");
        return;
    }
    FILE* diary = fopen("diary.txt", "a");
    if (diary == NULL) {
        printf("Ошибка: не удалось открыть diary.txt\n");
        return;
    }
    write_diary_entry(stdout, diary, player->name, day, hour,
        player->inventory, INVENTORY_SIZE, item_names);
    fclose(diary);
    printf("Запись сохранена в diary.txt!\n");
}

int process_quests_to_streams(FILE* out_screen, FILE* out_file,
    const char* filename, int current_day,
    const ItemType inv[], int inv_size,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) file = fopen("data/input.txt", "r");
    if (file == NULL) return -1;

    fprintf(out_screen, "=== Доска объявлений (день %d) ===\n", current_day);
    if (out_file) fprintf(out_file, "=== Доска объявлений (день %d) ===\n", current_day);

    char line[MAX_LINE_LEN];
    int any = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        int day, reward;
        char customer[MAX_NAME_LEN], item[MAX_NAME_LEN];
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;
        int parsed = sscanf(line, "%d | %31[^|] | %31[^|] | %d",
            &day, customer, item, &reward);
        if (parsed != 4) continue;
        if (day > current_day) continue;

        int item_id = find_item_id_by_name(item_names, item);
        int ready = 0;
        if (item_id >= 0) {
            int slots[INVENTORY_SIZE];
            ready = find_inventory_slots(inv, inv_size, item_id,
                slots, INVENTORY_SIZE) > 0;
        }
        const char* status = ready ? "[ГОТОВО К СДАЧЕ]" : "[В ПРОЦЕССЕ]";
        fprintf(out_screen, "День %d | %s | %s | Награда: %d | %s\n",
            day, customer, item, reward, status);
        if (out_file) {
            fprintf(out_file, "День %d | %s | %s | Награда: %d | %s\n",
                day, customer, item, reward, status);
        }
        any = 1;
    }

    if (!any) {
        fprintf(out_screen, "Нет доступных заказов.\n");
        if (out_file) fprintf(out_file, "Нет доступных заказов.\n");
    }
    fclose(file);
    return 0;
}

void process_quests_action(int day, const ItemType inv[],
    char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE* out_file = fopen("output.txt", "w");
    if (out_file == NULL) {
        printf("Ошибка: не удалось создать output.txt\n");
        return;
    }
    int res = process_quests_to_streams(stdout, out_file, "input.txt",
        day, inv, INVENTORY_SIZE, item_names);
    fclose(out_file);
    if (res != 0) printf("Ошибка: файл input.txt не найден!\n");
    else          printf("Заказы записаны в output.txt!\n");
}

void print_time(int day, int hour) {
    printf("Текущее время: День %d, %02d:00\n", day, hour);
}

int advance_time(int* day, int* hour, int hours) {
    if (hours < 1) {
        printf("Можно промотать только положительное число часов.\n");
        return 0;
    }
    if (*hour == NIGHT_HOUR) {
        printf("Уже 02:00. Фермер должен спать! Используйте «Отдых».\n");
        return 0;
    }
    int advanced = 0;
    for (int i = 0; i < hours; i++) {
        (*hour)++;
        advanced++;
        if (*hour >= HOURS_IN_DAY) {
            *hour = 0;
            (*day)++;
        }
        if (*hour == NIGHT_HOUR) {
            printf("Достигнуто 02:00 ночи. Дальше нельзя — нужно спать.\n");
            break;
        }
    }
    return advanced;
}

void sleep_until_morning(int* day, int* hour, Player* player) {
    *day += 1;
    *hour = MORNING_HOUR;
    if (player) {
        player->hp = player->max_hp;
        player->stamina = player->max_stamina;
        printf("Вы выспались. Здоровье и выносливость восстановлены.\n");
    }
    printf("Наступило утро. День %d, %02d:00\n", *day, *hour);
}

/* ===== Реликвии ===== */
static const char* RELIC_NAMES[RELIC_COUNT] = {
    "Нет",
    "Кольцо жадности",
    "Сапоги-скороходы",
    "Камень тролля"
};

Relic get_random_relic(void) {
    Relic r;
    int idx = 1 + rand() % (RELIC_COUNT - 1);
    r.effect = (RelicEffect)idx;
    strncpy(r.name, RELIC_NAMES[idx], MAX_NAME_LEN - 1);
    r.name[MAX_NAME_LEN - 1] = '\0';
    return r;
}

const char* relic_effect_description(RelicEffect effect) {
    switch (effect) {
    case RELIC_GREED: return "x2 золото за этаж, но шаг стоит 2 стамины";
    case RELIC_SPEED: return "смена этажа стоит 1 стамину вместо 5";
    case RELIC_TROLL: return "+1 HP за каждый шаг";
    default:          return "нет эффекта";
    }
}