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

#ifndef __NMEALIB_PARSE_H__
#define __NMEALIB_PARSE_H__

#include <nmealib/info.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Parse nmeaTIME (time only, no date) from a string.
 *
 * The format that is used (hhmmss, hhmmss.s, hhmmss.ss or hhmmss.sss) is
 * determined by the length of the string.
 *
 * @param s The string
 * @param t The structure in which to store the parsed time
 * @param prefix The NMEA prefix
 * @return True on success, false otherwise
 */
bool _nmea_parse_time(const char *s, nmeaTIME *t, const char * prefix);

/**
 * Parse nmeaTIME (date only, no time) from a string.
 *
 * The month is adjusted (decremented by 1) to comply with the nmeaTIME month
 * range of [0, 11]. The year is adjusted (incremented by 100) for years
 * before 90 to comply with the nmeaTIME year range of [90, 189].
 *
 * @param date The date (DDMMYY)
 * @param t The structure in which to store the parsed date
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True on success, false otherwise
 */
bool _nmea_parse_date(const int date, nmeaTIME *t, const char * prefix, const char * s);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_PARSE_H__ */
