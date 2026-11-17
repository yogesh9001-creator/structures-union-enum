#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RECTANGLES 1000
#define INPUT_SIZE 256
#define SQUARE_TOLERANCE 1e-5

typedef struct 
{
    int id;
    double width;
    double height;
    double area;
    double perimeter;
} Rectangle;

static int read_line(const char *prompt, char *buffer, size_t size)
{
    size_t length;
    int ch;
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL)
        return 0;
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
            printf("Input is too long. Try again.\n");
            return -1;
        }
    }
    return 1;
}

static int read_long(const char *prompt, long *value)
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
        if (line[0] && *end == '\0' && errno != ERANGE)
            return 1;
        printf("Enter a valid whole number.\n");
    }
}

static int read_positive_id(int *id)
{
    long value;
    for (;;)
    {
        if (!read_long("Rectangle ID (positive integer): ", &value))
            return 0;
        if (value > 0 && value <= 2147483647L)
        {
            *id = (int)value;
            return 1;
        }
        printf("ID must be a positive integer.\n");
    }
}

static int read_positive_dimension(const char *prompt, double *dimension)
{
    char line[INPUT_SIZE], *end;
    int status;
    double value;
    for (;;)
    {
        status = read_line(prompt, line, sizeof line);
        if (status == 0)
            return 0;
        if (status < 0)
            continue;
        errno = 0;
        value = strtod(line, &end);
        while (*end == ' ' || *end == '\t' || *end == '\r')
            end++;
        if (line[0] && *end == '\0' && errno != ERANGE && isfinite(value) && value > 0.0)
        {
            *dimension = value;
            return 1;
        }
        printf("Enter a finite number greater than zero (NaN and infinity are not allowed).\n");
    }
}

static int read_nonnegative_number(const char *prompt, double *number)
{
    char line[INPUT_SIZE], *end;
    int status;
    double value;
    for (;;)
    {
        status = read_line(prompt, line, sizeof line);
        if (status == 0)
            return 0;
        if (status < 0)
            continue;
        errno = 0;
        value = strtod(line, &end);
        while (*end == ' ' || *end == '\t' || *end == '\r')
            end++;
        if (line[0] && *end == '\0' && errno != ERANGE && isfinite(value) && value >= 0.0)
        {
            *number = value;
            return 1;
        }
        printf("Enter a finite, non-negative number.\n");
    }
}

static int calculate_geometry(Rectangle *rectangle)
{
    double area = rectangle->width * rectangle->height;
    double perimeter = 2.0 * (rectangle->width + rectangle->height);
    if (!isfinite(area) || !isfinite(perimeter))
        return 0;
    rectangle->area = area;
    rectangle->perimeter = perimeter;
    return 1;
}

static Rectangle *create_rectangle_array(size_t capacity)
{
    if (capacity == 0 || capacity > MAX_RECTANGLES || capacity > (size_t)-1 / sizeof(Rectangle))
        return NULL;
    return (Rectangle *)malloc(capacity * sizeof(Rectangle));
}

static int ensure_capacity(Rectangle **rectangles, size_t *capacity, size_t needed)
{
    size_t next;
    Rectangle *resized;
    if (needed <= *capacity)
        return 1;
    if (needed > MAX_RECTANGLES)
        return 0;
    next = *capacity > MAX_RECTANGLES / 2 ? MAX_RECTANGLES : *capacity * 2;
    if (next < needed)
        next = needed;
    if (next > (size_t)-1 / sizeof(Rectangle))
        return 0;
    resized = (Rectangle *)realloc(*rectangles, next * sizeof(Rectangle));
    if (resized == NULL)
        return 0;
    *rectangles = resized;
    *capacity = next;
    return 1;
}

static int find_rectangle(const Rectangle *rectangles, size_t count, int id)
{
    size_t i;
    for (i = 0; i < count; i++)
        if (rectangles[i].id == id)
            return (int)i;
    return -1;
}

static void display_rectangle(const Rectangle *r)
{
    double scale = fmax(1.0, fmax(fabs(r->width), fabs(r->height)));
    int square = fabs(r->width - r->height) <= SQUARE_TOLERANCE * scale;
    printf("ID: %d | Width: %.2f | Height: %.2f | Area: %.2f | Perimeter: %.2f | %s\n",
           r->id, r->width, r->height, r->area, r->perimeter,
           square ? "Square" : "Non-square");
}

static void display_rectangles(const Rectangle *rectangles, size_t count)
{
    size_t i;
    if (count == 0)
    {
        printf("The rectangle collection is empty.\n");
        return;
    }
    for (i = 0; i < count; i++)
        display_rectangle(&rectangles[i]);
}

static int read_dimensions(Rectangle *r)
{
    for (;;)
    {
        if (!read_positive_dimension("Width: ", &r->width) ||
            !read_positive_dimension("Height: ", &r->height))
            return 0;
        if (calculate_geometry(r))
            return 1;
        printf("These dimensions produce an unrepresentable area or perimeter. Try again.\n");
    }
}

static int add_rectangle(Rectangle **rectangles, size_t *count, size_t *capacity)
{
    Rectangle r;
    if (*count >= MAX_RECTANGLES)
    {
        printf("The collection has reached its limit of %d rectangles.\n", MAX_RECTANGLES);
        return 1;
    }
    if (!read_positive_id(&r.id))
        return 0;
    if (find_rectangle(*rectangles, *count, r.id) >= 0)
    {
        printf("Error: rectangle ID %d already exists.\n", r.id);
        return 1;
    }
    if (!read_dimensions(&r))
        return 0;
    if (!ensure_capacity(rectangles, capacity, *count + 1))
    {
        printf("Unable to expand the collection; no rectangle was added.\n");
        return 1;
    }
    (*rectangles)[(*count)++] = r;
    printf("Rectangle added successfully.\n");
    return 1;
}

static int search_rectangle(const Rectangle *rectangles, size_t count)
{
    int id, index;
    if (!read_positive_id(&id))
        return 0;
    index = find_rectangle(rectangles, count, id);
    if (index < 0)
        printf("No rectangle found with ID %d.\n", id);
    else
        display_rectangle(&rectangles[index]);
    return 1;
}

static int update_rectangle(Rectangle *rectangles, size_t count)
{
    int id, index;
    Rectangle updated;
    if (!read_positive_id(&id))
        return 0;
    index = find_rectangle(rectangles, count, id);
    if (index < 0)
    {
        printf("No rectangle found with ID %d.\n", id);
        return 1;
    }
    updated = rectangles[index];
    if (!read_dimensions(&updated))
        return 0;
    rectangles[index] = updated;
    printf("Rectangle updated successfully; area and perimeter were recomputed.\n");
    return 1;
}

static int delete_rectangle(Rectangle *rectangles, size_t *count)
{
    int id, index;
    size_t i;
    if (!read_positive_id(&id))
        return 0;
    index = find_rectangle(rectangles, *count, id);
    if (index < 0)
    {
        printf("No rectangle found with ID %d.\n", id);
        return 1;
    }
    for (i = (size_t)index; i + 1 < *count; i++)
        rectangles[i] = rectangles[i + 1];
    --(*count);
    printf("Rectangle deleted successfully.\n");
    return 1;
}

static void show_area_summary(const Rectangle *rectangles, size_t count)
{
    size_t i, largest = 0, smallest = 0;
    double total = 0.0;
    if (count == 0)
    {
        printf("No area summary is available because the collection is empty.\n");
        return;
    }
    for (i = 0; i < count; i++)
    {
        total += rectangles[i].area;
        if (rectangles[i].area > rectangles[largest].area)
            largest = i;
        if (rectangles[i].area < rectangles[smallest].area)
            smallest = i;
    }
    printf("Largest area: ");
    display_rectangle(&rectangles[largest]);
    printf("Smallest area: ");
    display_rectangle(&rectangles[smallest]);
    printf("Total area: %.2f\nAverage area: %.2f\n", total, total / count);
}

static int compare_area(const void *left, const void *right)
{
    const Rectangle *a = (const Rectangle *)left;
    const Rectangle *b = (const Rectangle *)right;
    if (a->area < b->area)
        return -1;
    if (a->area > b->area)
        return 1;
    if (a->id < b->id)
        return -1;
    if (a->id > b->id)
        return 1;
    return 0;
}

static void display_sorted(const Rectangle *rectangles, size_t count)
{
    Rectangle *copy;
    if (count == 0)
    {
        printf("The rectangle collection is empty.\n");
        return;
    }
    if (count > (size_t)-1 / sizeof(Rectangle) ||
        (copy = (Rectangle *)malloc(count * sizeof(Rectangle))) == NULL)
    {
        printf("Unable to allocate memory for the sorted view.\n");
        return;
    }
    memcpy(copy, rectangles, count * sizeof(Rectangle));
    qsort(copy, count, sizeof(Rectangle), compare_area);
    printf("Rectangles by ascending area (ties by ID):\n");
    display_rectangles(copy, count);
    free(copy);
}

static int display_area_filter(const Rectangle *rectangles, size_t count)
{
    double threshold;
    size_t i;
    int found = 0;
    if (!read_nonnegative_number("Show rectangles with area greater than: ", &threshold))
        return 0;
    for (i = 0; i < count; i++)
    {
        if (rectangles[i].area > threshold)
        {
            display_rectangle(&rectangles[i]);
            found = 1;
        }
    }
    if (!found)
        printf("No rectangles have an area greater than %.2f.\n", threshold);
    return 1;
}

int main(void)
{
    long initial;
    size_t count = 0, capacity;
    Rectangle *rectangles;
    int running = 1;
    printf("Rectangle Inventory and Geometry Analysis\n");
    for (;;)
    {
        if (!read_long("Initial number of rectangles (1-1000): ", &initial))
            return 0;
        if (initial >= 1 && initial <= MAX_RECTANGLES)
            break;
        printf("Enter a number from 1 through %d.\n", MAX_RECTANGLES);
    }
    capacity = (size_t)initial;
    rectangles = create_rectangle_array(capacity);
    if (rectangles == NULL)
    {
        printf("Unable to allocate memory for the rectangle collection.\n");
        return 1;
    }
    while (count < capacity)
    {
        printf("Enter initial rectangle %zu of %zu:\n", count + 1, capacity);
        if (!add_rectangle(&rectangles, &count, &capacity))
        {
            free(rectangles);
            return 0;
        }
    }
    while (running)
    {
        long choice;
        printf("\n1 Add  2 Display all  3 Search by ID  4 Update dimensions\n"
               "5 Delete  6 Area summary  7 Sort by area  8 Filter by area  0 Exit\n");
        if (!read_long("Choose an operation: ", &choice))
            break;
        switch (choice)
        {
        case 1:
            if (!add_rectangle(&rectangles, &count, &capacity))
                running = 0;
            break;
        case 2:
            display_rectangles(rectangles, count);
            break;
        case 3:
            if (!search_rectangle(rectangles, count))
                running = 0;
            break;
        case 4:
            if (!update_rectangle(rectangles, count))
                running = 0;
            break;
        case 5:
            if (!delete_rectangle(rectangles, &count))
                running = 0;
            break;
        case 6:
            show_area_summary(rectangles, count);
            break;
        case 7:
            display_sorted(rectangles, count);
            break;
        case 8:
            if (!display_area_filter(rectangles, count))
                running = 0;
            break;
        case 0:
            running = 0;
            break;
        default:
            printf("Unknown option. Choose a number from 0 to 8.\n");
        }
    }
    free(rectangles);
    printf("Rectangle collection memory released. Goodbye.\n");
    return 0;
}
