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

#ifndef __NMEALIB_GPGGA_H__
#define __NMEALIB_GPGGA_H__

#include <nmealib/info.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * GPGGA packet information structure (Global Positioning System Fix Data)
 *
 * Essential fix data which provide 3D location and accuracy data.
 *
 * <pre>
 * $GPGGA,time,lat,ns,lon,ew,sig,sats,hdop,elv,elv unit,height,height unit,dgps age,dgps id*checksum
 * </pre>
 *
 * | Field       | Description                                            | present       |
 * | :---------: | ------------------------------------------------------ | :-----------: |
 * | $GPGGA      | NMEA prefix                                            | -             |
 * | time        | Fix time, in the format HHMMSS.hh (UTC)                | UTCTIME       |
 * | lat         | Latitude, in NDEG (DDMM.SSS)                           | LAT (1)       |
 * | ns          | North or south ('N' or 'S')                            | LAT (1)       |
 * | lon         | Longitude, in NDEG (DDDMM.SSS)                         | LON (2)       |
 * | ew          | East or west ('E' or 'W')                              | LON (2)       |
 * | sig         | Signal quality, see the NMEA_SIG_* defines             | SIG           |
 * | sats        | Number of satellites being tracked                     | SATINUSECOUNT |
 * | hdop        | Horizontal dilution of position                        | HDOP          |
 * | elv         | Altitude above mean sea level, in meters               | ELV (3)       |
 * | elv unit    | Unit of altitude ('M')                                 | ELV (3)       |
 * | height      | Height of geoid (mean sea level) above WGS84 ellipsoid | - (4)         |
 * | height unit | Unit of height ('M')                                   | - (4)         |
 * | dgps age    | Time since last DGPS update, in seconds                | - (4)         |
 * | dgps id     | DGPS station ID number                                 | - (4)         |
 * | checksum    | NMEA checksum                                          | -             |
 *
 * (1) These fields are both required for a valid latitude<br/>
 * (2) These fields are both required for a valid longitude<br/>
 * (3) These fields are both required for a valid altitude<br/>
 * (4) Not supported yet<br/>
 *
 * Example:
 *
 * <pre>
 * $GPGGA,123519.43,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
 * </pre>
 *
 * Note that if the height of geoid is missing then the altitude should be
 * suspect. Some non-standard implementations report altitude with respect to
 * the ellipsoid rather than geoid altitude. Some units do not report negative
 * altitudes at all. This is the only sentence that reports altitude.
 */
typedef struct _nmeaGPGGA {
  uint32_t present;
  nmeaTIME time;
  double   lat;
  char     ns;
  double   lon;
  char     ew;
  int      sig;
  int      satinuse;
  double   HDOP;
  double   elv;
  char     elv_units;
  double   diff;
  char     diff_units;
  double   dgps_age;
  int      dgps_sid;
} nmeaGPGGA;

/**
 * Parse a GPGGA sentence from a string
 *
 * @param s The string
 * @param sz The length of the string
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
bool nmeaGPGGAparse(const char *s, const size_t sz, nmeaGPGGA *pack);

/**
 * Update an unsanitised nmeaINFO structure from a GPGGA packet structure
 *
 * @param pack The GPGGA packet structure
 * @param info The nmeaINFO structure
 */
void nmeaGPGGAToInfo(const nmeaGPGGA *pack, nmeaINFO *info);

/**
 * Convert a sanitised nmeaINFO structure into a nmeaGPGGA structure
 *
 * @param info The nmeaINFO structure
 * @param pack The nmeaGPGGA structure
 */
void nmeaGPGGAFromInfo(const nmeaINFO *info, nmeaGPGGA *pack);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_GPGGA_H__ */
