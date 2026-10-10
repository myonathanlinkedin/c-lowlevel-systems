#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * Lightbulb Computer: Spatial & Ambient Computing with Projectors
 * 
 * This module models a spatial computing environment where projectors
 * cast light patterns onto a 2D grid. The core algorithm involves:
 * 1. Managing a grid of "lightbulbs" (pixels) that can be on/off
 * 2. Projectors that cast light in specific patterns (rectangles, circles, lines)
 * 3. Computing the final illumination state after multiple projector operations
 * 4. Optimizing with spatial indexing for large grids
 *
 * Time Complexity: O(N * P) where N is grid size and P is number of projectors
 * Space Complexity: O(N^2) for the grid
 */

/* Grid dimensions */
#define MAX_GRID_SIZE 1024
#define MAX_PROJECTORS 1000

/* Light intensity levels */
typedef enum {
    LIGHT_OFF = 0,
    LIGHT_LOW = 1,
    LIGHT_MEDIUM = 2,
    LIGHT_HIGH = 3,
    LIGHT_MAX = 4
} LightIntensity;

/* Projector pattern types */
typedef enum {
    PATTERN_RECTANGLE = 0,
    PATTERN_CIRCLE = 1,
    PATTERN_LINE = 2,
    PATTERN_TRIANGLE = 3
} PatternType;

/* A single lightbulb/pixel in the grid */
typedef struct {
    int x;
    int y;
    LightIntensity intensity;
    bool active;
} Lightbulb;

/* A projector that casts light patterns */
typedef struct {
    int id;
    PatternType pattern;
    
    /* For rectangle: x, y is top-left, width, height */
    /* For circle: x, y is center, radius */
    /* For line: x1, y1 to x2, y2 */
    /* For triangle: x1,y1 x2,y2 x3,y3 */
    int x1, y1;
    int x2, y2;
    int x3, y3;
    
    LightIntensity intensity;
    bool enabled;
} Projector;

/* The spatial grid of lightbulbs */
typedef struct {
    int width;
    int height;
    Lightbulb* grid;
    int bulb_count;
} LightGrid;

/* Operation result for testing */
typedef struct {
    bool success;
    int affected_bulbs;
    int total_on_bulbs;
} OperationResult;

/* Function prototypes */
LightGrid* grid_create(int width, int height);
void grid_destroy(LightGrid* grid);
void grid_reset(LightGrid* grid);

Projector* projector_create(int id, PatternType pattern, LightIntensity intensity);
void projector_destroy(Projector* proj);

OperationResult apply_projector(LightGrid* grid, const Projector* proj);
int count_illuminated_bulbs(const LightGrid* grid);
LightIntensity get_bulb_intensity(const LightGrid* grid, int x, int y);
void set_bulb_intensity(LightGrid* grid, int x, int y, LightIntensity intensity);

/* Spatial query functions */
bool is_point_in_rectangle(int px, int py, int x1, int y1, int x2, int y2);
bool is_point_in_circle(int px, int py, int cx, int cy, int radius);
bool is_point_on_line(int px, int py, int x1, int y1, int x2, int y2);
bool is_point_in_triangle(int px, int py, int x1, int y1, int x2, int y2, int x3, int y3);

/* Utility functions */
int min_int(int a, int b);
int max_int(int a, int b);
