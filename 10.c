#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TYPE_EMPTY,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_STRING
} DataType;

typedef union {
    int int_value;
    float float_value;
    char char_value;
    char *string_value;
} Data;

typedef struct {
    Data data;
    DataType type;
} Container;

/* API: every Container must be initialized before any other operation. */
static void container_init(Container *container);
static int container_set_int(Container *container, int value);
static int container_set_float(Container *container, float value);
static int container_set_char(Container *container, char value);
static int container_set_string(Container *container, const char *value);
static int container_get_int(const Container *container, int *out_value);
static int container_get_float(const Container *container, float *out_value);
static int container_get_char(const Container *container, char *out_value);
static int container_get_string(const Container *container, const char **out_value);
static void container_clear(Container *container);
static int container_copy(Container *destination, const Container *source);
static const char *container_type_name(const Container *container);
static void container_print(const Container *container);

static void container_init(Container *container) {
    if (container != NULL) {
        container->data.string_value = NULL;
        container->type = TYPE_EMPTY;
    }
}

static void container_clear(Container *container) {
    if (container == NULL) {
        return;
    }
    if (container->type == TYPE_STRING) {
        free(container->data.string_value);
    }
    container->data.string_value = NULL;
    container->type = TYPE_EMPTY;
}

static int container_set_int(Container *container, int value) {
    if (container == NULL) {
        return 0;
    }
    container_clear(container);
    container->data.int_value = value;
    container->type = TYPE_INT;

    return 1;
}

static int container_set_float(Container *container, float value) {
    if (container == NULL) {
        return 0;
    }
    container_clear(container);
    container->data.float_value = value;
    container->type = TYPE_FLOAT;

    return 1;
}

static int container_set_char(Container *container, char value) {
    if (container == NULL) {
        return 0;
    }
    container_clear(container);
    container->data.char_value = value;
    container->type = TYPE_CHAR;

    return 1;
}

static int container_set_string(Container *container, const char *value) {
    size_t length;
    char *copy;
    if (container == NULL || value == NULL) {
        return 0;
    }
    length = strlen(value);
    if (length == (size_t)-1) {
        return 0;
    }
    copy = (char *)malloc(length + 1);
    if (copy == NULL) {
        return 0;
    }
    memcpy(copy, value, length + 1);
    container_clear(container);
    container->data.string_value = copy;
    container->type = TYPE_STRING;

    return 1;
}

static int container_get_int(const Container *container, int *out_value) {
    if (container == NULL || out_value == NULL || container->type != TYPE_INT) {
        return 0;
    }
    *out_value = container->data.int_value;

    return 1;
}

static int container_get_float(const Container *container, float *out_value) {
    if (container == NULL || out_value == NULL || container->type != TYPE_FLOAT) {
        return 0;
    }
    *out_value = container->data.float_value;

    return 1;
}

static int container_get_char(const Container *container, char *out_value) {
    if (container == NULL || out_value == NULL || container->type != TYPE_CHAR) {
        return 0;
    }
    *out_value = container->data.char_value;

    return 1;
}

static int container_get_string(const Container *container, const char **out_value) {
    if (container == NULL || out_value == NULL || container->type != TYPE_STRING ||
        container->data.string_value == NULL) {
        return 0;
    }
    *out_value = container->data.string_value;

    return 1;
}

/* Destination must be initialized. Self-copy is a successful no-op. */
static int container_copy(Container *destination, const Container *source) {
    char *string_copy;
    size_t length;
    if (destination == NULL || source == NULL) {
        return 0;
    }
    if (destination == source) {
        return 1;
    }
    if (source->type == TYPE_STRING) {
        if (source->data.string_value == NULL) {
            return 0;
        }
        length = strlen(source->data.string_value);
        string_copy = (char *)malloc(length + 1);
        if (string_copy == NULL) {
            return 0;
        }
        memcpy(string_copy, source->data.string_value, length + 1);
        container_clear(destination);
        destination->data.string_value = string_copy;
        destination->type = TYPE_STRING;

        return 1;
    }
    if (source->type == TYPE_EMPTY) {
        container_clear(destination);
        return 1;
    }
    if (source->type != TYPE_INT && source->type != TYPE_FLOAT && source->type != TYPE_CHAR) {
        return 0;
    }
    container_clear(destination);
    destination->data = source->data;
    destination->type = source->type;

    return 1;
}

static const char *container_type_name(const Container *container) {
    if (container == NULL) {
        return "INVALID";
    }
    switch (container->type) {
        case TYPE_EMPTY: return "EMPTY";
        case TYPE_INT: return "INT";
        case TYPE_FLOAT: return "FLOAT";
        case TYPE_CHAR: return "CHAR";
        case TYPE_STRING: return "STRING";
        default: return "INVALID";
    }
}

static void container_print(const Container *container) {
    if (container == NULL) {
        printf("<invalid container>");
        return;
    }
    switch (container->type) {
        case TYPE_EMPTY:
            printf("<empty>");
            break;
        case TYPE_INT:
            printf("%d", container->data.int_value);
            break;
        case TYPE_FLOAT:
            printf("%.2f", container->data.float_value);
            break;
        case TYPE_CHAR:
            printf("%c", container->data.char_value);
            break;
        case TYPE_STRING:
            if (container->data.string_value != NULL) {
                printf("\"%s\"", container->data.string_value);
            } else {
                printf("<invalid string>");
            }
            break;
        default:
            printf("<invalid type>");
            break;
    }
}

int main(void) {
    Container *records;
    Container string_copy;
    Container transition;
    size_t i;
    int int_value;
    float float_value;
    char char_value;
    const char *text_value;

    records = (Container *)malloc(6 * sizeof *records);
    if (records == NULL) {
        fprintf(stderr, "Unable to allocate container array.\n");
        return 1;
    }
    for (i = 0; i < 6; i++) {
        container_init(&records[i]);
    }
    container_init(&string_copy);
    container_init(&transition);

    if (!container_set_int(&records[0], 10) ||
        !container_set_float(&records[1], 3.14f) ||
        !container_set_char(&records[2], 'A') ||
        !container_set_string(&records[3], "Hello, World!") ||
        !container_set_int(&records[4], 0) ||
        !container_set_string(&records[5], "Data")) {
        fprintf(stderr, "Unable to set up demonstration values.\n");
        goto cleanup;
    }

    printf("Mixed container records:\n");
    for (i = 0; i < 6; i++) {
        printf("[%zu] %-6s ", i, container_type_name(&records[i]));
        container_print(&records[i]);
        putchar('\n');
    }

    printf("\nDeep-copy demonstration:\n");
    if (!container_copy(&string_copy, &records[5])) {
        fprintf(stderr, "Unable to copy string value.\n");
        goto cleanup;
    }
    container_set_string(&records[5], "Changed original");
    printf("Original: "); container_print(&records[5]);
    printf(" | Copy: "); container_print(&string_copy); putchar('\n');
    printf("Self-copy succeeds: %s\n", container_copy(&string_copy, &string_copy) ? "yes" : "no");

    printf("\nType transitions STRING -> INT -> STRING -> FLOAT:\n");
    container_set_string(&transition, "first text");
    container_set_int(&transition, 25);
    container_set_string(&transition, "second text");
    container_set_float(&transition, 8.5f);
    printf("Current value (%s): ", container_type_name(&transition));
    container_print(&transition);
    putchar('\n');

    printf("\nGetter checks:\n");
    if (container_get_int(&records[4], &int_value)) {
        printf("Integer zero retrieved successfully: %d\n", int_value);
    }
    printf("Wrong-type integer getter rejected: %s\n",
           container_get_int(&records[1], &int_value) ? "no" : "yes");
    if (container_get_float(&records[1], &float_value)) {
        printf("Float getter returned: %.2f\n", float_value);
    }
    if (container_get_char(&records[2], &char_value)) {
        printf("Character getter returned: %c\n", char_value);
    }
    container_set_string(&transition, "");
    if (container_get_string(&transition, &text_value)) {
        printf("Empty string is valid (length %zu).\n", strlen(text_value));
    }
    container_clear(&transition);
    container_clear(&transition);
    printf("Repeated clear leaves type %s.\n", container_type_name(&transition));

cleanup:
    for (i = 0; i < 6; i++) {
        container_clear(&records[i]);
    }
    container_clear(&string_copy);
    container_clear(&transition);
    free(records);

    return 0;
}
