#include "dynamic_array.h"
#include <stdlib.h>
void arr_push(struct array *a, void *pV) {
    void **tmp;
    if(a->len == a->capacity) {
        a->capacity += DEFAULT_CAPACITY;
        tmp = realloc(a->items,a->capacity * a->item_size);
        if(tmp == NULL) {
            free(a->items);
            return;
        }
        a->items = tmp;
    }

    a->items[a->len] = pV;
    a->len++;
}

struct array *arr_init(size_t size) {
    struct array *a = malloc(sizeof(struct array));
    a->capacity = DEFAULT_CAPACITY; 
    a->len = 0;
    a->item_size = size;
    a->items = malloc(a->capacity * size);
    return a;
}

void arr_free(struct array *a) {
    for (int i = 0; i < a->len; i++) {
        free(a->items[i]);
    }
    free(a->items);
    free(a);
};

void arr_pop(struct array *a) {
};
