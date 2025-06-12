#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    size_t size = 10;

    if (alloc_init(NULL, size) != ALLOC_ERR_INVALID_ARG)
    {
        fprintf(stderr, "alloc_alloc(NULL, 10) != ALLOC_ERR_INVALID_ARG\n");
        return 1;
    }

    alloc_context_t ctx;
    int err = alloc_init(&ctx, size);
    if (err)
    {
        fprintf(stderr, "alloc_init failed\n");
        return 1;
    }

    if (ctx.els == NULL)
    {
        fprintf(stderr, "ctx.els is NULL\n");
        return 1;
    }

    if (ctx.elements != 0)
    {
        fprintf(stderr, "ctx.elements is not 0\n");
        return 1;
    }

    if (ctx.cap != size)
    {
        fprintf(stderr, "ctx.cap is not %zu\n", size);
        return 1;
    }

    void *ptr = alloc_alloc(NULL, sizeof(int));
    if (ptr)
    {
        fprintf(stderr, "alloc_alloc(NULL, sizeof(int)) != NULL\n");
        return 1;
    }

    ptr = alloc_alloc(&ctx, sizeof(int));
    if (!ptr)
    {
        fprintf(stderr, "alloc_alloc failed\n");
        return 1;
    }

    if (ctx.elements != 1)
    {
        fprintf(stderr, "ctx.elements is not 1\n");
        return 1;
    }

    if (ctx.els[0].ptr != ptr)
    {
        fprintf(stderr, "ctx.els[0].ptr is not %p\n", ptr);
        return 1;
    }

    if (ctx.els[0].is_free != 0)
    {
        fprintf(stderr, "ctx.els[0].is_free is not 0\n");
        return 1;
    }

    if (alloc_were_all_allocations_freed(&ctx) != 0)
    {
        fprintf(stderr, "allocation was not added to the context\n");
        return 1;
    }

    alloc_free(&ctx, ptr);

    if (alloc_were_all_allocations_freed(&ctx) != 1)
    {
        fprintf(stderr, "allocation was not freed\n");
        return 1;
    }

    err = alloc_free_context(&ctx);
    if (err)
    {
        fprintf(stderr, "alloc_free_context failed\n");
        return 1;
    }

    return 0;
}
