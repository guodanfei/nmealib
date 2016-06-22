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
 * Extended descriptions of sentences are taken from
 *   http://www.gpsinformation.org/dale/nmea.htm
 */

#ifndef __NMEALIB_GPGSA_H__
#define __NMEALIB_GPGSA_H__

#include <nmea/info.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GPGSA_SAT_COUNT (12)

/**
 * GSA packet information structure (Satellite status)
 *
 * GPS DOP and active satellites.
 *
 * <pre>
 * $GPGSA,selection,fix,prn1,prn2,prn3,,,,,,,,,prn12,pdop,hdop,vdop*checksum
 * </pre>
 *
 * | Field       | Description                                      | present                   |
 * | :---------: | ------------------------------------------------ | :-----------------------: |
 * | $GPGSA      | NMEA prefix                                      | -                         |
 * | selection   | Selection of 2D or 3D fix (A = auto, M = manual) | SIG                       |
 * | fix         | Fix, see NMEA_FIX_* defines                      | FIX                       |
 * | prn1..prn12 | PRNs of satellites used for fix (12 PRNs)        | SATINUSE \| SATINUSECOUNT |
 * | pdop        | Dilution of position                             | PDOP                      |
 * | hdop        | Horizontal dilution of position                  | HDOP                      |
 * | vdop        | Vertical dilution of position                    | VDOP                      |
 * | checksum    | NMEA checksum                                    | -                         |
 *
 * This sentence provides details on the nature of the fix. It includes the
 * numbers of the satellites being used in the current solution and the DOP.
 *
 * DOP (dilution of precision) is an indication of the effect of satellite
 * geometry on the accuracy of the fix. It is a unit-less number where smaller
 * is better. For 3D fixes using 4 satellites a 1.0 would be considered to be
 * a perfect number, however for over-determined solutions it is possible to see
 * numbers below 1.0.
 *
 * There are differences in the way the PRN's are presented which can effect the
 * ability of some programs to display this data. For example, in the example
 * shown below there are 5 satellites in the solution and the null fields are
 * scattered indicating that the almanac would show satellites in the null
 * positions that are not being used as part of this solution. Other receivers
 * might output all of the satellites used at the beginning of the sentence with
 * the null field all stacked up at the end. This difference accounts for some
 * satellite display programs not always being able to display the satellites
 * being tracked. Some units may show all satellites that have ephemeris data
 * without regard to their use as part of the solution but this is non-standard.
 *
 * Example:
 *
 * <pre>
 * $GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39
 * </pre>
 */
typedef struct _nmeaGPGSA {
  uint32_t present;
  char     sig;
  int      fix;
  int      sat_prn[GPGSA_SAT_COUNT];
  double   PDOP;
  double   HDOP;
  double   VDOP;
} nmeaGPGSA;

/**
 * Parse a GPGSA sentence from a string
 *
 * @param s The string
 * @param sz The length of the string
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
bool nmea_parse_GPGSA(const char *s, const size_t sz, nmeaGPGSA *pack);

/**
 * Update an unsanitised nmeaINFO structure from a GSA packet structure
 *
 * @param pack The GSA packet structure
 * @param info The nmeaINFO structure
 */
void nmea_GPGSA2info(const nmeaGPGSA *pack, nmeaINFO *info);

/**
 * Convert a sanitised nmeaINFO structure into a nmeaGPGSA structure
 *
 * @param info The nmeaINFO structure
 * @param pack The nmeaGPGSA structure
 */
void nmea_info2GPGSA(const nmeaINFO *info, nmeaGPGSA *pack);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_GPGSA_H__ */
