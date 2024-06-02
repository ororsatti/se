#include "dynamic_array.h"
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

struct array *arr_diff(struct array *src, struct array *dest) {
    struct array *diffs = arr_init(sizeof(char*));
    size_t src_len = src->len, 
        dest_len = dest->len;
    for (int i = 0; i < src_len; i++) {
        char *src_item = src->items[i];
        int str_eqls = 1;
        for (int j = 0; j < dest_len; j++) {
            char *dest_item = dest->items[i];
            if(str_eqls == 0) {
                break;
            }
            str_eqls = strcmp(src_item, dest_item);
        }
        
        if(str_eqls != 0) {
            char *cp = malloc(strlen(dest->items[i]));
            strcpy(cp, dest->items[i]);
            arr_push(diffs, cp);
        }
    }
    return diffs;
}
