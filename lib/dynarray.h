#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stddef.h>
#include <stdint.h>

/**
 * A generic dynamic array implementation.
 * 
 * @tparam T The type of elements stored in the array
 */
#define DYNARRAY(T) struct { \
    T *items;               \
    size_t size;           \
    size_t capacity;       \
}

/**
 * Initialize a dynamic array with a default capacity.
 * 
 * @param arr The array to initialize
 * @param initial_capacity The initial capacity of the array
 * @return 0 on success, -1 on failure
 */
#define DYNARRAY_INIT(arr, initial_capacity) ({ \
    (arr).items = malloc((initial_capacity) * sizeof(*(arr).items)); \
    (arr).size = 0; \
    (arr).capacity = (arr).items ? (initial_capacity) : 0; \
    (arr).items ? 0 : -1; \
})

/**
 * Free the memory allocated for the array.
 * 
 * @param arr The array to free
 */
#define DYNARRAY_FREE(arr) ({ \
    free((arr).items); \
    (arr).items = NULL; \
    (arr).size = 0; \
    (arr).capacity = 0; \
})

/**
 * Push an element to the end of the array.
 * Automatically resizes if necessary.
 * 
 * @param arr The array to push to
 * @param value The value to push
 * @return 0 on success, -1 on failure
 */
#define DYNARRAY_PUSH(arr, value, T) { \
    int result = 0; \
    if ((arr).size >= (arr).capacity) { \
        size_t new_capacity = (arr).capacity == 0 ? 1 : (arr).capacity * 2; \
        T *new_items = realloc((arr).items, new_capacity * sizeof(*(arr).items)); \
        if (!new_items) { \
            result = -1; \
        } else { \
            (arr).items = new_items; \
            (arr).capacity = new_capacity; \
        } \
    } \
    if (result == 0) { \
        (arr).items[(arr).size++] = (value); \
    } \
}

/**
 * Get the size of the array.
 * 
 * @param arr The array to get the size of
 * @return The size of the array
 */
#define DYNARRAY_SIZE(arr) ((arr).size)

/**
 * Get the capacity of the array.
 * 
 * @param arr The array to get the capacity of
 * @return The capacity of the array
 */
#define DYNARRAY_CAPACITY(arr) ((arr).capacity)

/**
 * Get a pointer to the element at the given index.
 * 
 * @param arr The array to get the element from
 * @param index The index of the element
 * @return A pointer to the element, or NULL if the index is out of bounds
 */
#define DYNARRAY_AT(arr, index) \
    ((index) < (arr).size ? &(arr).items[(index)] : NULL)

/**
 * Clear the array, setting its size to 0.
 * 
 * @param arr The array to clear
 */
#define DYNARRAY_CLEAR(arr) ((arr).size = 0)

#endif 
