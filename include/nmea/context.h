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

#ifndef __NMEA_CONTEXT_H__
#define __NMEA_CONTEXT_H__

#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/** the default size for the temporary buffers */
#define NMEA_TRACE_ERROR_BUFF_DEF  4096

/** the minimum size for the temporary buffers */
#define NMEA_TRACE_ERROR_BUFF_MIN   256

/**
 * Function type definition for tracing
 *
 * @param s the string to trace
 * @param sz the length of the string
 */
typedef void (*nmeaTraceFunc)(const char *s, size_t sz);

/**
 * Function type definition for error logging
 *
 * @param s the string to log
 * @param sz the length of the string
 */
typedef void (*nmeaErrorFunc)(const char *s, size_t sz);

void nmea_context_set_trace_func(nmeaTraceFunc func);
void nmea_context_set_error_func(nmeaErrorFunc func);
void nmea_context_set_buffer_size(size_t sz);
size_t nmea_context_get_buffer_size(void);

void nmea_trace(const char *s, ...) __attribute__ ((format(printf, 1, 2)));
void nmea_trace_buff(const char *s, size_t sz);
void nmea_error(const char *s, ...) __attribute__ ((format(printf, 1, 2)));

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEA_CONTEXT_H__ */
