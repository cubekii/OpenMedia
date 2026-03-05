#pragma once

#include <stdint.h>

#define OM_MAGIC(ch4)                     \
  ((((uint32_t) (ch4) & 0xFF) << 24) |    \
   (((uint32_t) (ch4) & 0xFF00) << 8) |   \
   (((uint32_t) (ch4) & 0xFF0000) >> 8) | \
   (((uint32_t) (ch4) & 0xFF000000) >> 24))

#define OM_MAGIC_RAW(c1, c2, c3, c4) \
  (((uint32_t) (c1) << 24) |         \
   ((uint32_t) (c2) << 16) |         \
   ((uint32_t) (c3) << 8) |          \
   ((uint32_t) (c4)))
