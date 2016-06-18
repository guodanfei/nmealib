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

#include <stdarg.h>
#include <stdio.h>

/**
 * The structure with nmealib context.
 */
typedef struct _nmeaPROPERTY {
  nmeaTraceFunc trace_func; /**< the tracing function, defaults to NULL (disabled) */
  nmeaErrorFunc error_func; /**< the error function, defaults to NULL (disabled) */
  int parse_buff_size; /**< the size to use for temporary buffers, minimum is NMEA_MIN_PARSEBUFF */
} nmeaPROPERTY;

/** the nmealib context */
static nmeaPROPERTY property = {
    .trace_func = NULL,
    .error_func = NULL,
    .parse_buff_size = NMEA_TRACE_ERROR_BUFF_DEF };

/**
 * Set the trace function
 *
 * @param func the trace function
 */
void nmea_context_set_trace_func(nmeaTraceFunc func) {
  property.trace_func = func;
}

/**
 * Set the error function
 *
 * @param func the error function
 */
void nmea_context_set_error_func(nmeaErrorFunc func) {
  property.error_func = func;
}

/**
 * Set the buffer size for temporary buffers.
 * If the size is less than NMEA_MIN_PARSEBUFF, then the size that is
 * configured will be NMEA_MIN_PARSEBUFF.
 *
 * @param sz the buffer size for temporary buffers
 */
void nmea_context_set_buffer_size(int sz) {
  if (sz < NMEA_TRACE_ERROR_BUFF_MIN)
    property.parse_buff_size = NMEA_TRACE_ERROR_BUFF_MIN;
  else
    property.parse_buff_size = sz;
}

/**
 * @return the buffer size for temporary buffers
 */
int nmea_context_get_buffer_size(void) {
  return property.parse_buff_size;
}

/**
 * Trace a formatted string
 *
 * @param s a formatted string
 */
void nmea_trace(const char *s, ...) {
  nmeaTraceFunc func = property.trace_func;

  if (func) {
    int size;
    va_list arg_list;
    char buff[property.parse_buff_size];

    va_start(arg_list, s);
    size = vsnprintf(&buff[0], property.parse_buff_size - 1, s, arg_list);
    va_end(arg_list);

    if (size > 0)
      (*func)(&buff[0], size);
  }
}

/**
 * Trace a buffer
 *
 * @param s a pointer to the buffer
 * @param sz the size of the buffer
 */
void nmea_trace_buff(const char *s, int sz) {
  nmeaTraceFunc func = property.trace_func;
  if (func && sz)
    (*func)(s, sz);
}

/**
 * Log a formatted error string
 *
 * @param s a formatted error string
 */
void nmea_error(const char *s, ...) {
  nmeaErrorFunc func = property.error_func;

  if (func) {
    int size;
    va_list arg_list;
    char buff[property.parse_buff_size];

    va_start(arg_list, s);
    size = vsnprintf(&buff[0], property.parse_buff_size - 1, s, arg_list);
    va_end(arg_list);

    if (size > 0)
      (*func)(&buff[0], size);
  }
}
