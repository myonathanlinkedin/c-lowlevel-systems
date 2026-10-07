#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "types.h"

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           ((uint32_t)p[3]);
}

/* Simple JPEG XL header parser.
   Expected format (for this reference implementation):
   - 4 bytes magic: 'J','X','L',' '
   - 4 bytes width  (big‑endian uint32)
   - 4 bytes height (big‑endian uint32)
   - 1 byte flags: bit0 == 1 indicates an alpha channel
*/
JXLResult parse_jxl_header(const uint8_t *data, size_t size, JXLImageInfo *out) {
    if (data == NULL || out == NULL) {
        return JXL_ERR_INVALID_DATA;
    }

    const size_t needed = 4 + 4 + 4 + 1;  // magic + width + height + flags
    if (size < needed) {
        return JXL_ERR_INSUFFICIENT_SIZE;
    }

    if (data[0] != 'J' || data[1] != 'X' || data[2] != 'L' || data[3] != ' ') {
        return JXL_ERR_UNSUPPORTED_FORMAT;
    }

    out->width = read_be32(data + 4);
    out->height = read_be32(data + 8);
    uint8_t flags = data[12];
    out->has_alpha = (flags & 0x01) != 0;

    return JXL_OK;
}
