#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include "stdlib.h"
#include "string.h"
#include "assert.h"

typedef struct Vector
{
    size_t element_size;
    size_t count;
    /// It is guaranteed that if capacity==0 then data==0
    size_t capacity;
    void *data;
} Vector;

static inline void vector_init(Vector *vec, size_t element_size, size_t capacity)
{
    vec->element_size = element_size;
    vec->capacity = capacity;
    vec->count = 0;
    if (capacity)
        vec->data = calloc(capacity, element_size);
    else 
        vec->data = 0;
    assert(vec->data || !(capacity * element_size));
}

static inline void vector_deinit(Vector *vec)
{
    free(vec->data);
    vec->data = 0;
    vec->capacity = 0;
    vec->count = 0;
    vec->element_size = 0;
}

static inline void vector_set_capacity(Vector *vec, size_t capacity)
{
    if (vec->data)
        vec->data = realloc(vec->data, vec->element_size * capacity);
    else
        vec->data = calloc(capacity, vec->element_size);
    assert(vec->data || !(capacity * vec->element_size));
    vec->capacity = capacity;

    if (!capacity)
    {
        // in case that malloc(0) returns a non-deref pointer: it might have to be free'd.
        free(vec->data);
        vec->data = 0;
    }
}

static inline void vector_set_count(Vector *vec, size_t count)
{
    if (count > vec->capacity)
        vector_set_capacity(vec, 2 * count);
    vec->count = count;
}

static inline void *vector_at(Vector *vec, int i)
{
    return (char *)vec->data + i * vec->element_size;
}

static inline void vector_set(Vector *vec, int i, void *element)
{
    memmove(vector_at(vec, i), element, vec->element_size);
}

static inline void vector_push(Vector *vec, void *element)
{
    vector_set_count(vec, vec->count + 1);
    vector_set(vec, vec->count - 1, element);
}

static inline void vector_swap_remove(Vector *vec, int i)
{
    vector_set(vec, i, vector_at(vec, vec->count - 1));
    vector_set_count(vec, vec->count - 1);
}

static inline int vector_find(Vector *vec, void *needle)
{
    for (int i = 0; i < (int)vec->count; ++i)
        if (!memcmp(vector_at(vec, i), needle, vec->element_size))
            return i;
    return -1;
}

static inline void *vector_end(Vector *vec)
{
    return vector_at(vec, vec->count);
}

#endif
