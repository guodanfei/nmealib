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

#ifndef __NMEALIB_SENTENCE_H__
#define __NMEALIB_SENTENCE_H__

#include <nmea/gpgga.h>
#include <nmea/gpgsa.h>
#include <nmea/gpgsv.h>
#include <nmea/gprmc.h>
#include <nmea/gpvtg.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Supported NMEA sentences
 */
enum NmeaSentence {
  GPNON = 0,
  GPGGA = (1u << 0),
  GPGSA = (1u << 1),
  GPGSV = (1u << 2),
  GPRMC = (1u << 3),
  GPVTG = (1u << 4),
  _NmeaSentenceLast = GPVTG
};

/**
 * Determine the NMEA prefix from the sentence type.
 *
 * @param sentence The sentence type
 * @return The NMEA prefix, or NULL when the sentence type is unknown
 */
const char * nmeaSentenceToPrefix(enum NmeaSentence sentence);

/**
 * Determine the sentence type from the start of the specified string
 * (an NMEA sentence). The '$' character with which an NMEA sentence
 * starts must NOT be at the start of the specified string.
 *
 * @param s The string. Must be the NMEA string right after the initial '$'
 * character
 * @param sz The length of the string
 * @return The packet type, or GPNON when it could not be determined
 */
enum NmeaSentence nmeaPrefixToSentence(const char *s, const size_t sz);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_SENTENCE_H__ */
