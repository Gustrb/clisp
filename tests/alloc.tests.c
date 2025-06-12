#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"

int should_be_able_to_init_a_context(void);
int should_be_able_to_alloc_and_free(void);
int should_be_able_to_realloc(void);
int should_be_able_to_alloc_a_shitload_of_memory(void);

int main(void)
{
    int err = 0;
    err = err || should_be_able_to_init_a_context();
    err = err || should_be_able_to_alloc_and_free();
    err = err || should_be_able_to_realloc();
    err = err || should_be_able_to_alloc_a_shitload_of_memory();

    if (err == 0)
    {
        fprintf(stdout, "[OK] All alloc tests passed\n");
    }
    else
    {
        fprintf(stdout, "[FAIL] Some alloc tests failed\n");
        return 1;
    }

    return 0;
}

int should_be_able_to_init_a_context(void)
{
    fprintf(stdout, "[TEST] should_be_able_to_init_a_context\n");
    if (alloc_init(NULL, 10) != ALLOC_ERR_INVALID_ARG)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_init_a_context: alloc_init(NULL, 10) != ALLOC_ERR_INVALID_ARG\n");
        return 1;
    }

    alloc_context_t ctx;
    int err = alloc_init(&ctx, 10);
    if (err)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_init_a_context: alloc_init failed: %d\n", err);
        return 1;
    }

    if (ctx.els == NULL)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_init_a_context: ctx.els is NULL\n");
        return 1;
    }

    if (ctx.elements != 0)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_init_a_context: ctx.elements is not 0\n");
        return 1;
    }

    if (ctx.cap != 10)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_init_a_context: ctx.cap is not 10\n");
        return 1;
    }

    fprintf(stdout, "[OK] should_be_able_to_init_a_context\n");
    return 0;
}

int should_be_able_to_alloc_and_free(void)
{
    fprintf(stdout, "[TEST] should_be_able_to_alloc_and_free\n");

    void *ptr = alloc_alloc(NULL, sizeof(int));
    if (ptr)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_and_free: alloc_alloc(NULL, sizeof(int)) != NULL\n");
        return 1;
    }

    size_t size = 10;

    alloc_context_t ctx;
    int err = alloc_init(&ctx, size);
    if (err)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_and_free: alloc_init failed: %d\n", err);
        return 1;
    }

    ptr = alloc_alloc(&ctx, sizeof(int));
    if (!ptr)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_and_free: alloc_alloc failed\n");
        return 1;
    }

    if (ctx.elements != 1)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_and_free: ctx.elements is not 1\n");
        return 1;
    }

    if (ctx.els[0].ptr != ptr)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_and_free: ctx.els[0].ptr is not %p\n", ptr);
        return 1;
    }

    if (ctx.els[0].is_free != 0)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_and_free: ctx.els[0].is_free is not 0\n");
        return 1;
    }

    if (alloc_were_all_allocations_freed(&ctx) != 0)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_and_free: allocation was not added to the context\n");
        return 1;
    }

    alloc_free(&ctx, ptr);

    if (alloc_were_all_allocations_freed(&ctx) != 1)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_and_free: allocation was not freed\n");
        return 1;
    }

    err = alloc_free_context(&ctx);
    if (err)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_and_free: alloc_free_context failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[OK] should_be_able_to_alloc_and_free\n");
    return 0;
}

int should_be_able_to_realloc(void)
{
    fprintf(stdout, "[TEST] should_be_able_to_realloc\n");

    if (alloc_realloc(NULL, NULL, 10) != NULL)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_realloc: alloc_realloc(NULL, NULL, 10) != NULL\n");
        return 1;
    }

    alloc_context_t ctx;
    int err = alloc_init(&ctx, 10);
    if (err)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_realloc: alloc_init failed: %d\n", err);
        return 1;
    }

    void *ptr = alloc_alloc(&ctx, sizeof(int));
    if (!ptr)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_realloc: alloc_alloc failed\n");
        return 1;
    }

    ptr = alloc_realloc(&ctx, ptr, sizeof(int) * 2);
    if (!ptr)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_realloc: alloc_realloc failed\n");
        return 1;
    }

    if (ctx.els[0].ptr != ptr)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_realloc: ctx.els[0].ptr is not %p\n", ptr);
        return 1;
    }

    if (ctx.els[0].is_free != 0)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_realloc: ctx.els[0].is_free is not 0\n");
        return 1;
    }

    alloc_free(&ctx, ptr);

    if (alloc_were_all_allocations_freed(&ctx) != 1)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_realloc: allocation was not freed\n");
        return 1;
    }

    err = alloc_free_context(&ctx);
    if (err)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_realloc: alloc_free_context failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[OK] should_be_able_to_realloc\n");

    return 0;
}

int should_be_able_to_alloc_a_shitload_of_memory(void)
{
    fprintf(stdout, "[TEST] should_be_able_to_alloc_a_shitload_of_memory\n");

    alloc_context_t ctx;
    int err = alloc_init(&ctx, 10);
    if (err)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_a_shitload_of_memory: alloc_init failed: %d\n", err);
        return 1;
    }

    void *ptrs[100000];
    size_t size = 100000;

    for (size_t i = 0; i < size; ++i)
    {
        ptrs[i] = alloc_alloc(&ctx, sizeof(int));
        if (!ptrs[i])
        {
            fprintf(stderr, "[FAIL] should_be_able_to_alloc_a_shitload_of_memory: alloc_alloc failed\n");
            return 1;
        }
    }

    if (ctx.elements != size)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_a_shitload_of_memory: ctx.elements is not %zu\n", size);
        return 1;
    }

    for (size_t i = 0; i < size; ++i)
    {
        alloc_free(&ctx, ptrs[i]);
    }

    if (alloc_were_all_allocations_freed(&ctx) != 1)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_a_shitload_of_memory: allocation was not freed\n");
        return 1;
    }

    err = alloc_free_context(&ctx);
    if (err)
    {
        fprintf(stderr, "[FAIL] should_be_able_to_alloc_a_shitload_of_memory: alloc_free_context failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[OK] should_be_able_to_alloc_a_shitload_of_memory\n");
    return 0;
}
