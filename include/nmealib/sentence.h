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

/**
 * @file
 * NMEA Sentences
 *
 * The table below describes which nmeaINFO fields are present in the
 * sentences that are supported by the library.
 *
 * | nmeaINFO field       | GPGGA | GPGSA | GPGSV | GPRMC | GPVTG |
 * | :------------------- | :---: | :---: | :---: | :---: | :---: |
 * | present              | x     | x     | x     | x     | x     |
 * | smask                | x     | x     | x     | x     | x     |
 * | utc (date)           |       |       |       | x     |       |
 * | utc (time)           | x     |       |       | x     |       |
 * | sig                  | x     |       |       | x (1) |       |
 * | fix                  |       | x     |       | x (1) |       |
 * | PDOP                 |       | x     |       |       |       |
 * | HDOP                 | x     | x     |       |       |       |
 * | VDOP                 |       | x     |       |       |       |
 * | lat                  | x     |       |       | x     |       |
 * | lon                  | x     |       |       | x     |       |
 * | elv                  | x     |       |       |       |       |
 * | speed                |       |       |       | x     | x     |
 * | track                |       |       |       | x     | x     |
 * | mtrack               |       |       |       |       | x     |
 * | magvar               |       |       |       | x     |       |
 * | dgps_age             | x (2) |       |       |       |       |
 * | dgps_sid             | x (2) |       |       |       |       |
 * | satinfo inuse count  | x     |       |       |       |       |
 * | satinfo inuse        |       | x     |       |       |       |
 * | satinfo inview count |       |       | x     |       |       |
 * | satinfo inview       |       |       | x     |       |       |
 *
 * (1) Not present in the sentence but the library sets it up.
 * (2) Not supported yet
 */

#ifndef __NMEALIB_SENTENCE_H__
#define __NMEALIB_SENTENCE_H__

#include <nmealib/gpgga.h>
#include <nmealib/gpgsa.h>
#include <nmealib/gpgsv.h>
#include <nmealib/gprmc.h>
#include <nmealib/gpvtg.h>

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

/** The bit-mask with all NmeaSentence entries */
#define NMEA_SENTENCE_MASK (_NmeaSentenceLast - 1)

/** The fixed length of a NMEA prefix */
#define NMEA_PREFIX_LENGTH 5

/**
 * Determine the NMEA prefix from the sentence type.
 *
 * @param sentence The sentence type
 * @return The NMEA prefix, or NULL when the sentence type is unknown
 */
const char * nmeaSentenceToPrefix(enum NmeaSentence sentence);

/**
 * Determine the sentence type from the start of the specified NMEA
 * sentence. If the first character of the string is equal
 * to the NMEA start-of-line character ('$') then that character is
 * skipped.
 *
 * @param s The NMEA sentence
 * @param sz The length of the NMEA sentence
 * @return The packet type, or GPNON when it could not be determined
 */
enum NmeaSentence nmeaPrefixToSentence(const char *s, const size_t sz);

/**
 * Parse a NMEA sentence into an (unsanitised) nmeaINFO structure
 *
 * @param s The NMEA sentence
 * @param sz The length of the NMEA sentence
 * @param info The nmeaINFO structure in which to stored the information
 * @return True when successful
 */
bool nmeaSentenceToInfo(const char *s, const size_t sz, nmeaINFO *info);

/**
 * Generate NMEA sentences from a (sanitised) nmeaINFO structure.
 *
 * @param s The buffer in which to generate the sentences
 * @param sz The size of the buffer
 * @param info The (sanitised) nmeaINFO structure
 * @param mask The bit-mask of sentences to generate
 * @return The total length of the generated sentences
 */
int nmeaSentenceGenerate(char *s, const size_t sz, const nmeaINFO *info, const enum NmeaSentence mask);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_SENTENCE_H__ */
