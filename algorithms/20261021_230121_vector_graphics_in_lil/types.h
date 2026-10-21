#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*---------------------------*
 *  Core Data Types          *
 *---------------------------*/

typedef struct {
    float x;
    float y;
} Point;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

typedef struct {
    Point a;
    Point b;
    Color color;
} Line;

typedef struct {
    Point *vertices;   /* Dynamically allocated array of vertices */
    size_t count;      /* Number of vertices (must be >= 3 for a valid polygon) */
    Color color;
} Polygon;

typedef struct {
    size_t width;
    size_t height;
    uint32_t *pixels;  /* Row-major ARGB (0xAARRGGBB) */
} Bitmap;

/*---------------------------*
 *  API Function Declarations*
 *---------------------------*/

/* Point transformations */
void point_translate(Point *p, float dx, float dy);
void point_rotate(Point *p, float angle_rad, const Point *origin);
void point_scale(Point *p, float sx, float sy, const Point *origin);

/* Bitmap management */
Bitmap *bitmap_create(size_t width, size_t height);
void bitmap_destroy(Bitmap *bm);
void bitmap_clear(Bitmap *bm, uint32_t argb);
bool bitmap_set_pixel(Bitmap *bm, int32_t x, int32_t y, uint32_t argb);
uint32_t bitmap_get_pixel(const Bitmap *bm, int32_t x, int32_t y);

/* Line rasterisation */
void line_rasterize(const Line *ln, Bitmap *bm);

/* Polygon rasterisation */
Polygon *polygon_create(const Point *verts, size_t count, Color color);
void polygon_destroy(Polygon *poly);
void polygon_rasterize(const Polygon *poly, Bitmap *bm);

/* Utility */
static inline uint32_t color_to_argb(const Color *c) {
    return ((uint32_t)c->a << 24) |
           ((uint32_t)c->r << 16) |
           ((uint32_t)c->g << 8)  |
           ((uint32_t)c->b);
}
