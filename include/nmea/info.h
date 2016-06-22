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
 * NMEA Info
 *
 * The table below describes which fields are present in the sentences that are
 * supported by the library.
 * | field \ sentence       | GPGGA | GPGSA | GPGSV | GPRMC | GPVTG |
 * | :--------------------- | :---: | :---: | :---: | :---: | :---: |
 * | present:               | x     | x     | x     | x     | x     |
 * | smask:                 | x     | x     | x     | x     | x     |
 * | utc (date):            |       |       |       | x     |       |
 * | utc (time):            | x     |       |       | x     |       |
 * | sig:                   | x     |       |       | x (1) |       |
 * | fix:                   |       | x     |       | x (1) |       |
 * | PDOP:                  |       | x     |       |       |       |
 * | HDOP:                  | x     | x     |       |       |       |
 * | VDOP:                  |       | x     |       |       |       |
 * | lat:                   | x     |       |       | x     |       |
 * | lon:                   | x     |       |       | x     |       |
 * | elv:                   | x     |       |       |       |       |
 * | speed:                 |       |       |       | x     | x     |
 * | track:                 |       |       |       | x     | x     |
 * | mtrack:                |       |       |       |       | x     |
 * | magvar:                |       |       |       | x     |       |
 * | satinfo (inuse count): | x     | x (1) |       |       |       |
 * | satinfo (inuse):       |       | x     |       |       |       |
 * | satinfo (inview):      |       |       | x     |       |       |
 *
 * (1) Not present in the sentence but the library sets it up.
 */

#ifndef __NMEA_INFO_H__
#define __NMEA_INFO_H__

#include <nmea/compiler.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NMEA_SIG_FIRST        (NMEA_SIG_INVALID)
#define NMEA_SIG_INVALID      (0)
#define NMEA_SIG_FIX          (1)
#define NMEA_SIG_DIFFERENTIAL (2)
#define NMEA_SIG_SENSITIVE    (3)
#define NMEA_SIG_RTKIN        (4)
#define NMEA_SIG_FLOAT_RTK    (5)
#define NMEA_SIG_ESTIMATED    (6)
#define NMEA_SIG_MANUAL       (7)
#define NMEA_SIG_SIMULATION   (8)
#define NMEA_SIG_LAST         (NMEA_SIG_SIMULATION)

static INLINE const char * nmea_INFO_sig_to_string(int sig) {
  switch (sig) {
    case NMEA_SIG_INVALID:
      return "INVALID";
    case NMEA_SIG_FIX:
      return "FIX";
    case NMEA_SIG_DIFFERENTIAL:
      return "DIFFERENTIAL";
    case NMEA_SIG_SENSITIVE:
      return "SENSITIVE";
    case NMEA_SIG_RTKIN:
      return "REAL TIME KINEMATIC";
    case NMEA_SIG_FLOAT_RTK:
      return "FLOAT RTK";
    case NMEA_SIG_ESTIMATED:
      return "ESTIMATED (DEAD RECKONING)";
    case NMEA_SIG_MANUAL:
      return "MANUAL INPUT";
    case NMEA_SIG_SIMULATION:
      return "SIMULATION";
    default:
      return NULL;
  }
}

static INLINE int nmea_INFO_mode_to_sig(char mode) {
  switch (mode) {
    case 'N':
      return NMEA_SIG_INVALID;

    case 'A':
      return NMEA_SIG_FIX;

    case 'D':
      return NMEA_SIG_DIFFERENTIAL;

    case 'P':
      return NMEA_SIG_SENSITIVE;

    case 'R':
      return NMEA_SIG_RTKIN;

    case 'F':
      return NMEA_SIG_FLOAT_RTK;

    case 'E':
      return NMEA_SIG_ESTIMATED;

    case 'M':
      return NMEA_SIG_MANUAL;

    case 'S':
      return NMEA_SIG_SIMULATION;

    default:
      return NMEA_SIG_INVALID;
  }
}

static INLINE char nmea_INFO_sig_to_mode(int sig) {
  switch (sig) {
    case NMEA_SIG_INVALID:
      return 'N';

    case NMEA_SIG_FIX:
      return 'A';

    case NMEA_SIG_DIFFERENTIAL:
      return 'D';

    case NMEA_SIG_SENSITIVE:
      return 'P';

    case NMEA_SIG_RTKIN:
      return 'R';

    case NMEA_SIG_FLOAT_RTK:
      return 'F';

    case NMEA_SIG_ESTIMATED:
      return 'E';

    case NMEA_SIG_MANUAL:
      return 'M';

    case NMEA_SIG_SIMULATION:
      return 'S';

    default:
      return 'N';
  }
}

#define NMEA_FIX_FIRST (NMEA_FIX_BAD)
#define NMEA_FIX_BAD   (1)
#define NMEA_FIX_2D    (2)
#define NMEA_FIX_3D    (3)
#define NMEA_FIX_LAST  (NMEA_FIX_3D)

static INLINE const char * nmea_INFO_fix_to_string(int fix) {
  if (!fix) {
    return NULL;
  }

  switch (fix) {
    case NMEA_FIX_BAD:
      return "BAD";
    case NMEA_FIX_2D:
      return "2D";
    case NMEA_FIX_3D:
      return "3D";
    default:
      return NULL;
  }
}

#define NMEA_PREFIX_LENGTH 5

#define NMEA_MAXSAT    (72)
#define NMEA_SATINPACK (4)
#define NMEA_NSATPACKS (NMEA_MAXSAT / NMEA_SATINPACK)

#define NMEA_DEF_LAT   (0.0)
#define NMEA_DEF_LON   (0.0)

#define GPGSA_SAT_COUNT (12)

/**
 * Date and time data
 * @see nmea_time_now
 */
typedef struct _nmeaTIME {
    int year; /**< Years since 1900 */
    int mon; /**< Months since January - [0,11] */
    int day; /**< Day of the month - [1,31] */
    int hour; /**< Hours since midnight - [0,23] */
    int min; /**< Minutes after the hour - [0,59] */
    int sec; /**< Seconds after the minute - [0,60] (1 leap second) */
    int hsec; /**< Hundredth part of second - [0,99] */
} nmeaTIME;

/**
 * Position data in fractional degrees or radians
 */
typedef struct _nmeaPOS {
    double lat; /**< Latitude */
    double lon; /**< Longitude */
} nmeaPOS;

/**
 * Information about satellite
 * @see nmeaSATINFO
 * @see nmeaGPGSV
 */
typedef struct _nmeaSATELLITE {
    int id; /**< Satellite PRN number */
    int elv; /**< Elevation in degrees, 90 maximum */
    int azimuth; /**< Azimuth, degrees from true north, 000 to 359 */
    int sig; /**< Signal, 00-99 dB */
} nmeaSATELLITE;

/**
 * Information about all satellites in view
 * @see nmeaINFO
 * @see nmeaGPGSV
 */
typedef struct _nmeaSATINFO {
    int inuse; /**< Number of satellites in use (not those in view) */
    int in_use[NMEA_MAXSAT]; /**< IDs of satellites in use (not those in view) */
    int inview; /**< Total number of satellites in view */
    nmeaSATELLITE sat[NMEA_MAXSAT]; /**< Satellites information (in view) */
} nmeaSATINFO;

/**
 * Summary GPS information from all parsed packets,
 * used also for generating NMEA stream
 * @see nmea_parse
 * @see nmea_GPGGA2info,  nmea_...2info
 */
typedef struct _nmeaINFO {
    uint32_t present; /**< Mask specifying which fields are present */

    int smask; /**< Mask specifying from which sentences data has been obtained */

    nmeaTIME utc; /**< UTC of position */

    int sig; /**< GPS quality indicator: 0 = Invalid
     1 = Fix
     2 = Differential
     3 = Sensitive
     4 = Real Time Kinematic
     5 = Float RTK,
     6 = estimated (dead reckoning) (v2.3)
     7 = Manual input mode
     8 = Simulation mode) */

    int fix; /**< Operating mode, used for navigation: 1 = Fix not available
     2 = 2D
     3 = 3D) */

    double PDOP; /**< Position Dilution Of Precision */
    double HDOP; /**< Horizontal Dilution Of Precision */
    double VDOP; /**< Vertical Dilution Of Precision */

    double lat; /**< Latitude in NDEG:  +/-[degree][min].[sec/60] */
    double lon; /**< Longitude in NDEG: +/-[degree][min].[sec/60] */
    double elv; /**< Antenna altitude above/below mean sea level (geoid) in meters */
    double speed; /**< Speed over the ground in kph */
    double track; /**< Track angle in degrees True */
    double mtrack; /**< Magnetic Track angle in degrees True */
    double magvar; /**< Magnetic variation degrees */

    nmeaSATINFO satinfo; /**< Satellites information */
} nmeaINFO;

/**
 * Enumeration for the fields names of a nmeaINFO structure.
 * The values are used in the 'present' mask.
 */
typedef enum _nmeaINFO_FIELD {
  SMASK = (1u << 0), /* 0x00001 */
  UTCDATE = (1u << 1), /* 0x00002 */
  UTCTIME = (1u << 2), /* 0x00004 */
  SIG = (1u << 3), /* 0x00008 */
  FIX = (1u << 4), /* 0x00010 */
  PDOP = (1u << 5), /* 0x00020 */
  HDOP = (1u << 6), /* 0x00040 */
  VDOP = (1u << 7), /* 0x00080 */
  LAT = (1u << 8), /* 0x00100 */
  LON = (1u << 9), /* 0x00200 */
  ELV = (1u << 10), /* 0x00400 */
  SPEED = (1u << 11), /* 0x00800 */
  TRACK = (1u << 12), /* 0x01000 */
  MTRACK = (1u << 13), /* 0x02000 */
  MAGVAR = (1u << 14), /* 0x04000 */
  SATINUSECOUNT = (1u << 15), /* 0x08000 */
  SATINUSE = (1u << 16), /* 0x10000 */
  SATINVIEW = (1u << 17), /* 0x20000 */
  _nmeaINFO_FIELD_LAST = SATINVIEW
} nmeaINFO_FIELD;

#define NMEA_INFO_PRESENT_MASK ((_nmeaINFO_FIELD_LAST << 1) - 1)

static INLINE const char * nmea_INFO_field_to_string(nmeaINFO_FIELD field) {
  if (!field) {
    return NULL;
  }

  switch (field) {
    case SMASK:
      return "SMASK";
    case UTCDATE:
      return "UTCDATE";
    case UTCTIME:
      return "UTCTIME";
    case SIG:
      return "SIG";
    case FIX:
      return "FIX";
    case PDOP:
      return "PDOP";
    case HDOP:
      return "HDOP";
    case VDOP:
      return "VDOP";
    case LAT:
      return "LAT";
    case LON:
      return "LON";
    case ELV:
      return "ELV";
    case SPEED:
      return "SPEED";
    case TRACK:
      return "TRACK";
    case MTRACK:
      return "MTRACK";
    case MAGVAR:
      return "MAGVAR";
    case SATINUSECOUNT:
      return "SATINUSECOUNT";
    case SATINUSE:
      return "SATINUSE";
    case SATINVIEW:
      return "SATINVIEW";
    default:
      return NULL;
  }
}

/**
 * NMEA packets type which parsed and generated by library
 */
enum nmeaPACKTYPE {
  GPNON = 0, /**< Unknown packet type. */
  GPGGA = (1u << 0), /**< GGA - Essential fix data which provide 3D location and accuracy data. */
  GPGSA = (1u << 1), /**< GSA - GPS receiver operating mode, SVs used for navigation, and DOP values. */
  GPGSV = (1u << 2), /**< GSV - Number of SVs in view, PRN numbers, elevation, azimuth & SNR values. */
  GPRMC = (1u << 3), /**< RMC - Recommended Minimum Specific GPS/TRANSIT Data. */
  GPVTG = (1u << 4), /**< VTG - Actual track made good and speed over ground. */
  _nmeaPACKTYPE_LAST = GPVTG
};

void nmea_time_now(nmeaTIME *utc, uint32_t * present);
void nmea_zero_INFO(nmeaINFO *info);

bool nmea_INFO_is_present_smask(int smask, nmeaINFO_FIELD fieldName);

/**
 * Determine if a nmeaINFO structure has a certain field
 *
 * @param present the presence field
 * @param fieldName use a name from nmeaINFO_FIELD
 * @return a boolean, true when the structure has the requested field
 */
static INLINE bool nmea_INFO_is_present(uint32_t present, nmeaINFO_FIELD fieldName) {
  return ((present & fieldName) != 0);
}

void nmea_INFO_set_present(uint32_t * present, nmeaINFO_FIELD fieldName);
void nmea_INFO_unset_present(uint32_t * present, nmeaINFO_FIELD fieldName);

void nmea_INFO_sanitise(nmeaINFO *nmeaInfo);

void nmea_INFO_unit_conversion(nmeaINFO * nmeaInfo);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEA_INFO_H__ */
