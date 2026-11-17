
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DESCRIPTION_MAX 99
#define LINE_SIZE 256

typedef enum {
    CMD_ADD, CMD_DELETE, CMD_VIEW, CMD_UPDATE, CMD_COMPLETE,
    CMD_FIND, CMD_HELP, CMD_EXIT, CMD_UNKNOWN
} CommandType;

typedef enum { PENDING, DONE } TaskStatus;

typedef struct {
    int id;
    char description[DESCRIPTION_MAX + 1];
    TaskStatus status;
} Task;

typedef struct {
    Task *items;
    size_t count;
    size_t capacity;
} TaskList;

/* Reads one complete line; returns 0 on EOF and -1 when the line is too long. */
static int read_line(const char *prompt, char *buffer, size_t size) {
    size_t length;
    int ch;
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }
    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
        return 1;
    }
    if (feof(stdin)) {
        return 1;
    }
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
    return -1;
}

static CommandType get_command_type(const char *command) {
    char normalized[LINE_SIZE];
    size_t i, length = strlen(command);
    if (length >= sizeof normalized) {
        return CMD_UNKNOWN;
    }
    for (i = 0; i <= length; i++) {
        normalized[i] = (char)toupper((unsigned char)command[i]);
    }
    if (strcmp(normalized, "ADD") == 0) { return CMD_ADD; }
    if (strcmp(normalized, "DELETE") == 0) { return CMD_DELETE; }
    if (strcmp(normalized, "VIEW") == 0) { return CMD_VIEW; }
    if (strcmp(normalized, "UPDATE") == 0) { return CMD_UPDATE; }
    if (strcmp(normalized, "COMPLETE") == 0) { return CMD_COMPLETE; }
    if (strcmp(normalized, "FIND") == 0) { return CMD_FIND; }
    if (strcmp(normalized, "HELP") == 0) { return CMD_HELP; }
    if (strcmp(normalized, "EXIT") == 0) { return CMD_EXIT; }
    return CMD_UNKNOWN;
}

static int read_task_id(int *id) {
    char line[LINE_SIZE], *end;
    long value;
    int status;
    for (;;) {
        status = read_line("Task ID (positive integer): ", line, sizeof line);
        if (status == 0) { return 0; }
        if (status < 0) {
            puts("ID input was too long. Please try again.");
            continue;
        }
        errno = 0;
        value = strtol(line, &end, 10);
        while (*end == ' ' || *end == '\t' || *end == '\r') end++;
        if (line[0] != '\0' && *end == '\0' && errno != ERANGE && value > 0 && value <= INT_MAX) {
            *id = (int)value;
            return 1;
        }
        puts("Invalid ID. Enter a positive whole number.");
    }
}

static int read_description(char description[DESCRIPTION_MAX + 1]) {
    char line[LINE_SIZE];
    int status;
    for (;;) {
        status = read_line("Description (1-99 characters): ", line, sizeof line);
        if (status == 0) { return 0; }
        if (status < 0) {
            puts("Description is too long. Please try again.");
            continue;
        }
        if (line[0] == '\0') {
            puts("Description cannot be empty.");
            continue;
        }
        if (strlen(line) > DESCRIPTION_MAX) {
            puts("Description must be at most 99 characters.");
            continue;
        }
        strcpy(description, line);
        return 1;
    }
}

static int find_index(const TaskList *list, int id) {
    size_t i;
    for (i = 0; i < list->count; i++) {
        if (list->items[i].id == id) { return (int)i; }
    }
    return -1;
}

static int ensure_capacity(TaskList *list) {
    size_t next;
    Task *resized;
    if (list->count < list->capacity) { return 1; }
    if (list->capacity > (size_t)-1 / 2) { return 0; }
    next = list->capacity == 0 ? 4 : list->capacity * 2;
    if (next > (size_t)-1 / sizeof(Task)) { return 0; }
    resized = (Task *)realloc(list->items, next * sizeof(Task));
    if (resized == NULL) { return 0; }
    list->items = resized;
    list->capacity = next;
    return 1;
}

static void print_task(const Task *task) {
    printf("ID: %d | Description: %s | Status: %s\n", task->id,
           task->description, task->status == DONE ? "DONE" : "PENDING");
}

static int add_task(TaskList *list) {
    Task task;
    if (!read_task_id(&task.id)) { return 0; }
    if (find_index(list, task.id) >= 0) {
        printf("Error: task ID %d already exists.\n", task.id);
        return 1;
    }
    if (!read_description(task.description)) { return 0; }
    if (!ensure_capacity(list)) {
        puts("Error: unable to allocate more space; task was not added.");
        return 1;
    }
    task.status = PENDING;
    list->items[list->count++] = task;
    printf("Task %d added successfully.\n", task.id);
    return 1;
}

static int delete_task(TaskList *list) {
    int id, index;
    size_t i;
    if (!read_task_id(&id)) { return 0; }
    index = find_index(list, id);
    if (index < 0) {
        printf("No task found with ID %d.\n", id);
        return 1;
    }
    for (i = (size_t)index; i + 1 < list->count; i++) {
        list->items[i] = list->items[i + 1];
    }
    list->count--;
    printf("Task %d deleted successfully.\n", id);
    return 1;
}

static void view_tasks(const TaskList *list) {
    size_t i;
    if (list->count == 0) {
        puts("The task list is empty.");
        return;
    }
    for (i = 0; i < list->count; i++) {
        print_task(&list->items[i]);
    }
}

static int update_task(TaskList *list) {
    int id, index;
    char description[DESCRIPTION_MAX + 1];
    if (!read_task_id(&id)) { return 0; }
    index = find_index(list, id);
    if (index < 0) {
        printf("No task found with ID %d.\n", id);
        return 1;
    }
    if (!read_description(description)) { return 0; }
    strcpy(list->items[index].description, description);
    printf("Task %d updated successfully.\n", id);
    return 1;
}

static int complete_task(TaskList *list) {
    int id, index;
    if (!read_task_id(&id)) { return 0; }
    index = find_index(list, id);
    if (index < 0) {
        printf("No task found with ID %d.\n", id);
        return 1;
    }
    list->items[index].status = DONE;
    printf("Task %d marked DONE.\n", id);
    return 1;
}

static int find_task(const TaskList *list) {
    int id, index;
    if (!read_task_id(&id)) { return 0; }
    index = find_index(list, id);
    if (index < 0) {
        printf("No task found with ID %d.\n", id);
    } else {
        print_task(&list->items[index]);
    }
    return 1;
}

static void show_help(void) {
    puts("Commands (case-insensitive):\n"
         "  ADD       Add a task with a unique ID and description\n"
         "  DELETE    Delete a task by ID\n"
         "  VIEW      Display all tasks\n"
         "  UPDATE    Change a task description\n"
         "  COMPLETE  Mark a task as done\n"
         "  FIND      Display one task by ID\n"
         "  HELP      Show this help\n"
         "  EXIT      Exit and release memory");
}

int main(void) {
    TaskList list = { NULL, 0, 0 };
    char input[LINE_SIZE];
    int running = 1;
    puts("Task List Manager. Enter HELP to see available commands.");
    while (running) {
        int status = read_line("Command> ", input, sizeof input);
        CommandType command;
        if (status == 0) {
            puts("Input ended. Releasing task list.");
            break;
        }
        if (status < 0) {
            puts("Command line is too long; enter a command such as HELP.");
            continue;
        }
        command = get_command_type(input);
        switch (command) {
            case CMD_ADD: {
                if (!add_task(&list)) { running = 0; }
                break;
            }
            case CMD_DELETE: {
                if (!delete_task(&list)) { running = 0; }
                break;
            }
            case CMD_VIEW: {
                view_tasks(&list);
                break;
            }
            case CMD_UPDATE: {
                if (!update_task(&list)) { running = 0; }
                break;
            }
            case CMD_COMPLETE: {
                if (!complete_task(&list)) { running = 0; }
                break;
            }
            case CMD_FIND: {
                if (!find_task(&list)) { running = 0; }
                break;
            }
            case CMD_HELP: {
                show_help();
                break;
            }
            case CMD_EXIT: {
                puts("Exiting task manager.");
                running = 0;
                break;
            }
            default: {
                puts("Unknown command. Enter HELP to see supported commands.");
                break;
            }
        }
    }
    free(list.items);
    return 0;
}
