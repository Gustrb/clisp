#include <stdlib.h>

#include "alloc.h"

int alloc_init(alloc_context_t *ctx, size_t size)
{
#ifdef ALLOC_TESTS
    if (!ctx)
        return ALLOC_ERR_INVALID_ARG;

    ctx->elements = 0;
    ctx->cap = size;
    ctx->els = (el_t *)calloc(size, sizeof(el_t));
    if (!ctx->els)
        return ALLOC_ERR_MALLOC_FAILED;
#endif

    return 0;
}

void *alloc_alloc(alloc_context_t *ctx, size_t size)
{
#ifdef ALLOC_TESTS
    if (!ctx)
        return NULL;

    if (ctx->elements + 1 > ctx->cap)
    {
        size_t new_cap = ctx->cap * 2;
        el_t *new_els = (el_t *)realloc(ctx->els, new_cap * sizeof(el_t));
        if (!new_els)
            return NULL;

        ctx->els = new_els;
        ctx->cap = new_cap;
    }

    void *ptr = malloc(size);
    if (!ptr)
        return NULL;

    ctx->els[ctx->elements].is_free = 0;
    ctx->els[ctx->elements].ptr = ptr;

    ctx->elements++;
    return ptr;
#else
    return malloc(size);
#endif
}

void alloc_free(alloc_context_t *ctx, void *ptr)
{
#ifdef ALLOC_TESTS
    if (!ctx)
        return;

    for (size_t i = 0; i < ctx->elements; i++)
    {
        if (ctx->els[i].ptr == ptr)
        {
            free(ptr);
            ctx->els[i].is_free = 1;
            ctx->els[i].ptr = NULL;
            ctx->elements--;
            return;
        }
    }
#else
    free(ptr);
#endif
}

int alloc_free_context(alloc_context_t *ctx)
{
#ifdef ALLOC_TESTS
    if (!ctx)
        return ALLOC_ERR_INVALID_ARG;

    for (size_t i = 0; i < ctx->elements; i++)
    {
        free(ctx->els[i].ptr);
    }

    free(ctx->els);
    ctx->els = NULL;
    ctx->elements = 0;
    ctx->cap = 0;
#endif
    return 0;
}

int alloc_were_all_allocations_freed(alloc_context_t *ctx)
{
#ifdef ALLOC_TESTS
    if (!ctx)
        return ALLOC_ERR_INVALID_ARG;

    for (size_t i = 0; i < ctx->elements; i++)
    {
        if (ctx->els[i].is_free == 0)
            return 0;
    }
#endif

    return 1;
}

void *alloc_realloc(alloc_context_t *ctx, void *ptr, size_t size)
{
#ifdef ALLOC_TESTS
    if (!ctx)
        return NULL;

    for (size_t i = 0; i < ctx->elements; ++i)
    {
        if (ctx->els[i].ptr == ptr)
        {
            void *new_ptr = realloc(ptr, size);
            if (!new_ptr)
                return NULL;

            ctx->els[i].ptr = new_ptr;
            return new_ptr;
        }
    }
#else
    return realloc(ptr, size);
#endif

    return NULL;
}
