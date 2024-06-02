#ifndef DYNAMIC_ARR_C
#define DYNAMIC_ARR_C

#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 100

struct array {
    void **items;
    size_t len;
    size_t capacity;
    size_t item_size;
};

void arr_push(struct array *a, void *pV);
struct array *arr_init(size_t item_size);
void arr_free(struct array *a);
void arr_pop(struct array *a);
struct array *arr_diff(struct array *src, struct array *dest);
#endif // DYNAMIC_ARR_C
