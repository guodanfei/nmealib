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
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/**
 * The structure with the nmealib context.
 */
typedef struct _nmeaPROPERTY {
  volatile nmeaTraceFunc traceCallback; /**< The trace callback, defaults to NULL (disabled)               */
  volatile nmeaErrorFunc errorCallback; /**< The error logging callback, defaults to NULL (disabled)       */
  volatile size_t parseBufferSize;      /**< The size to use for temporary trace and error logging buffers */
} nmeaPROPERTY;

/** The nmealib context */
static nmeaPROPERTY property = {
    .traceCallback = NULL,
    .errorCallback = NULL,
    .parseBufferSize = NMEA_TRACE_ERROR_BUFF_DEF
};

void nmea_context_set_trace_func(nmeaTraceFunc func) {
  property.traceCallback = func;
}

void nmea_context_set_error_func(nmeaErrorFunc func) {
  property.errorCallback = func;
}

void nmea_context_set_buffer_size(size_t sz) {
  property.parseBufferSize = (sz < NMEA_TRACE_ERROR_BUFF_MIN) ?
      NMEA_TRACE_ERROR_BUFF_MIN :
      sz;
}

size_t nmea_context_get_buffer_size(void) {
  return property.parseBufferSize;
}

void nmea_trace(const char *s, ...) {
  nmeaTraceFunc func = property.traceCallback;
  if (func) {
    int size;
    va_list arg_list;
    char *buff;
    size_t buffSize = property.parseBufferSize;

    buff = malloc(buffSize);
    buff[buffSize -1] = '\0';

    va_start(arg_list, s);
    size = vsnprintf(&buff[0], buffSize - 1, s, arg_list);
    va_end(arg_list);

    if (size > 0) {
      (*func)(&buff[0], size);
    }

    free(buff);
  }
}

void nmea_trace_buff(const char *s, size_t sz) {
  nmeaTraceFunc func = property.traceCallback;
  if (func && s && sz) {
    (*func)(s, sz);
  }
}

void nmea_error(const char *s, ...) {
  nmeaErrorFunc func = property.errorCallback;
  if (func) {
    int size;
    va_list arg_list;
    char *buff;
    size_t buffSize = property.parseBufferSize;

    buff = malloc(buffSize);
    buff[buffSize -1] = '\0';

    va_start(arg_list, s);
    size = vsnprintf(&buff[0], buffSize - 1, s, arg_list);
    va_end(arg_list);

    if (size > 0) {
      (*func)(&buff[0], size);
    }

    free(buff);
  }
}
