#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>

static void test_triangle(void) {
    Edge edges[] = {
        {0, 1, 4},
        {1, 2, 1},
        {0, 2, 3}
    };
    Edge mst[3];
    size_t mst_cnt = 0;
    int32_t weight = kruskal_mst(3, edges, 3, mst, &mst_cnt);
    assert(weight == 4); // edges (1,2)=1 and (0,2)=3
    assert(mst_cnt == 2);
    // verify selected edges
    bool has01 = false, has12 = false, has02 = false;
    for (size_t i = 0; i < mst_cnt; ++i) {
        if ((mst[i].u == 0 && mst[i].v == 1) || (mst[i].u == 1 && mst[i].v == 0))
            has01 = true;
        if ((mst[i].u == 1 && mst[i].v == 2) || (mst[i].u == 2 && mst[i].v == 1))
            has12 = true;
        if ((mst[i].u == 0 && mst[i].v == 2) || (mst[i].u == 2 && mst[i].v == 0))
            has02 = true;
    }
    assert(!has01);
    assert(has12);
    assert(has02);
}

static void test_square_with_diagonal(void) {
    Edge edges[] = {
        {0,1,1},
        {1,2,2},
        {2,3,1},
        {3,0,2},
        {0,2,2} // diagonal
    };
    Edge mst[5];
    size_t mst_cnt = 0;
    int32_t weight = kruskal_mst(4, edges, 5, mst, &mst_cnt);
    assert(weight == 4); // edges 0-1,2-3,0-2 (or 1-2) total 1+1+2
    assert(mst_cnt == 3);
}

static void test_disconnected(void) {
    Edge edges[] = {
        {0,1,1},
        {2,3,1}
    };
    Edge mst[4];
    size_t mst_cnt = 0;
    int32_t weight = kruskal_mst(4, edges, 2, mst, &mst_cnt);
    assert(weight == INT32_MAX); // cannot form spanning tree
    assert(mst_cnt != 3);
}

int main(void) {
    test_triangle();
    test_square_with_diagonal();
    test_disconnected();
    printf("All Kruskal MST tests passed.\n");
    return 0;
}
