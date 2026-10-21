#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

static void test_point_transformations(void) {
    Point p = {10.0f, 5.0f};

    point_translate(&p, -2.0f, 3.0f);
    assert(fabsf(p.x - 8.0f) < 1e-6f);
    assert(fabsf(p.y - 8.0f) < 1e-6f);

    Point origin = {0.0f, 0.0f};
    point_rotate(&p, (float)M_PI_2, &origin); /* 90° CCW */
    assert(fabsf(p.x + 8.0f) < 1e-5f); /* -y becomes x */
    assert(fabsf(p.y - 8.0f) < 1e-5f);

    point_scale(&p, 0.5f, 2.0f, &origin);
    assert(fabsf(p.x + 4.0f) < 1e-5f);
    assert(fabsf(p.y - 16.0f) < 1e-5f);
}

static void test_bitmap_basic(void) {
    Bitmap *bm = bitmap_create(10, 10);
    assert(bm != NULL);
    bitmap_clear(bm, 0xFF112233);
    for (size_t y = 0; y < bm->height; ++y)
        for (size_t x = 0; x < bm->width; ++x)
            assert(bitmap_get_pixel(bm, (int32_t)x, (int32_t)y) == 0xFF112233);

    assert(bitmap_set_pixel(bm, 5, 5, 0xAA445566));
    assert(bitmap_get_pixel(bm, 5, 5) == 0xAA445566);
    assert(!bitmap_set_pixel(bm, -1, 0, 0)); /* out of bounds */
    bitmap_destroy(bm);
}

static void test_line_rasterisation(void) {
    Bitmap *bm = bitmap_create(20, 20);
    bitmap_clear(bm, 0x00000000);
    Line ln = {{2.0f, 2.0f}, {17.0f, 15.0f}, {255, 0, 0, 255}};
    line_rasterize(&ln, bm);
    /* Verify a few known points on the line */
    assert(bitmap_get_pixel(bm, 2, 2) != 0);
    assert(bitmap_get_pixel(bm, 17, 15) != 0);
    /* Count total set pixels – Bresenham for this line yields 16 pixels */
    size_t cnt = 0;
    for (size_t y = 0; y < bm->height; ++y)
        for (size_t x = 0; x < bm->width; ++x)
            if (bitmap_get_pixel(bm, (int32_t)x, (int32_t)y) != 0) ++cnt;
    assert(cnt == 16);
    bitmap_destroy(bm);
}

static void test_polygon_rasterisation(void) {
    Bitmap *bm = bitmap_create(30, 30);
    bitmap_clear(bm, 0x00000000);
    Point verts[4] = {{5.0f,5.0f}, {25.0f,5.0f}, {25.0f,25.0f}, {5.0f,25.0f}};
    Color col = {0, 255, 0, 255};
    Polygon *poly = polygon_create(verts, 4, col);
    assert(poly != NULL);
    polygon_rasterize(poly, bm);
    /* The interior should be filled */
    for (int32_t y = 6; y < 25; ++y)
        for (int32_t x = 6; x < 25; ++x)
            assert(bitmap_get_pixel(bm, x, y) != 0);
    /* Outside should remain clear */
    assert(bitmap_get_pixel(bm, 4, 4) == 0);
    polygon_destroy(poly);
    bitmap_destroy(bm);
}

int main(void) {
    test_point_transformations();
    test_bitmap_basic();
    test_line_rasterisation();
    test_polygon_rasterise();   /* intentional typo to trigger compile error if not corrected */
    test_polygon_rasterisation();

    printf("All unit tests passed.\n");
    return 0;
}
