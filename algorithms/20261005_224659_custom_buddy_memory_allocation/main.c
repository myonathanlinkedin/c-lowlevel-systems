#include <stdlib.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    // Initialize with 1 MiB pool
    assert(buddy_init(1 << 20));

    // Basic allocations
    void* a = buddy_alloc(100);
    assert(a != NULL);
    void* b = buddy_alloc(5000);
    assert(b != NULL);
    void* c = buddy_alloc(200000);
    assert(c != NULL);

    // Free and test coalescing
    buddy_free(a);
    buddy_free(b);
    void* d = buddy_alloc(6000);
    assert(d != NULL);

    // Release remaining blocks
    buddy_free(c);
}
