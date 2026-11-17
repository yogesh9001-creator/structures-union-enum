#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef union
{
    int integer;
    float floating;
    char text[20];
} UnionData;

typedef struct
{
    int integer;
    float floating;
    char text[20];
} StructData;

typedef enum
{
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_TEXT
} ValueType;

typedef struct
{
    UnionData value;
    ValueType type;
} TaggedValue;

static int set_integer(TaggedValue *record, int value)
{
    if (record == NULL)
    {
        return 0;
    }

    record->value.integer = value;
    record->type = VALUE_INT;

    return 1;
}

static int set_float(TaggedValue *record, float value)
{
    if (record == NULL || !isfinite(value))
    {
        return 0;
    }

    record->value.floating = value;
    record->type = VALUE_FLOAT;

    return 1;
}

/* The terminating null byte must fit inside the 20-byte text member. */
static int set_text(TaggedValue *record, const char *text)
{
    size_t length;

    if (record == NULL || text == NULL)
    {
        return 0;
    }

    length = strlen(text);
    if (length >= sizeof record->value.text)
    {
        return 0;
    }

    memcpy(record->value.text, text, length + 1);
    record->type = VALUE_TEXT;

    return 1;
}

static void display_tagged_value(const TaggedValue *record)
{
    if (record == NULL)
    {
        puts("Invalid record pointer.");
        return;
    }

    switch (record->type)
    {
    case VALUE_INT:
    {
        printf("Integer: %d\n", record->value.integer);
        break;
    }
    case VALUE_FLOAT:
    {
        printf("Float: %.2f\n", record->value.floating);
        break;
    }
    case VALUE_TEXT:
    {
        printf("Text: %s\n", record->value.text);
        break;
    }
    default:
    {
        printf("Invalid value tag (%d); no union member displayed.\n",
               (int)record->type);
        break;
    }
    }
}

static void display_memory_layout(void)
{
    UnionData union_data;
    StructData struct_data;

    puts("Memory layout (platform-dependent):");
    printf("UnionData: size=%zu bytes, alignment=%zu bytes\n",
           sizeof(UnionData), _Alignof(UnionData));
    printf("StructData: size=%zu bytes, alignment=%zu bytes\n",
           sizeof(StructData), _Alignof(StructData));
    printf("Union member addresses: integer=%p, floating=%p, text=%p\n",
           (void *)&union_data.integer,
           (void *)&union_data.floating,
           (void *)&union_data.text);
    printf("Struct member addresses: integer=%p, floating=%p, text=%p\n",
           (void *)&struct_data.integer,
           (void *)&struct_data.floating,
           (void *)&struct_data.text);

    printf("Struct field offsets: integer=%zu, floating=%zu, text=%zu\n",
           offsetof(StructData, integer),
           offsetof(StructData, floating),
           offsetof(StructData, text));

    printf("Storage for 100 StructData records: %zu bytes\n",
           (size_t)100 * sizeof(StructData));
    printf("Storage for 100 UnionData records:  %zu bytes\n",
           (size_t)100 * sizeof(UnionData));

    puts("Union members begin at the same address. Struct fields have separate offsets.");
    puts("Offsets may include padding before fields to meet alignment requirements;");
    puts("the compiler may also add trailing padding to the complete structure or union.");
}

int main(void)
{
    UnionData union_demo;
    StructData structure_demo = {10, 220.5f, "Struct Programming"};
    TaggedValue records[5] = {0};
    TaggedValue switching_record = {0};
    TaggedValue invalid_record = {0};
    const char maximum_text[] = "1234567890123456789";
    const char overlength_text[] = "12345678901234567890";
    size_t index;

    display_memory_layout();

    printf("\nStructure values: integer=%d, float=%.2f, text=\"%s\"\n",
           structure_demo.integer, structure_demo.floating, structure_demo.text);

    puts("\nUnion assignments (each value is read before the next assignment):");
    union_demo.integer = 10;
    printf("Integer: %d\n", union_demo.integer);
    union_demo.floating = 220.5f;
    printf("Float: %.2f\n", union_demo.floating);
    memcpy(union_demo.text, "Union Programming", sizeof "Union Programming");
    printf("Text: %s\n", union_demo.text);

    if (!set_integer(&records[0], 10) ||
        !set_float(&records[1], 220.5f) ||
        !set_text(&records[2], "Union Programming") ||
        !set_integer(&records[3], 42) ||
        !set_text(&records[4], "Typed record"))
    {
        fputs("Unable to initialize tagged records.\n", stderr);

        return 1;
    }

    puts("\nFive mixed tagged records:");
    for (index = 0; index < sizeof records / sizeof records[0]; index++)
    {
        printf("Record %zu: ", index + 1);
        display_tagged_value(&records[index]);
    }

    puts("\nValidation and tag checks:");
    printf("19-character string accepted: %s\n",
           set_text(&switching_record, maximum_text) ? "yes" : "no");
    printf("20-character string accepted: %s\n",
           set_text(&switching_record, overlength_text) ? "yes" : "no");

    if (!set_integer(&switching_record, 7) ||
        !set_text(&switching_record, "now text") ||
        !set_float(&switching_record, 3.25f))
    {
        fputs("Unable to switch tagged record values.\n", stderr);

        return 1;
    }
    printf("After integer -> text -> float: ");
    display_tagged_value(&switching_record);

    invalid_record.type = (ValueType)99;
    printf("Invalid tag: ");
    display_tagged_value(&invalid_record);

    printf("\nTaggedValue size=%zu bytes; UnionData size=%zu bytes; StructData size=%zu bytes\n",
           sizeof(TaggedValue), sizeof(UnionData), sizeof(StructData));
    puts("Observation: a union saves space when a record stores one of several types at a time.");
    puts("Its members overlap, so their values cannot coexist independently; the tag identifies");
    puts("which member is active. Structure fields coexist in separate storage. Alignment and");
    puts("padding can change offsets and total sizes, so measurements are platform-specific.");

    return 0;
}
