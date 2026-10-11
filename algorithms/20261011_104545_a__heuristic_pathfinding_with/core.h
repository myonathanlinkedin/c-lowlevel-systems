#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include "types.h"

int a_star(const int *grid, int width, int height,
           Position start, Position goal,
           Position *path, int max_path_len, int *path_len);
