#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    void *ptr;
    uint8_t is_free;
} el_t;

typedef struct
{
    el_t *els;
    size_t elements;
    size_t cap;
} alloc_context_t;

#define ALLOC_ERR_INVALID_ARG -1
#define ALLOC_ERR_MALLOC_FAILED -2

int alloc_init(alloc_context_t *ctx, size_t size);
void *alloc_alloc(alloc_context_t *ctx, size_t size);
void *alloc_realloc(alloc_context_t *ctx, void *ptr, size_t size);
void alloc_free(alloc_context_t *ctx, void *ptr);
int alloc_free_context(alloc_context_t *ctx);
int alloc_were_all_allocations_freed(alloc_context_t *ctx);

#endif
