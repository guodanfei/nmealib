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

#ifndef __NMEALIB_TOK_H__
#define __NMEALIB_TOK_H__

#include <nmealib/info.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Parse a NMEA time into a nmeaTIME structure (time only, no date).
 *
 * The format that is used (HHMMSS, HHMMSS.t, HHMMSS.hh or HHMMSS.mmm) is
 * determined by the length of the string.
 *
 * @param s The time
 * @param time The structure in which to store the parsed time
 * @return True on success
 */
bool nmeaTimeParseTime(const char *s, NmeaTime *time);

/**
 * Parse a NMEA date into a nmeaTIME structure (date only, no time).
 *
 * The month is adjusted (decremented by 1) to comply with the nmeaTIME month
 * range of [0, 11]. The year is adjusted (incremented by 100) for years
 * before 90 to comply with the nmeaTIME year range of [90, 189].
 *
 * @param s The date (DDMMYY)
 * @param date The structure in which to store the parsed date
 * @return True on success
 */
bool nmeaTimeParseDate(const char *s, NmeaTime *date);

/**
 * Calculate the NMEA (CRC-8) checksum of a NMEA sentence.
 *
 * If the string starts with the NMEA start-of-line character '$' then that
 * character is skipped as per the NMEA spec.
 *
 * @param s The NMEA sentence
 * @param sz The length of the NMEA sentence
 * @return The NMEA checksum
 */
unsigned int nmeaCalculateCRC(const char *s, const size_t sz);

/**
 * Convert a string to an integer
 *
 * @param s The string
 * @param sz The length of the string
 * @param radix The radix of the numbers in the string
 * @return The converted number, or 0 on failure
 */
int nmeaStringToInteger(const char *s, const size_t sz, const int radix);

/**
 * Convert a string to an unsigned integer
 *
 * @param s The string
 * @param sz The length of the string
 * @param radix The radix of the numbers in the string
 * @return The converted number, or 0 on failure
 */
unsigned int nmeaStringToUnsignedInteger(const char *s, size_t sz, int radix);

/**
 * Convert string to a long integer
 *
 * @param s The string
 * @param sz The length of the string
 * @param radix The radix of the numbers in the string
 * @return The converted number, or 0 on failure
 */
long nmeaStringToLong(const char *s, size_t sz, int radix);

/**
 * Convert string to an unsigned long integer
 *
 * @param s The string
 * @param sz The length of the string
 * @param radix The radix of the numbers in the string
 * @return The converted number, or 0 on failure
 */
unsigned long nmeaStringToUnsignedLong(const char *s, size_t sz, int radix);

/**
 * Convert string to a floating point number
 *
 * @param s The string
 * @param sz The length of the string
 * @return The converted number, or 0.0 on failure
 */
double nmeaStringToDouble(const char *s, const size_t sz);

/**
 * Append a NMEA checksum to the string in the buffer
 *
 * @param s The buffer containing the string
 * @param sz The size of the buffer
 * @param len The length of the string in the buffer
 * @return The number of printed characters, -1 on error
 */
int nmeaAppendChecksum(char *s, size_t sz, size_t len);

/**
 * Format a string (with vsnprintf) and add the NMEA checksum
 *
 * @param s The buffer
 * @param sz The size of the buffer
 * @param format The string format to use
 * @return The number of printed characters, -1 on error
 */
int nmeaPrintf(char *s, size_t sz, const char *format, ...) __attribute__ ((format(printf, 3, 4)));

/**
 * Analyse a string (specific for NMEA sentences)
 *
 * @param s the string
 * @param sz the length of the string
 * @param format the string format to use
 * @return the number of scanned tokens
 */
size_t nmeaScanf(const char *s, size_t sz, const char *format, ...);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_TOK_H__ */
