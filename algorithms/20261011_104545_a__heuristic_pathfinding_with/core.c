#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "types.h"
#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    int g;          // cost from start
    int h;          // heuristic to goal
    int f;          // g + h
    int parent;     // index of parent node
    bool closed;    // node has been processed
} Node;

static inline int idx(int x, int y, int width) { return y * width + x; }
static inline int abs_int(int v) { return v < 0 ? -v : v; }

int a_star(const int *grid, int width, int height,
           Position start, Position goal,
           Position *path, int max_path_len, int *path_len)
{
    int size = width * height;
    Node *nodes = (Node*)calloc(size, sizeof(Node));
    bool *in_open = (bool*)calloc(size, sizeof(bool));
    int *open = (int*)malloc(size * sizeof(int));
    int open_count = 0;

    if (!nodes || !in_open || !open) return -1;

    // Initialize start node
    int start_idx = idx(start.x, start.y, width);
    nodes[start_idx].g = 0;
    nodes[start_idx].h = abs_int(goal.x - start.x) + abs_int(goal.y - start.y);
    nodes[start_idx].f = nodes[start_idx].h;
    nodes[start_idx].parent = -1;
    nodes[start_idx].closed = false;
    open[open_count++] = start_idx;
    in_open[start_idx] = true;

    int goal_idx = idx(goal.x, goal.y, width);
    int found = 0;

    while (open_count > 0) {
        // Find node with lowest f
        int best = open[0];
        for (int i = 1; i < open_count; ++i) {
            if (nodes[open[i]].f < nodes[best].f)
                best = open[i];
        }
        // Remove best from open list
        for (int i = 0; i < open_count; ++i)
            if (open[i] == best) { open[i] = open[--open_count]; break; }
        in_open[best] = false;
        nodes[best].closed = true;

        if (best == goal_idx) { found = 1; break; }

        int bx = best % width;
        int by = best / width;
        const int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
        for (int d = 0; d < 4; ++d) {
            int nx = bx + dirs[d][0];
            int ny = by + dirs[d][1];
            if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
            int nidx = idx(nx, ny, width);
            if (nodes[nidx].closed) continue;
            int step_cost = grid[nidx] > 0 ? grid[nidx] : 1;
            int tentative_g = nodes[best].g + step_cost;
            if (!in_open[nidx] || tentative_g < nodes[nidx].g) {
                nodes[nidx].g = tentative_g;
                nodes[nidx].h = abs_int(goal.x - nx) + abs_int(goal.y - ny);
                nodes[nidx].f = nodes[nidx].g + nodes[nidx].h;
                nodes[nidx].parent = best;
                if (!in_open[nidx]) {
                    open[open_count++] = nidx;
                    in_open[nidx] = true;
                }
            }
        }
    }

    if (!found) {
        free(nodes); free(in_open); free(open);
        return -1;
    }

    // Reconstruct path
    int count = 0;
    int cur = goal_idx;
    while (cur != -1 && count < max_path_len) {
        int cx = cur % width;
        int cy = cur / width;
        path[count++] = (Position){cx, cy};
        cur = nodes[cur].parent;
    }
    // Reverse path
    for (int i = 0; i < count / 2; ++i) {
        Position tmp = path[i];
        path[i] = path[count - 1 - i];
        path[count - 1 - i] = tmp;
    }
    *path_len = count;

    free(nodes); free(in_open); free(open);
    return 0;
}
