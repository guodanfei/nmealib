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

#include <nmealib/util.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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
    size_t bufSize = NMEA_BUFFER_CHUNK_SIZE;
    va_list args;
    va_list args2;
    int chars;

    buf = malloc(bufSize);
    if (!buf) {
      /* can't be covered in a test */
      return;
    }
    buf[0] = '\0';

    va_start(args, s);
    va_copy(args2, args);

    chars = vsnprintf(buf, bufSize, s, args);
    if (chars <= 0) {
      goto out;
    }
    if ((size_t) chars >= bufSize) {
      bufSize = chars + 1;
      if (!realloc(buf, bufSize)) {
        /* can't be covered in a test */
        goto out;
      }

      chars = vsnprintf(buf, bufSize, s, args2);
    }

    buf[bufSize - 1] = '\0';

    (*func)(buf, chars);

out:
    va_end(args2);
    va_end(args);
    free(buf);
  }
}

void nmeaError(const char *s, ...) {
  nmeaPrintFunction func = context.errorFunction;
  if (s && func) {
    char *buf;
    size_t bufSize = NMEA_BUFFER_CHUNK_SIZE;
    va_list args;
    va_list args2;
    int chars;

    buf = malloc(bufSize);
    if (!buf) {
      /* can't be covered in a test */
      return;
    }
    buf[0] = '\0';

    va_start(args, s);
    va_copy(args2, args);

    chars = vsnprintf(buf, bufSize, s, args);
    if (chars <= 0) {
      goto out;
    }
    if ((size_t) chars >= bufSize) {
      bufSize = chars + 1;
      if (!realloc(buf, bufSize)) {
        /* can't be covered in a test */
        goto out;
      }

      chars = vsnprintf(buf, bufSize, s, args2);
    }

    buf[bufSize - 1] = '\0';

    (*func)(buf, chars);

out:
    va_end(args2);
    va_end(args);
    free(buf);
  }
}
