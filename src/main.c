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

void get_player_name(char player_name[], int max_len) { 
    printf("Вы вошли в игру 'Веселый фермер'! Введите ваше имя (до 32 символов): ");
    fgets(player_name, max_len, stdin);
    player_name[strcspn(player_name, "\r\n")] = '\0'; 

    if (player_name[0] == '\0') {
        strcpy(player_name, "Name");  // если игрок нажал enter, то деф.имя
    }
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

int items_catalog(char names[MAX_ITEMS][MAX_NAME_LEN]) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        strncpy(names[i], DEFAULT_ITEM_NAMES[i], MAX_NAME_LEN - 1); // Сначала заполняем массив значениями по умолчанию
        names[i][MAX_NAME_LEN - 1] = '\0';
    }

    FILE* file = fopen("items.txt", "r");
    if (file == NULL) {
        printf("Предупреждение: Файл %s не найден! Загружен каталог по умолчанию.\n", "items.txt");
        return 0;   // файл не найден — массив уже заполнен дефолтами
    }

    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), file) != NULL) { 
        int id;
        char name[MAX_NAME_LEN];

        char format[20];
        sprintf(format, "%%d %%%d[^\n]", MAX_NAME_LEN - 1); // Получится "%d %31[^\n]"

        if (sscanf(line, format, &id, name) == 2) {
            if (id >= 0 && id < MAX_ITEMS) {

                if (name[0] != '\0') {
                    strncpy(names[id], name, MAX_NAME_LEN - 1);
                    names[id][MAX_NAME_LEN - 1] = '\0';
                } 
            }
        }
    }

    fclose(file);
    return 1;
}

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

int find_item_id_by_name(char names[][MAX_NAME_LEN], const char* word) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(names[i], word) == 0) {
            return i;
        }
    }

    return -1;
}

void write_diary_entry(FILE* out, const char* player_name,
    int day, int hour, const ItemType inv[], int size,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    fprintf(out, "===== Дневник фермера =====\n");
    fprintf(out, "Имя: %s\n", player_name);
    fprintf(out, "День: %d, Время: %02d:00\n", day, hour);
    fprintf(out, "Инвентарь:\n");

    for (int i = 0; i < size; i++) {
        int id = (int)inv[i];
        // в массиве проверяем лоежит ли предмет в диапазоне
        const char* name = (id >= 0 && id < MAX_ITEMS) ? item_names[id] : "Неизвестный предмет";
        fprintf(out, "  Слот %d: [%d] - %s\n", i, id, name);
    }

    fprintf(out, "==========================\n\n");
}

int find_inventory_slots(const ItemType inv[], int size, int id,
    int slots[], int max_slots) {
    int count = 0;
    for (int i = 0; i < size && count < max_slots; i++) {
        if ((int)inv[i] == id) {
            slots[count] = i;
            count++;
        }
    }
    return count;
}

int process_quests_to_streams(FILE* out_screen, FILE* out_file,
    const char* filename, int current_day,
    const ItemType inv[], int inv_size,
    char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return -1; // input.txt не открылся
    }

    fprintf(out_screen, "=== Доска объявлений (доступно на день %d) ===\n", current_day);
    if (out_file) {
        fprintf(out_file, "=== Доска объявлений (доступно на день %d) ===\n", current_day);
    }

    char line[MAX_LINE_LEN];
    int any_quest = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        int day, reward;
        char customer[MAX_NAME_LEN], item[MAX_NAME_LEN];

        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;

        int parsed = sscanf(line, "%d | %31[^|] | %31[^|] | %d",
            &day, customer, item, &reward);
        if (parsed != 4) continue;

        // только доступные квесты (День_появления <= current_day)
        if (day > current_day) continue;

        // проверка, есть ли вообще требуемый предмет в inventory 
        int item_id = find_item_id_by_name(item_names, item);
        int ready = 0;
        if (item_id >= 0) {
            int slots[INVENTORY_SIZE];
            ready = find_inventory_slots(inv, inv_size, item_id, slots, INVENTORY_SIZE) > 0;
        }

        const char* status = ready ? "[ГОТОВО К СДАЧЕ]" : "[В ПРОЦЕССЕ]";

        // ── вывод на экран И в файл ──
        fprintf(out_screen,
            "День %d | %s | %s | Награда: %d | %s\n",
            day, customer, item, reward, status);
        if (out_file) {
            fprintf(out_file,
                "День %d | %s | %s | Награда: %d | %s\n",
                day, customer, item, reward, status);
        }

        any_quest = 1;
    }

    if (!any_quest) {
        fprintf(out_screen, "Нет доступных заказов.\n");
        if (out_file) fprintf(out_file, "Нет доступных заказов.\n");
    }

    fclose(file);
    return 0;
}

int main(void) {
    SetConsoleOutputCP(CP_UTF8); // чтоб умная консоль выводила человеческий русский

    char item_names[MAX_ITEMS][MAX_NAME_LEN];
    items_catalog(item_names);
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
        printf("\n============= Меню (%s) =============\n", player_name);
        printf("[0] Выход\n");
        printf("[1] Посмотреть на часы\n");
        printf("[2] Промотать время (Поработать)\n");
        printf("[3] Посмотреть инвентарь\n");
        printf("[4] Положить предмет в слот\n");
        printf("[5] Выбросить предмет\n");
        printf("[6] Очистить инвентарь от мусора\n");
        printf("[7] Поиск предмета в рюкзаке по названию\n");
        printf("[8] Записать состояние в дневник фермера (diary.txt) \n");
        printf("[9] Расшифровать старые записи\n");
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

        case 8: {
            FILE* diary = fopen("diary.txt", "a"); // старые записи не трогает
            if (diary == NULL)
            {
                printf("Не удалось открыть diary.txt.\n");
                break;
            }
            write_diary_entry(diary, player_name, current_day, current_hour,
                inventory, INVENTORY_SIZE, item_names);
            fclose(diary);
            printf("Запись добавлена в diary.txt.\n");
            break;
        }

        case 9: {
            FILE* out_file = fopen("output.txt", "w");

            if (out_file == NULL) {
                printf("Ошибка: не удалось открыть файл output.txt для записи.\n");
                break;
            }

            int result = process_quests_to_streams(
                stdout, out_file,          // экран и файл
                "input.txt", current_day,  // входной файл и текущий день
                inventory, INVENTORY_SIZE,
                item_names
            );

            fclose(out_file);

            if (result != 0) {
                printf("Ошибка: не удалось открыть файл input.txt.\n");
                break;
            }

            printf("Результат записан в файл output.txt.\n");
            break;
        }
    }
    }

    return 0;
}