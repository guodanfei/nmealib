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

#ifndef __NMEALIB_GPVTG_H__
#define __NMEALIB_GPVTG_H__

#include <nmealib/info.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * GPVTG packet information structure (Track made good and ground speed)
 *
 * <pre>
 * $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*checksum
 * </pre>
 *
 * | Field       | Description                           | present   |
 * | :---------: | ------------------------------------- | :-------: |
 * | $GPVTG      | NMEA prefix                           | -         |
 * | track       | Track, in degress true north          | TRACK (1) |
 * | T           | Track indicator (True north)          | TRACK (1) |
 * | mtrack      | Magnetic track made good              | TRACK (2) |
 * | M           | Magnetic track indicator (Made good)  | TRACK (2) |
 * | speed       | Ground speed, in knots                | SPEED (3) |
 * | N           | Ground speed unit (kNots)             | SPEED (3) |
 * | speedk      | Ground speed, in kph                  | SPEED (4) |
 * | K           | Ground speed unit (Kph)               | SPEED (4) |
 * | checksum    | NMEA checksum                         | -         |
 *
 * (1) These fields are both required for a valid track<br/>
 * (2) These fields are both required for a valid magnetic track<br/>
 * (3) These fields are both required for a valid speed<br/>
 * (4) These fields are both required for a valid speed<br/>
 *
 * Example:
 *
 * <pre>
 * $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48
 * </pre>
 */
typedef struct _nmeaGPVTG {
  uint32_t present;
  double   track;
  char     track_t;
  double   mtrack;
  char     mtrack_m;
  double   spn;
  char     spn_n;
  double   spk;
  char     spk_k;
} nmeaGPVTG;

/**
 * Parse a GPVTG sentence from a string
 *
 * @param s The string
 * @param sz The length of the string
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
bool nmeaGPVTGparse(const char *s, const size_t sz, nmeaGPVTG *pack);

/**
 * Update an unsanitised nmeaINFO structure from a GPVTG packet structure
 *
 * @param pack The GPVTG packet structure
 * @param info The nmeaINFO structure
 */
void nmeaGPVTGToInfo(const nmeaGPVTG *pack, nmeaINFO *info);

/**
 * Convert a sanitised nmeaINFO structure into a nmeaGPVTG structure
 *
 * @param info The nmeaINFO structure
 * @param pack The nmeaGPVTG structure
 */
void nmeaGPVTGFromInfo(const nmeaINFO *info, nmeaGPVTG *pack);

/**
 * Generate a GPVTG sentence from an nmeaGPVTG structure
 *
 * @param s a pointer to the buffer to generate the string in
 * @param len the size of the buffer
 * @param pack the structure
 * @return the length of the generated sentence
 */
int nmea_gen_GPVTG(char *s, const int len, const nmeaGPVTG *pack);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_GPVTG_H__ */
