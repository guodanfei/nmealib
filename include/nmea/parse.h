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

#ifndef __NMEA_PARSE_H__
#define __NMEA_PARSE_H__

#include <nmea/sentence.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * The type definition for an invalid NMEA character/description
 */
typedef struct {
    const char character;
    const char * description;
} InvalidNMEACharacter;

/**
 * Determine whether the given character is not allowed in an NMEA string
 *
 * @param c The character to check
 *
 * @return The invalid character name/description when the string has invalid
 * characters, NULL otherwise
 */
const InvalidNMEACharacter * isInvalidNMEACharacter(const char * c);

/**
 * Determine whether the specified string contains characters that are not
 * allowed in an NMEA string
 *
 * @param s The string to check
 * @param sz The length of the string to check
 *
 * @return The invalid character name/description when the string has invalid
 * characters, NULL otherwise
 */
const InvalidNMEACharacter * nmea_parse_sentence_has_invalid_chars(const char * s, const size_t sz);

/**
 * Determine the sentence type (see nmeaPACKTYPE) from the start of the
 * specified string (an NMEA sentence). The '$' character with which an
 * NMEA sentence start must NOT be at the start of the specified string.
 *
 * @param s The string. Must be the NMEA string right after the initial '$'
 * character
 * @param sz The length of the string
 * @return The packet type, or GPNON when it could not be determined
 */
enum nmeaPACKTYPE nmea_parse_get_sentence_type(const char *s, const size_t sz);

/**
 * Parse a GPGGA sentence from a string
 *
 * @param s The string
 * @param sz The length of the string
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
bool nmea_parse_GPGGA(const char *s, const size_t sz, nmeaGPGGA *pack);

/**
 * Parse a GPGSA sentence from a string
 *
 * @param s The string
 * @param sz The length of the string
 * @param hasChecksum True when the string contains a checksum
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
bool nmea_parse_GPGSA(const char *s, const size_t sz, bool hasChecksum, nmeaGPGSA *pack);

/**
 * Parse a GPGSV sentence from a string
 *
 * @param s The string
 * @param sz The length of the string
 * @param hasChecksum True when the string contains a checksum
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
bool nmea_parse_GPGSV(const char *s, const size_t sz, bool hasChecksum, nmeaGPGSV *pack);

/**
 * Parse a GPRMC sentence from a string
 *
 * @param s The string
 * @param sz The length of the string
 * @param hasChecksum True when the string contains a checksum
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
bool nmea_parse_GPRMC(const char *s, const size_t sz, bool hasChecksum, nmeaGPRMC *pack);

/**
 * Parse a GPVTG sentence from a string
 *
 * @param s The string
 * @param sz The length of the string
 * @param hasChecksum True when the string contains a checksum
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
bool nmea_parse_GPVTG(const char *s, const size_t sz, bool hasChecksum, nmeaGPVTG *pack);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEA_PARSE_H__ */
