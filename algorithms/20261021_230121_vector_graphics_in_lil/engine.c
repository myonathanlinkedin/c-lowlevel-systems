#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/*---------------------------*
 *  Point Transformations    *
 *---------------------------*/

void point_translate(Point *p, float dx, float dy) {
    assert(p != NULL);
    p->x += dx;
    p->y += dy;
}

void point_rotate(Point *p, float angle_rad, const Point *origin) {
    assert(p != NULL && origin != NULL);
    float s = sinf(angle_rad);
    float c = cosf(angle_rad);
    float tx = p->x - origin->x;
    float ty = p->y - origin->y;
    float rx = tx * c - ty * s;
    float ry = tx * s + ty * c;
    p->x = rx + origin->x;
    p->y = ry + origin->y;
}

void point_scale(Point *p, float sx, float sy, const Point *origin) {
    assert(p != NULL && origin != NULL);
    p->x = origin->x + (p->x - origin->x) * sx;
    p->y = origin->y + (p->y - origin->y) * sy;
}

/*---------------------------*
 *  Bitmap Management        *
 *---------------------------*/

Bitmap *bitmap_create(size_t width, size_t height) {
    Bitmap *bm = (Bitmap *)malloc(sizeof(Bitmap));
    if (!bm) return NULL;
    bm->width = width;
    bm->height = height;
    bm->pixels = (uint32_t *)calloc(width * height, sizeof(uint32_t));
    if (!bm->pixels) {
        free(bm);
        return NULL;
    }
    return bm;
}

void bitmap_destroy(Bitmap *bm) {
    if (bm) {
        free(bm->pixels);
        free(bm);
    }
}

void bitmap_clear(Bitmap *bm, uint32_t argb) {
    assert(bm != NULL && bm->pixels != NULL);
    size_t total = bm->width * bm->height;
    for (size_t i = 0; i < total; ++i) {
        bm->pixels[i] = argb;
    }
}

bool bitmap_set_pixel(Bitmap *bm, int32_t x, int32_t y, uint32_t argb) {
    if (!bm || !bm->pixels) return false;
    if ((uint32_t)x >= bm->width || (uint32_t)y >= bm->height) return false;
    bm->pixels[y * bm->width + x] = argb;
    return true;
}

uint32_t bitmap_get_pixel(const Bitmap *bm, int32_t x, int32_t y) {
    if (!bm || !bm->pixels) return 0;
    if ((uint32_t)x >= bm->width || (uint32_t)y >= bm->height) return 0;
    return bm->pixels[y * bm->width + x];
}

/*---------------------------*
 *  Line Rasterisation       *
 *---------------------------*/

static void bresenham_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                           Bitmap *bm, uint32_t argb) {
    int32_t dx = abs(x1 - x0);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t dy = -abs(y1 - y0);
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;  /* error value e_xy */

    while (true) {
        bitmap_set_pixel(bm, x0, y0, argb);
        if (x0 == x1 && y0 == y1) break;
        int32_t e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void line_rasterize(const Line *ln, Bitmap *bm) {
    assert(ln != NULL && bm != NULL);
    int32_t x0 = (int32_t)lroundf(ln->a.x);
    int32_t y0 = (int32_t)lroundf(ln->a.y);
    int32_t x1 = (int32_t)lroundf(ln->b.x);
    int32_t y1 = (int32_t)lroundf(ln->b.y);
    uint32_t argb = color_to_argb(&ln->color);
    bresenham_line(x0, y0, x1, y1, bm, argb);
}

/*---------------------------*
 *  Polygon Management       *
 *---------------------------*/

Polygon *polygon_create(const Point *verts, size_t count, Color color) {
    if (verts == NULL || count < 3) return NULL;
    Polygon *poly = (Polygon *)malloc(sizeof(Polygon));
    if (!poly) return NULL;
    poly->vertices = (Point *)malloc(count * sizeof(Point));
    if (!poly->vertices) {
        free(poly);
        return NULL;
    }
    memcpy(poly->vertices, verts, count * sizeof(Point));
    poly->count = count;
    poly->color = color;
    return poly;
}

void polygon_destroy(Polygon *poly) {
    if (poly) {
        free(poly->vertices);
        free(poly);
    }
}

/* Helper: compute intersection of a scanline y with edge (p0,p1) */
static bool edge_intersect_y(const Point *p0, const Point *p1, int32_t y, float *x_out) {
    if ((int32_t)floorf(p0->y) == (int32_t)floorf(p1->y)) return false; /* horizontal edge */
    if ((y < (int32_t)floorf(p0->y) && y < (int32_t)floorf(p1->y)) ||
        (y > (int32_t)floorf(p0->y) && y > (int32_t)floorf(p1->y))) {
        return false;
    }
    /* Linear interpolation */
    float t = (y + 0.5f - p0->y) / (p1->y - p0->y);
    *x_out = p0->x + t * (p1->x - p0->x);
    return true;
}

/* Simple scanline fill (non‑antialiased) */
void polygon_rasterize(const Polygon *poly, Bitmap *bm) {
    assert(poly != NULL && bm != NULL);
    if (poly->count < 3) return;

    /* Compute integer bounding box */
    float min_y_f = poly->vertices[0].y, max_y_f = poly->vertices[0].y;
    for (size_t i = 1; i < poly->count; ++i) {
        if (poly->vertices[i].y < min_y_f) min_y_f = poly->vertices[i].y;
        if (poly->vertices[i].y > max_y_f) max_y_f = poly->vertices[i].y;
    }
    int32_t min_y = (int32_t)floorf(min_y_f);
    int32_t max_y = (int32_t)ceilf(max_y_f);

    uint32_t argb = color_to_argb(&poly->color);

    /* For each scanline */
    for (int32_t y = min_y; y <= max_y; ++y) {
        /* Collect all intersections */
        float *intersections = (float *)malloc(poly->count * sizeof(float));
        size_t n = 0;
        for (size_t i = 0; i < poly->count; ++i) {
            const Point *p0 = &poly->vertices[i];
            const Point *p1 = &poly->vertices[(i + 1) % poly->count];
            float x_int;
            if (edge_intersect_y(p0, p1, y, &x_int)) {
                intersections[n++] = x_int;
            }
        }
        if (n < 2) {
            free(intersections);
            continue;
        }
        /* Sort intersections (simple insertion sort – n is tiny) */
        for (size_t i = 1; i < n; ++i) {
            float key = intersections[i];
            size_t j = i;
            while (j > 0 && intersections[j - 1] > key) {
                intersections[j] = intersections[j - 1];
                --j;
            }
            intersections[j] = key;
        }
        /* Fill between pairs */
        for (size_t i = 0; i + 1 < n; i += 2) {
            int32_t x_start = (int32_t)ceilf(intersections[i]);
            int32_t x_end   = (int32_t)floorf(intersections[i + 1]);
            for (int32_t x = x_start; x <= x_end; ++x) {
                bitmap_set_pixel(bm, x, y, argb);
            }
        }
        free(intersections);
    }
}
