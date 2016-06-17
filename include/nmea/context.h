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

/** The default size for temporary trace and error logging buffers */
#define NMEA_TRACE_ERROR_BUFF_DEF  4096

/** The minimum size for temporary trace and error logging buffers */
#define NMEA_TRACE_ERROR_BUFF_MIN   256

/**
 * Function type definition for tracing functions
 *
 * @param s The string to trace
 * @param sz The length of the string
 */
typedef void (*nmeaTraceFunc)(const char *s, size_t sz);

/**
 * Function type definition for error logging functions
 *
 * @param s The string to log
 * @param sz The length of the string
 */
typedef void (*nmeaErrorFunc)(const char *s, size_t sz);

/**
 * Set the trace function
 *
 * Note that only 1 trace function is accepted, it will overwrite
 * any trace function that was previously set.
 *
 * Setting the function to NULL disables tracing.
 *
 * The function can be set at any time.
 *
 * @param func The trace function
 */
void nmea_context_set_trace_func(nmeaTraceFunc func);

/**
 * Set the error logging function
 *
 * Note that only 1 error logging function is accepted, it will overwrite
 * any error logging function that was previously set.
 *
 * Setting the function to NULL disables error logging.
 *
 * The function can be set at any time.
 *
 * @param func The error logging function
 */
void nmea_context_set_error_func(nmeaErrorFunc func);

/**
 * Set the buffer size for temporary trace and error logging buffers
 *
 * If the size is less than NMEA_TRACE_ERROR_BUFF_MIN, then the size that is
 * configured will be NMEA_TRACE_ERROR_BUFF_MIN.
 *
 * @param sz The buffer size for temporary trace and error logging buffers
 */
void nmea_context_set_buffer_size(size_t sz);

/**
 * @return The buffer size for temporary trace and error logging buffers
 */
size_t nmea_context_get_buffer_size(void);

/**
 * Trace a formatted string
 *
 * @param s The formatted string to trace
 */
void nmea_trace(const char *s, ...) __attribute__ ((format(printf, 1, 2)));

/**
 * Trace a sized string (buffer)
 *
 * @param s The size string
 * @param sz The length (size) of the string (buffer)
 */
void nmea_trace_buff(const char *s, size_t sz);

/**
 * Log a formatted error string as an error
 *
 * @param s The formatted string to log as an error
 */
void nmea_error(const char *s, ...) __attribute__ ((format(printf, 1, 2)));

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEA_CONTEXT_H__ */
