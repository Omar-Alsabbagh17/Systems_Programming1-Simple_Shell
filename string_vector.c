#include <stdio.h>
#include <stdlib.h>

#include "string_vector.h"

void vec_init(string_vector *v)
{
    v->capacity = INIT_CAP;
    v->items = malloc(sizeof(void *) * INIT_CAP);
    v->total = 0;
}


void vec_add(string_vector *v, void *item)
{
    if (v->capacity == v->total)
        vec_resize(v, v->total * 2);
    v->items[v->total++] = item;
    
}

static void vec_resize(string_vector *v, int capacity)
{
    void **items = realloc(v->items, sizeof(void *) * capacity);
    if (items) {
        // if not empty
        v->items = items;
        v->capacity = capacity;
    }
}
