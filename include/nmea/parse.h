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
 * Determine whether the given character is not allowed in an NMEA string
 *
 * @param c The character to check
 *
 * @return The invalid character name/description when the string has invalid
 * characters, NULL otherwise
 */
const char * isInvalidNMEACharacter(const char * c);

/**
 * Determine whether the specified string contains characters that are not
 * allowed in an NMEA string
 *
 * @param s The string to check
 * @param len The length of the string to check
 *
 * @return The invalid character name/description when the string has invalid
 * characters, NULL otherwise
 */
const char * nmea_parse_sentence_has_invalid_chars(const char * s, const size_t len);

/**
 * Determine the sentence type (see nmeaPACKTYPE) from the start of the
 * specified string (an NMEA sentence). The '$' character with which an
 * NMEA sentence start must NOT be at the start of the specified string.
 *
 * @param s The string. Must be the NMEA string right after the initial '$'
 * character
 * @param len The length of the string
 * @return The packet type, or GPNON when it could not be determined
 */
enum nmeaPACKTYPE nmea_parse_get_sentence_type(const char *s, const int len);

/**
 * Parse a GPGGA sentence from a string
 *
 * @param s The string
 * @param len The length of the string
 * @param has_checksum True when the string contains a checksum
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
int nmea_parse_GPGGA(const char *s, const int len, bool has_checksum, nmeaGPGGA *pack);

/**
 * Parse a GPGSA sentence from a string
 *
 * @param s The string
 * @param len The length of the string
 * @param has_checksum True when the string contains a checksum
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
int nmea_parse_GPGSA(const char *s, const int len, bool has_checksum, nmeaGPGSA *pack);

/**
 * Parse a GPGSV sentence from a string
 *
 * @param s The string
 * @param len The length of the string
 * @param has_checksum True when the string contains a checksum
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
int nmea_parse_GPGSV(const char *s, const int len, bool has_checksum, nmeaGPGSV *pack);

/**
 * Parse a GPRMC sentence from a string
 *
 * @param s The string
 * @param len The length of the string
 * @param has_checksum True when the string contains a checksum
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
int nmea_parse_GPRMC(const char *s, const int len, bool has_checksum, nmeaGPRMC *pack);

/**
 * Parse a GPVTG sentence from a string
 *
 * @param s The string
 * @param len The length of the string
 * @param has_checksum True when the string contains a checksum
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
int nmea_parse_GPVTG(const char *s, const int len, bool has_checksum, nmeaGPVTG *pack);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEA_PARSE_H__ */
