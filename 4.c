#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_LIMIT 49
#define INPUT_SIZE 256

typedef struct
{
    int roll_number;
    char name[NAME_LIMIT + 1];
    int age;
    float gpa;
} Student;

static int read_line(const char *prompt, char *buffer, size_t size)
{
    size_t length;
    int ch;
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL)
    {
        return 0;
    }
    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n')
    {
        buffer[length - 1] = '\0';
    }
    else
    {
        while ((ch = getchar()) != '\n' && ch != EOF)
        {
        }
        if (length == size - 1)
        {
            printf("Input is too long. Please try again.\n");
            return -1;
        }
    }
    return 1;
}

static int read_long_value(const char *prompt, long *value)
{
    char line[INPUT_SIZE], *end;
    int status;
    for (;;)
    {
        status = read_line(prompt, line, sizeof line);
        if (status == 0)
            return 0;
        if (status < 0)
            continue;
        errno = 0;
        *value = strtol(line, &end, 10);
        while (*end == ' ' || *end == '\t' || *end == '\r')
            end++;
        if (line[0] != '\0' && *end == '\0' && errno != ERANGE)
        {
            return 1;
        }
        printf("Please enter a valid whole number.\n");
    }
}

static int read_gpa_value(const char *prompt, float *value)
{
    char line[INPUT_SIZE], *end;
    int status;
    for (;;)
    {
        status = read_line(prompt, line, sizeof line);
        if (status == 0)
            return 0;
        if (status < 0)
            continue;
        errno = 0;
        *value = strtof(line, &end);
        while (*end == ' ' || *end == '\t' || *end == '\r')
            end++;
        if (line[0] != '\0' && *end == '\0' && errno != ERANGE &&
            *value >= 0.0f && *value <= 4.0f)
        {
            return 1;
        }
        printf("GPA must be a number from 0.00 through 4.00.\n");
    }
}

static int read_name(char name[NAME_LIMIT + 1])
{
    char line[INPUT_SIZE];
    int status;
    for (;;)
    {
        status = read_line("Name (up to 49 characters): ", line, sizeof line);
        if (status == 0)
            return 0;
        if (status < 0)
            continue;
        if (line[0] == '\0')
        {
            printf("Name cannot be empty.\n");
            continue;
        }
        if (strlen(line) > NAME_LIMIT)
        {
            printf("Name must be no longer than %d characters.\n", NAME_LIMIT);
            continue;
        }
        strcpy(name, line);
        return 1;
    }
}

static Student *create_student_list(size_t capacity)
{
    if (capacity == 0 || capacity > (size_t)-1 / sizeof(Student))
        return NULL;
    return (Student *)malloc(capacity * sizeof(Student));
}

static int ensure_capacity(Student **students, size_t *capacity, size_t required)
{
    size_t new_capacity = *capacity;
    Student *resized;
    if (required <= *capacity)
        return 1;
    while (new_capacity < required)
    {
        if (new_capacity > (size_t)-1 / 2)
        {
            new_capacity = required;
            break;
        }
        new_capacity *= 2;
    }
    if (new_capacity > (size_t)-1 / sizeof(Student))
        return 0;
    resized = (Student *)realloc(*students, new_capacity * sizeof(Student));
    if (resized == NULL)
        return 0;
    *students = resized;
    *capacity = new_capacity;
    return 1;
}

static int find_student(const Student *students, size_t count, int roll_number)
{
    size_t i;
    for (i = 0; i < count; i++)
        if (students[i].roll_number == roll_number)
            return (int)i;
    return -1;
}

static void display_student(const Student *student)
{
    printf("Roll: %d | Name: %s | Age: %d | GPA: %.2f\n",
           student->roll_number, student->name, student->age, student->gpa);
}

static void display_students(const Student *students, size_t count)
{
    size_t i;
    if (count == 0)
    {
        printf("No student records are stored.\n");
        return;
    }
    for (i = 0; i < count; i++)
        display_student(&students[i]);
}

static int read_valid_age(int *age)
{
    long value;
    for (;;)
    {
        if (!read_long_value("Age (16-100): ", &value))
            return 0;
        if (value >= 16 && value <= 100)
        {
            *age = (int)value;
            return 1;
        }
        printf("Age must be between 16 and 100.\n");
    }
}

static int read_valid_roll(int *roll)
{
    long value;
    for (;;)
    {
        if (!read_long_value("Positive roll number: ", &value))
            return 0;
        if (value > 0 && value <= INT_MAX)
        {
            *roll = (int)value;
            return 1;
        }
        printf("Roll number must be a positive integer.\n");
    }
}

static int add_student(Student **students, size_t *count, size_t *capacity)
{
    Student record;
    if (!read_valid_roll(&record.roll_number))
        return 0;
    if (find_student(*students, *count, record.roll_number) >= 0)
    {
        printf("Error: that roll number already exists.\n");
        return 1;
    }
    if (!read_name(record.name) || !read_valid_age(&record.age) ||
        !read_gpa_value("GPA (0.00-4.00): ", &record.gpa))
        return 0;
    if (*count == (size_t)-1 || !ensure_capacity(students, capacity, *count + 1))
    {
        printf("Error: unable to grow the student list; no record was added.\n");
        return 1;
    }
    (*students)[*count] = record;
    (*count)++;
    printf("Student added successfully.\n");
    return 1;
}

static int update_student(Student *students, size_t count)
{
    int roll;
    int index;
    Student replacement;
    if (!read_valid_roll(&roll))
        return 0;
    index = find_student(students, count, roll);
    if (index < 0)
    {
        printf("No student found with roll number %d.\n", roll);
        return 1;
    }
    replacement.roll_number = roll;
    if (!read_name(replacement.name) || !read_valid_age(&replacement.age) ||
        !read_gpa_value("GPA (0.00-4.00): ", &replacement.gpa))
        return 0;
    students[index] = replacement;
    printf("Student updated successfully.\n");
    return 1;
}

static int delete_student(Student *students, size_t *count)
{
    int roll, index;
    size_t i;
    if (!read_valid_roll(&roll))
        return 0;
    index = find_student(students, *count, roll);
    if (index < 0)
    {
        printf("No student found with roll number %d.\n", roll);
        return 1;
    }
    for (i = (size_t)index; i + 1 < *count; i++)
        students[i] = students[i + 1];
    (*count)--;
    printf("Student deleted successfully.\n");
    return 1;
}

static void show_summary(const Student *students, size_t count)
{
    size_t i, high = 0, low = 0;
    double total = 0.0;
    if (count == 0)
    {
        printf("No GPA summary is available because the collection is empty.\n");
        return;
    }
    for (i = 0; i < count; i++)
    {
        total += students[i].gpa;
        if (students[i].gpa > students[high].gpa)
            high = i;
        if (students[i].gpa < students[low].gpa)
            low = i;
    }
    printf("Highest GPA: ");
    display_student(&students[high]);
    printf("Lowest GPA:  ");
    display_student(&students[low]);
    printf("Average GPA: %.2f\n", total / count);
}

static int compare_students(const void *left, const void *right)
{
    const Student *a = (const Student *)left;
    const Student *b = (const Student *)right;
    if (a->gpa < b->gpa)
        return 1;
    if (a->gpa > b->gpa)
        return -1;
    if (a->roll_number < b->roll_number)
        return -1;
    if (a->roll_number > b->roll_number)
        return 1;
    return 0;
}

static void display_sorted(const Student *students, size_t count)
{
    Student *copy;
    if (count == 0)
    {
        printf("No student records are stored.\n");
        return;
    }
    if (count > (size_t)-1 / sizeof(Student) ||
        (copy = (Student *)malloc(count * sizeof(Student))) == NULL)
    {
        printf("Unable to allocate memory for the sorted view.\n");
        return;
    }
    memcpy(copy, students, count * sizeof(Student));
    qsort(copy, count, sizeof(Student), compare_students);
    printf("Students sorted by GPA (descending), then roll number (ascending):\n");
    display_students(copy, count);
    free(copy);
}

int main(void)
{
    long requested;
    Student *students;
    size_t count = 0, capacity;
    int running = 1;
    printf("Student Records and Academic Analytics System\n");
    for (;;)
    {
        if (!read_long_value("Initial capacity (positive whole number): ", &requested))
            return 0;
        if (requested > 0 && (unsigned long)requested <= (size_t)-1 / sizeof(Student))
            break;
        printf("Capacity must be positive and small enough to allocate safely.\n");
    }
    capacity = (size_t)requested;
    students = create_student_list(capacity);
    if (students == NULL)
    {
        printf("Unable to allocate the requested student list. Exiting.\n");
        return 1;
    }

    while (running)
    {
        long choice;
        printf("\n1 Add  2 Display  3 Search  4 Update  5 Delete\n"
               "6 GPA summary  7 Sorted view  0 Exit\n");
        if (!read_long_value("Choose an operation: ", &choice))
            break;
        switch (choice)
        {
        case 1:
            if (!add_student(&students, &count, &capacity))
                running = 0;
            break;
        case 2:
            display_students(students, count);
            break;
        case 3:
        {
            int roll, index;
            if (!read_valid_roll(&roll))
            {
                running = 0;
                break;
            }
            index = find_student(students, count, roll);
            if (index < 0)
                printf("No student found with roll number %d.\n", roll);
            else
                display_student(&students[index]);
            break;
        }
        case 4:
            if (!update_student(students, count))
                running = 0;
            break;
        case 5:
            if (!delete_student(students, &count))
                running = 0;
            break;
        case 6:
            show_summary(students, count);
            break;
        case 7:
            display_sorted(students, count);
            break;
        case 0:
            running = 0;
            break;
        default:
            printf("Unknown option. Choose a number from 0 to 7.\n");
        }
    }
    free(students);
    printf("Student list memory released. Goodbye.\n");
    return 0;
}

