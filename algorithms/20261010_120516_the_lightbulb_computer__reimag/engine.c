#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "types.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Utility functions */
int min_int(int a, int b) {
    return a < b ? a : b;
}

int max_int(int a, int b) {
    return a > b ? a : b;
}

/* Grid management */
LightGrid* grid_create(int width, int height) {
    if (width <= 0 || height <= 0 || width > MAX_GRID_SIZE || height > MAX_GRID_SIZE) {
        return NULL;
    }
    
    LightGrid* grid = (LightGrid*)malloc(sizeof(LightGrid));
    if (!grid) return NULL;
    
    grid->width = width;
    grid->height = height;
    grid->bulb_count = width * height;
    grid->grid = (Lightbulb*)calloc(grid->bulb_count, sizeof(Lightbulb));
    if (!grid->grid) {
        free(grid);
        return NULL;
    }
    
    /* Initialize all bulbs */
    for (int i = 0; i < grid->bulb_count; i++) {
        grid->grid[i].x = i % width;
        grid->grid[i].y = i / width;
        grid->grid[i].intensity = LIGHT_OFF;
        grid->grid[i].active = false;
    }
    
    return grid;
}

void grid_destroy(LightGrid* grid) {
    if (grid) {
        free(grid->grid);
        free(grid);
    }
}

void grid_reset(LightGrid* grid) {
    if (!grid) return;
    for (int i = 0; i < grid->bulb_count; i++) {
        grid->grid[i].intensity = LIGHT_OFF;
        grid->grid[i].active = false;
    }
}

/* Projector management */
Projector* projector_create(int id, PatternType pattern, LightIntensity intensity) {
    Projector* proj = (Projector*)malloc(sizeof(Projector));
    if (!proj) return NULL;
    
    proj->id = id;
    proj->pattern = pattern;
    proj->intensity = intensity;
    proj->enabled = true;
    proj->x1 = proj->y1 = proj->x2 = proj->y2 = proj->x3 = proj->y3 = 0;
    
    return proj;
}

void projector_destroy(Projector* proj) {
    free(proj);
}

/* Spatial geometry functions */
bool is_point_in_rectangle(int px, int py, int x1, int y1, int x2, int y2) {
    int min_x = min_int(x1, x2);
    int max_x = max_int(x1, x2);
    int min_y = min_int(y1, y2);
    int max_y = max_int(y1, y2);
    
    return (px >= min_x && px <= max_x && py >= min_y && py <= max_y);
}

bool is_point_in_circle(int px, int py, int cx, int cy, int radius) {
    int dx = px - cx;
    int dy = py - cy;
    return (dx * dx + dy * dy) <= (radius * radius);
}

bool is_point_on_line(int px, int py, int x1, int y1, int x2, int y2) {
    /* Check if point is on the line segment using cross product */
    int cross = (px - x1) * (y2 - y1) - (py - y1) * (x2 - x1);
    if (cross != 0) return false;
    
    /* Check if point is within the bounding box of the line segment */
    int min_x = min_int(x1, x2);
    int max_x = max_int(x1, x2);
    int min_y = min_int(y1, y2);
    int max_y = max_int(y1, y2);
    
    return (px >= min_x && px <= max_x && py >= min_y && py <= max_y);
}

bool is_point_in_triangle(int px, int py, int x1, int y1, int x2, int y2, int x3, int y3) {
    /* Barycentric coordinate method */
    int area = (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);
    if (area == 0) return false;
    
    int a = ((x2 - x1) * (py - y1) - (x3 - x1) * (py - y1)) / area;
    int b = ((x3 - x1) * (px - x1) - (x2 - x1) * (px - x1)) / area;
    int c = 1 - a - b;
    
    return (a >= 0 && b >= 0 && c >= 0);
}

/* Core algorithm: Apply projector to grid */
OperationResult apply_projector(LightGrid* grid, const Projector* proj) {
    OperationResult result = {false, 0, 0};
    
    if (!grid || !proj || !proj->enabled) {
        return result;
    }
    
    int affected = 0;
    
    for (int y = 0; y < grid->height; y++) {
        for (int x = 0; x < grid->width; x++) {
            bool in_pattern = false;
            
            switch (proj->pattern) {
                case PATTERN_RECTANGLE:
                    in_pattern = is_point_in_rectangle(x, y, proj->x1, proj->y1, proj->x2, proj->y2);
                    break;
                case PATTERN_CIRCLE:
                    in_pattern = is_point_in_circle(x, y, proj->x1, proj->y1, proj->x2);
                    break;
                case PATTERN_LINE:
                    in_pattern = is_point_on_line(x, y, proj->x1, proj->y1, proj->x2, proj->y2);
                    break;
                case PATTERN_TRIANGLE:
                    in_pattern = is_point_in_triangle(x, y, proj->x1, proj->y1, 
                                                       proj->x2, proj->y2, proj->x3, proj->y3);
                    break;
                default:
                    in_pattern = false;
                    break;
            }
            
            if (in_pattern) {
                Lightbulb* bulb = &grid->grid[y * grid->width + x];
                /* Add intensity (cap at max) */
                int new_intensity = bulb->intensity + proj->intensity;
                if (new_intensity > LIGHT_MAX) {
                    new_intensity = LIGHT_MAX;
                }
                bulb->intensity = (LightIntensity)new_intensity;
                bulb->active = (new_intensity > LIGHT_OFF);
                affected++;
            }
        }
    }
    
    result.success = true;
    result.affected_bulbs = affected;
    result.total_on_bulbs = count_illuminated_bulbs(grid);
    
    return result;
}

/* Query functions */
int count_illuminated_bulbs(const LightGrid* grid) {
    if (!grid) return 0;
    
    int count = 0;
    for (int i = 0; i < grid->bulb_count; i++) {
        if (grid->grid[i].active) {
            count++;
        }
    }
    return count;
}

LightIntensity get_bulb_intensity(const LightGrid* grid, int x, int y) {
    if (!grid || x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return LIGHT_OFF;
    }
    return grid->grid[y * grid->width + x].intensity;
}

void set_bulb_intensity(LightGrid* grid, int x, int y, LightIntensity intensity) {
    if (!grid || x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return;
    }
    Lightbulb* bulb = &grid->grid[y * grid->width + x];
    bulb->intensity = intensity;
    bulb->active = (intensity > LIGHT_OFF);
}
