#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* Test helper: Print grid state */
static void print_grid(const LightGrid* grid) {
    printf("Grid (%dx%d):\n", grid->width, grid->height);
    for (int y = 0; y < grid->height; y++) {
        for (int x = 0; x < grid->width; x++) {
            LightIntensity intensity = get_bulb_intensity(grid, x, y);
            char c = ' ';
            if (intensity == LIGHT_LOW) c = '.';
            else if (intensity == LIGHT_MEDIUM) c = '+';
            else if (intensity == LIGHT_HIGH) c = '#';
            else if (intensity == LIGHT_MAX) c = '@';
            printf("%c", c);
        }
        printf("\n");
    }
    printf("\n");
}

/* Test 1: Grid creation and initialization */
static void test_grid_creation(void) {
    printf("Test 1: Grid creation and initialization\n");
    
    LightGrid* grid = grid_create(10, 10);
    assert(grid != NULL);
    assert(grid->width == 10);
    assert(grid->height == 10);
    assert(grid->bulb_count == 100);
    
    /* All bulbs should be off initially */
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            assert(get_bulb_intensity(grid, x, y) == LIGHT_OFF);
        }
    }
    
    grid_destroy(grid);
    printf("  PASSED\n\n");
}

/* Test 2: Invalid grid creation */
static void test_invalid_grid(void) {
    printf("Test 2: Invalid grid creation\n");
    
    assert(grid_create(0, 10) == NULL);
    assert(grid_create(10, 0) == NULL);
    assert(grid_create(-1, 10) == NULL);
    assert(grid_create(10, -1) == NULL);
    assert(grid_create(MAX_GRID_SIZE + 1, 10) == NULL);
    
    printf("  PASSED\n\n");
}

/* Test 3: Rectangle pattern */
static void test_rectangle_pattern(void) {
    printf("Test 3: Rectangle pattern\n");
    
    LightGrid* grid = grid_create(10, 10);
    assert(grid != NULL);
    
    Projector* proj = projector_create(1, PATTERN_RECTANGLE, LIGHT_MEDIUM);
    assert(proj != NULL);
    proj->x1 = 2;
    proj->y1 = 2;
    proj->x2 = 5;
    proj->y2 = 5;
    
    OperationResult result = apply_projector(grid, proj);
    assert(result.success == true);
    assert(result.affected_bulbs == 16); /* 4x4 rectangle */
    assert(result.total_on_bulbs == 16);
    
    /* Check corners */
    assert(get_bulb_intensity(grid, 2, 2) == LIGHT_MEDIUM);
    assert(get_bulb_intensity(grid, 5, 5) == LIGHT_MEDIUM);
    assert(get_bulb_intensity(grid, 3, 3) == LIGHT_MEDIUM);
    
    /* Check outside */
    assert(get_bulb_intensity(grid, 0, 0) == LIGHT_OFF);
    assert(get_bulb_intensity(grid, 6, 6) == LIGHT_OFF);
    
    projector_destroy(proj);
    grid_destroy(grid);
    printf("  PASSED\n\n");
}

/* Test 4: Circle pattern */
static void test_circle_pattern(void) {
    printf("Test 4: Circle pattern\n");
    
    LightGrid* grid = grid_create(11, 11);
    assert(grid != NULL);
    
    Projector* proj = projector_create(2, PATTERN_CIRCLE, LIGHT_HIGH);
    assert(proj != NULL);
    proj->x1 = 5;  /* center x */
    proj->y1 = 5;  /* center y */
    proj->x2 = 3;  /* radius */
    
    OperationResult result = apply_projector(grid, proj);
    assert(result.success == true);
    
    /* Center should be lit */
    assert(get_bulb_intensity(grid, 5, 5) == LIGHT_HIGH);
    
    /* Points within radius should be lit */
    assert(get_bulb_intensity(grid, 5, 2) == LIGHT_HIGH);  /* distance 3 */
    assert(get_bulb_intensity(grid, 8, 5) == LIGHT_HIGH);  /* distance 3 */
    assert(get_bulb_intensity(grid, 5, 8) == LIGHT_HIGH);  /* distance 3 */
    assert(get_bulb_intensity(grid, 2, 5) == LIGHT_HIGH);  /* distance 3 */
    
    /* Points outside radius should be off */
    assert(get_bulb_intensity(grid, 0, 0) == LIGHT_OFF);
    assert(get_bulb_intensity(grid, 10, 10) == LIGHT_OFF);
    
    projector_destroy(proj);
    grid_destroy(grid);
    printf("  PASSED\n\n");
}

/* Test 5: Line pattern */
static void test_line_pattern(void) {
    printf("Test 5: Line pattern\n");
    
    LightGrid* grid = grid_create(10, 10);
    assert(grid != NULL);
    
    Projector* proj = projector_create(3, PATTERN_LINE, LIGHT_LOW);
}
