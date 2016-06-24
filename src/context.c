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

#include <nmealib/context.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/** The power-of-2 chunk size of a buffer allocation */
#define BUFFER_CHUNK_SIZE (4096UL)

/** The maximum buffer size (16MB) */
#define BUFFER_SIZE_MAX   (1UL << 24)

/**
 * The structure with the nmealib context.
 */
typedef struct _NmeaContext {
  volatile nmeaPrintFunction traceFunction;
  volatile nmeaPrintFunction errorFunction;
} NmeaContext;

/** The nmealib context */
static NmeaContext context = {
    .traceFunction = NULL,
    .errorFunction = NULL };

/**
 * Round up a value to the first larger multiple of the power-of-2 value pow2
 *
 * @param val The value to round up
 * @param pow2 The power-of-2 value to round up to
 * @return The first larger multiple of the power-of-2 value pow2, or BUFFER_SIZE_MAX
 * when the request buffer would become too large
 */
static size_t ROUND_UP_TO_POWER_OF_2(size_t val, size_t pow2) {
  if (val <= (BUFFER_SIZE_MAX - (pow2 - 1))) {
    /* no overflow */
    return ((val + (pow2 - 1)) & ~(pow2 - 1));
  }

  /* overflow */
  return (BUFFER_SIZE_MAX & ~(pow2 - 1));
}

/**
 * Enlarge a malloc-ed buffer
 *
 * @param buf The malloc-ed buffer
 * @param sz The new size
 * @return The re-alloc-ed buffer, can be NULL
 */
static void * nmeaEnlargeBuffer(void * buf, size_t sz) {
  size_t bufSize = ROUND_UP_TO_POWER_OF_2(sz, BUFFER_CHUNK_SIZE);
  buf = realloc(buf, bufSize);
  return buf;
}

nmeaPrintFunction nmeaContextSetTraceFunction(nmeaPrintFunction traceFunction) {
  nmeaPrintFunction r = context.traceFunction;
  context.traceFunction = traceFunction;
  return r;
}

nmeaPrintFunction nmeaContextSetErrorFunction(nmeaPrintFunction errorFunction) {
  nmeaPrintFunction r = context.errorFunction;
  context.errorFunction = errorFunction;
  return r;
}

void nmeaTraceBuffer(const char *s, size_t sz) {
  nmeaPrintFunction func = context.traceFunction;
  if (func && s && sz) {
    (*func)(s, sz);
  }
}

void nmeaTrace(const char *s, ...) {
  nmeaPrintFunction func = context.traceFunction;
  if (s && func) {
    char *buf;
    size_t bufSize = BUFFER_CHUNK_SIZE;
    va_list args;
    int chars;

    buf = malloc(bufSize);
    if (!buf) {
      return;
    }
    buf[0] = '\0';

    va_start(args, s);

    chars = vsnprintf(buf, bufSize, s, args);
    if (chars < 0) {
      goto out;
    }
    if ((size_t) chars >= bufSize) {
      if (!nmeaEnlargeBuffer(buf, (size_t) chars + 1)) {
        goto out;
      }

      chars = vsnprintf(buf, bufSize, s, args);
    }

    buf[bufSize - 1] = '\0';

    (*func)(buf, chars);

out:
    va_end(args);
    free(buf);
  }
}

void nmeaError(const char *s, ...) {
  nmeaPrintFunction func = context.errorFunction;
  if (s && func) {
    char *buf;
    size_t bufSize = BUFFER_CHUNK_SIZE;
    va_list args;
    int chars;

    buf = malloc(bufSize);
    if (!buf) {
      return;
    }
    buf[0] = '\0';

    va_start(args, s);

    chars = vsnprintf(buf, bufSize, s, args);
    if (chars < 0) {
      goto out;
    }
    if ((size_t) chars >= bufSize) {
      if (!nmeaEnlargeBuffer(buf, (size_t) chars + 1)) {
        goto out;
      }

      chars = vsnprintf(buf, bufSize, s, args);
    }

    buf[bufSize - 1] = '\0';

    (*func)(buf, chars);

out:
    va_end(args);
    free(buf);
  }
}
