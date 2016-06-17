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

#include <nmea/context.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/**
 * The structure with nmealib context.
 */
typedef struct _nmeaPROPERTY {
  nmeaTraceFunc traceCallback; /**< the tracing callback, defaults to NULL (disabled)     */
  nmeaErrorFunc errorCallback; /**< the error callback, defaults to NULL (disabled)       */
  size_t parseBufferSize;      /**< the size to use for temporary trace and error buffers */
} nmeaPROPERTY;

/** the nmealib context */
static nmeaPROPERTY property = {
    .traceCallback = NULL,
    .errorCallback = NULL,
    .parseBufferSize = NMEA_TRACE_ERROR_BUFF_DEF
};

/**
 * Set the trace function
 *
 * @param func the trace function
 */
void nmea_context_set_trace_func(nmeaTraceFunc func) {
  property.traceCallback = func;
}

/**
 * Set the error function
 *
 * @param func the error function
 */
void nmea_context_set_error_func(nmeaErrorFunc func) {
  property.errorCallback = func;
}

/**
 * Set the buffer size for temporary buffers.
 * If the size is less than NMEA_MIN_PARSEBUFF, then the size that is
 * configured will be NMEA_MIN_PARSEBUFF.
 *
 * @param sz the buffer size for temporary buffers
 */
void nmea_context_set_buffer_size(size_t sz) {
  property.parseBufferSize = (sz < NMEA_TRACE_ERROR_BUFF_MIN) ?
      NMEA_TRACE_ERROR_BUFF_MIN :
      sz;
}

/**
 * @return the buffer size for temporary buffers
 */
size_t nmea_context_get_buffer_size(void) {
  return property.parseBufferSize;
}

/**
 * Trace a formatted string
 *
 * @param s a formatted string
 */
void nmea_trace(const char *s, ...) {
  nmeaTraceFunc func = property.traceCallback;
  if (func) {
    int size;
    va_list arg_list;
    char *buff;

    buff = malloc(property.parseBufferSize);

    va_start(arg_list, s);
    size = vsnprintf(&buff[0], property.parseBufferSize - 1, s, arg_list);
    va_end(arg_list);

    if (size > 0) {
      (*func)(&buff[0], size);
    }

    free(buff);
  }
}

/**
 * Trace a buffer
 *
 * @param s a pointer to the buffer
 * @param sz the size of the buffer
 */
void nmea_trace_buff(const char *s, size_t sz) {
  nmeaTraceFunc func = property.traceCallback;
  if (func && s && sz) {
    (*func)(s, sz);
  }
}

/**
 * Log a formatted error string
 *
 * @param s a formatted error string
 */
void nmea_error(const char *s, ...) {
  nmeaErrorFunc func = property.errorCallback;
  if (func) {
    int size;
    va_list arg_list;
    char *buff;

    buff = malloc(property.parseBufferSize);

    va_start(arg_list, s);
    size = vsnprintf(&buff[0], property.parseBufferSize - 1, s, arg_list);
    va_end(arg_list);

    if (size > 0) {
      (*func)(&buff[0], size);
    }

    free(buff);
  }
}
