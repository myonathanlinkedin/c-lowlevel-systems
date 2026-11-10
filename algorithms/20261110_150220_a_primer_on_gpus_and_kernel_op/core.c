#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stddef.h>

// -----------------------------------------------------------------------------
// Internal: simulated kernel launcher
// -----------------------------------------------------------------------------
static void gpu_launch(gpu_kernel_t kernel, const void *args, const gpu_grid_t *grid)
{
    // Simple deterministic simulation: iterate over blocks then threads.
    for (size_t block = 0; block < grid->gridDimX; ++block) {
        for (size_t thread = 0; thread < grid->blockDimX; ++thread) {
            size_t global_id = block * grid->blockDimX + thread;
            // Cast away constness of args – kernels treat it as mutable.
            kernel(global_id, (void *)args);
        }
    }
}

// -----------------------------------------------------------------------------
// Kernel: vector addition
// -----------------------------------------------------------------------------
static void vec_add_kernel(size_t gid, void *arg_ptr)
{
    vec_add_args_t *args = (vec_add_args_t *)arg_ptr;
    if (gid < args->n) {
        args->c[gid] = args->a[gid] + args->b[gid];
    }
}

// -----------------------------------------------------------------------------
// Public API implementation
// -----------------------------------------------------------------------------
void gpu_vector_add(const float *a, const float *b, float *c, size_t n)
{
    // Choose a reasonable block size (power of two for alignment).
    const size_t BLOCK_SIZE = 256;

    // Compute grid dimension to cover all elements.
    gpu_grid_t grid;
    grid.blockDimX = BLOCK_SIZE;
    grid.gridDimX = (n + BLOCK_SIZE - 1) / BLOCK_SIZE;

    // Pack kernel arguments.
    vec_add_args_t args = { .a = a, .b = b, .c = c, .n = n };

    // Launch the simulated kernel.
    gpu_launch(vec_add_kernel, &args, &grid);
}
