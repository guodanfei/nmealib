/*
 * This file is part of nmealib.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __NMEALIB_UTIL_H__
#define __NMEALIB_UTIL_H__

#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef INLINE
#define INLINE inline __attribute__((always_inline))
#endif

#ifndef MAX
#define MAX(x,y) (((x) >= (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) (((x) <= (y)) ? (x) : (y))
#endif

/** The power-of-2 chunk size of a buffer allocation */
#define NMEA_BUFFER_CHUNK_SIZE (4096UL)

/** The maximum buffer size (16MB) */
#define NMEA_BUFFER_SIZE_MAX   (1UL << 24)

/**
 * Round up a value to the first larger multiple of the power-of-2 value pow2
 *
 * @param maxSize The maximum size (must be a power of 2)
 * @param val The value to round up
 * @param pow2 The power-of-2 value to round up to
 * @return The first larger multiple of the power-of-2 value pow2, or maxSize
 * when the request buffer would become too large
 */
static INLINE size_t nmeaUtilRoundUpToPowerOfTwo(size_t maxSize, size_t val, size_t pow2) {
  if (!maxSize //
      || !val //
      || !pow2 //
      || (maxSize < (pow2 - 1)) //
      ) {
    /* underflow */
    return pow2;
  }

  if (val <= (maxSize - (pow2 - 1))) {
    /* no overflow */
    return ((val + (pow2 - 1)) & ~(pow2 - 1));
  }

  /* overflow */
  return (maxSize & ~(pow2 - 1));
}

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_UTIL_H__ */
