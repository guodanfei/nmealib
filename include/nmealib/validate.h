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

#ifndef __NMEALIB_VALIDATE_H__
#define __NMEALIB_VALIDATE_H__

#include <nmealib/sentence.h>
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
 * Validate the time fields in an nmeaTIME structure.
 *
 * Expects:
 * <pre>
 *   hour [0, 23]
 *   min  [0, 59]
 *   sec  [0, 60] (1 leap second)
 *   hsec [0, 99]
 * </pre>
 *
 * @param t The structure
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True when valid, false otherwise
 */
bool validateTime(const nmeaTIME * t, const char * prefix, const char * s);

/**
 * Validate the date fields in an nmeaTIME structure.
 *
 * Expects:
 * <pre>
 *   year  [90, 189]
 *   month [ 0,  11]
 *   day   [ 1,  31]
 * </pre>
 *
 * @param t a pointer to the structure
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return true when valid, false otherwise
 */
bool validateDate(const nmeaTIME * t, const char * prefix, const char * s);

/**
 * Validate north/south or east/west and upper-case it.
 *
 * Expects:
 * <pre>
 *   c in { n, N, s, S } (for north/south)
 *   c in { e, E, w, W } (for east/west)
 * </pre>
 *
 * @param c The character, will also be converted to upper-case.
 * @param ns Evaluate north/south when true, evaluate east/west otherwise
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True when valid, false otherwise
 */
bool validateNSEW(char * c, const bool ns, const char * prefix, const char * s);

/**
 * Validate a fix.
 *
 * Expects:
 * <pre>
 *   fix in [NMEA_FIX_FIRST, NMEA_FIX_LAST]
 * </pre>
 *
 * @param fix The fix
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True when valid, false otherwise
 */
bool validateFix(int * fix, const char * prefix, const char * s);

/**
 * Validate a signal.
 *
 * Expects:
 * <pre>
 *   sig in [NMEA_SIG_FIRST, NMEA_SIG_LAST]
 * </pre>
 *
 * @param sig The signal
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True when valid, false otherwise
 */
bool validateSignal(int * sig, const char * prefix, const char * s);

/**
 * Validate and upper-case the mode.
 *
 * Expects:
 * <pre>
 *   c in { A, D, E, F, M, N, P, R, S }
 *
 *   A = Autonomous
 *   D = Differential
 *   E = Estimated (dead reckoning)
 *   F = Float RTK (using floating integers)
 *   M = Manual input
 *   N = No fix
 *   P = Precise
 *   R = Real Time Kinematic (using fixed integers)
 *   S = Simulation mode
 * </pre>
 *
 * @param c The character, will also be converted to upper-case.
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True when valid, false otherwise
 */
bool validateMode(char * c, const char * prefix, const char * s);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_VALIDATE_H__ */
