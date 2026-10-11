#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <assert.h>

static void test_simple()
{
    int grid[25] = {0};
    Position path[25];
    int plen;
    int res = a_star(grid, 5, 5, (Position){0,0}, (Position){4,4}, path, 25, &plen);
    assert(res == 0);
    assert(plen == 9); // Manhattan distance 8 + start
    assert(path[0].x == 0 && path[0].y == 0);
    assert(path[plen-1].x == 4 && path[plen-1].y == 4);
}

static void test_obstacle()
{
    int grid[25] = {0};
    for (int i = 0; i < 5; ++i) grid[5 + i] = 10; // block row y=1
    Position path[25];
    int plen;
    int res = a_star(grid, 5, 5, (Position){0,0}, (Position){4,4}, path, 25, &plen);
    assert(res == 0);
    assert(plen > 9); // detour needed
    assert(path[0].x == 0 && path[0].y == 0);
    assert(path[plen-1].x == 4 && path[plen-1].y == 4);
}

static void test_dynamic_cost()
{
    int grid[25] = {0};
    grid[12] = 100; // high cost at center
    Position path[25];
    int plen;
    int res = a_star(grid, 5, 5, (Position){0,0}, (Position){4,4}, path, 25, &plen);
    assert(res == 0);
    // Path should avoid center if possible
    bool center_used = false;
    for (int i = 0; i < plen; ++i)
        if (path[i].x == 2 && path[i].y == 2) center_used = true;
    assert(!center_used);
}

static void test_no_path()
{
    int grid[25] = {0};
    for (int i = 0; i < 5; ++i) grid[5 + i] = 10; // block row y=1
    grid[6] = 10; // block start's right neighbor
    Position path[25];
    int plen;
    int res = a_star(grid, 5, 5, (Position){0,0}, (Position){4,4}, path, 25, &plen);
    assert(res == -1);
}

int main(void)
{
    test_simple();
    test_obstacle();
    test_dynamic_cost();
    test_no_path();
    printf("All tests passed.\n");
    return 0;
}
