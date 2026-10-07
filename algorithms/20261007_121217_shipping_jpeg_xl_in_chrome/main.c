#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include <stdio.h>
#include <assert.h>
#include "types.h"

extern JXLResult parse_jxl_header(const uint8_t *data, size_t size, JXLImageInfo *out);

static void test_valid_header(void) {
    uint8_t data[] = {
        'J','X','L',' ',
        0x00,0x00,0x04,0x00,   // width = 1024
        0x00,0x00,0x03,0x00,   // height = 768
        0x01                   // flags: alpha present
    };
    JXLImageInfo info;
    JXLResult res = parse_jxl_header(data, sizeof(data), &info);
    assert(res == JXL_OK);
    assert(info.width == 1024);
    assert(info.height == 768);
    assert(info.has_alpha == true);
}

static void test_invalid_magic(void) {
    uint8_t data[] = {
        'B','A','D',' ',
        0x00,0x00,0x01,0x00,
        0x00,0x00,0x01,0x00,
        0x00
    };
    JXLImageInfo info;
    JXLResult res = parse_jxl_header(data, sizeof(data), &info);
    assert(res == JXL_ERR_UNSUPPORTED_FORMAT);
}

static void test_insufficient_size(void) {
    uint8_t data[] = {
        'J','X','L',' ',
        0x00,0x00,0x01 // truncated
    };
    JXLImageInfo info;
    JXLResult res = parse_jxl_header(data, sizeof(data), &info);
    assert(res == JXL_ERR_INSUFFICIENT_SIZE);
}

static void test_null_pointers(void) {
    uint8_t data[] = {
        'J','X','L',' ',
        0x00,0x00,0x01,0x00,
        0x00,0x00,0x01,0x00,
        0x00
    };
    JXLResult r1 = parse_jxl_header(NULL, sizeof(data), NULL);
    assert(r1 == JXL_ERR_INVALID_DATA);
    JXLResult r2 = parse_jxl_header(data, sizeof(data), NULL);
    assert(r2 == JXL_ERR_INVALID_DATA);
}

int main(void) {
    test_valid_header();
    test_invalid_magic();
    test_insufficient_size();
    test_null_pointers();
    printf("All JPEG XL header tests passed.\n");
    return 0;
}
