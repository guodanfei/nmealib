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

#ifndef __NMEALIB_INFO_H__
#define __NMEALIB_INFO_H__

#include <nmealib/util.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * SIG
 */

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

/**
 * Convert a NMEA_SIG_* define into a string
 *
 * @param sig The NMEA_SIG_* define
 * @return The corresponding string, or NULL when the define is unknown
 */
const char * nmea_INFO_sig_to_string(int sig);

/**
 * Convert a mode character into the corresponding NMEA_SIG_* define
 *
 * @param mode The mode character
 * @return The corresponding NMEA_SIG_* define, or NMEA_SIG_INVALID when the
 * mode is unknown
 */
int nmea_INFO_mode_to_sig(char mode);

/**
 * Convert a NMEA_SIG_* define into the corresponding mode character
 *
 * @param sig The NMEA_SIG_* define
 * @return The corresponding mode character, or 'N' when the NMEA_SIG_* define
 * is unknown
 */
char nmea_INFO_sig_to_mode(int sig);

/*
 * FIX
 */

#define NMEA_FIX_FIRST (NMEA_FIX_BAD)
#define NMEA_FIX_BAD   (1)
#define NMEA_FIX_2D    (2)
#define NMEA_FIX_3D    (3)
#define NMEA_FIX_LAST  (NMEA_FIX_3D)

/**
 * Convert a NMEA_FIX_* define into a string
 *
 * @param fix The NMEA_FIX_* define
 * @return The corresponding string, or NULL when the NMEA_FIX_* define is
 * unknown
 */
const char * nmea_INFO_fix_to_string(int fix);

/*
 * Limits and defaults
 */

#define NMEALIB_MAX_SATELLITES (72)

#define NMEA_DEF_LAT           (0.0)
#define NMEA_DEF_LON           (0.0)

/**
 * Date and time data
 */
typedef struct _nmeaTIME {
  int year; /**< Years since 1900         - [90, 189]                 */
  int mon;  /**< Months since January     - [ 0,  11]                 */
  int day;  /**< Day of the month         - [ 1,  31]                 */
  int hour; /**< Hours since midnight     - [ 0,  23]                 */
  int min;  /**< Minutes after the hour   - [ 0,  59]                 */
  int sec;  /**< Seconds after the minute - [ 0,  60] (1 leap second) */
  int hsec; /**< Hundredth part of second - [ 0,  99]                 */
} nmeaTIME;

/**
 * Position data in fractional degrees or radians
 */
typedef struct _nmeaPOS {
  double lat; /**< Latitude  */
  double lon; /**< Longitude */
} nmeaPOS;

/**
 * Information about satellite
 */
typedef struct _nmeaSATELLITE {
  int id;      /**< Satellite PRN number             - [1, inf) */
  int elv;     /**< Elevation, in degrees            - [0,  90] */
  int azimuth; /**< Azimuth, degrees from true north - [0, 359] */
  int sig;     /**< Signal                           - [0,  99] */
} nmeaSATELLITE;

/**
 * Information about all tracked satellites
 */
typedef struct _nmeaSATINFO {
  int inuse;                                 /**< The number of satellites in use (not those in view) */
  int in_use[NMEALIB_MAX_SATELLITES];        /**< The PRNs of satellites in use   (not those in view) */
  int inview;                                /**< The number of satellites in view                    */
  nmeaSATELLITE sat[NMEALIB_MAX_SATELLITES]; /**< Satellites information (in view)                    */
} nmeaSATINFO;

/**
 * GPS information from all supported sentences, used also for generating NMEA sentences
 */
typedef struct _nmeaINFO {
  uint32_t    present; /**< Bit-mask specifying which fields are present                    */
  int         smask;   /**< Bit-mask specifying from which sentences data has been obtained */
  nmeaTIME    utc;     /**< UTC of the position data                                        */
  int         sig;     /**< Signal quality, see NMEA_SIG_* defines                          */
  int         fix;     /**< Operating mode, see NMEA_FIX_* defines                          */
  double      PDOP;    /**< Position Dilution Of Precision                                  */
  double      HDOP;    /**< Horizontal Dilution Of Precision                                */
  double      VDOP;    /**< Vertical Dilution Of Precision                                  */
  double      lat;     /**< Latitude,  in NDEG: +/-[degree][min].[sec/60]                   */
  double      lon;     /**< Longitude, in NDEG: +/-[degree][min].[sec/60]                   */
  double      elv;     /**< Elevation above/below mean sea level (geoid), in meters         */
  double      height;  /**< Height of geoid (elv) above WGS84 ellipsoid, in meters          */
  double      speed;   /**< Speed over the ground in kph                                    */
  double      track;   /**< Track angle in degrees true north                               */
  double      mtrack;  /**< Magnetic Track angle in degrees true north                      */
  double      magvar;  /**< Magnetic variation degrees                                      */
  double      dgpsAge; /**< Time since last DGPS update, in seconds                         */
  int         dgpsSid; /**< DGPS station ID number                                          */
  nmeaSATINFO satinfo; /**< Satellites information */
} nmeaINFO;

/**
 * Enumeration for the fields names of a nmeaINFO structure.
 * The values are used in the 'present' bit-mask.
 */
typedef enum _nmeaINFO_FIELD {
  SMASK          = (1u << 0),  /* 0x00000001 */
  UTCDATE        = (1u << 1),  /* 0x00000002 */
  UTCTIME        = (1u << 2),  /* 0x00000004 */
  SIG            = (1u << 3),  /* 0x00000008 */

  FIX            = (1u << 4),  /* 0x00000010 */
  PDOP           = (1u << 5),  /* 0x00000020 */
  HDOP           = (1u << 6),  /* 0x00000040 */
  VDOP           = (1u << 7),  /* 0x00000080 */

  LAT            = (1u << 8),  /* 0x00000100 */
  LON            = (1u << 9),  /* 0x00000200 */
  ELV            = (1u << 10), /* 0x00000400 */
  SPEED          = (1u << 11), /* 0x00000800 */

  TRACK          = (1u << 12), /* 0x00001000 */
  MTRACK         = (1u << 13), /* 0x00002000 */
  MAGVAR         = (1u << 14), /* 0x00004000 */
  SATINUSECOUNT  = (1u << 15), /* 0x00008000 */

  SATINUSE       = (1u << 16), /* 0x00010000 */
  SATINVIEWCOUNT = (1u << 17), /* 0x00020000 */
  SATINVIEW      = (1u << 18), /* 0x00040000 */
  HEIGHT         = (1u << 19), /* 0x00080000 */

  DGPSAGE        = (1u << 20), /* 0x00100000 */
  DGPSSID        = (1u << 21), /* 0x00200000 */

  _nmeaINFO_FIELD_LAST = DGPSSID
} nmeaINFO_FIELD;

/** The bit-mask of all supported field name bits */
#define NMEA_INFO_PRESENT_MASK ((_nmeaINFO_FIELD_LAST << 1) - 1)

/**
 * Convert a nmeaINFO_FIELD into a string
 *
 * @param field The nmeaINFO_FIELD
 * @return The corresponding string, or NULL when the nmeaINFO_FIELD is
 * unknown
 */
const char * nmea_INFO_field_to_string(nmeaINFO_FIELD field);

/**
 * Determine if a 'present' bit-mask indicates presence of a certain
 * nmeaINFO_FIELD
 *
 * @param present The 'present' field
 * @param fieldName The nmeaINFO_FIELD to check for presence
 * @return True when the nmeaINFO_FIELD is present
 */
static INLINE bool nmea_INFO_is_present(uint32_t present, nmeaINFO_FIELD fieldName) {
  return ((present & fieldName) == fieldName);
}

/**
 * Adjust a 'present' bit-mask to indicate presence of a certain
 * nmeaINFO_FIELD
 *
 * @param present The 'present' field
 * @param fieldName The nmeaINFO_FIELD to indicate presence of
 */
static INLINE void nmea_INFO_set_present(uint32_t * present, nmeaINFO_FIELD fieldName) {
  if (present) {
    *present |= fieldName;
  }
}

/**
 * Adjust a 'present' bit-mask to indicate absence of a certain nmeaINFO_FIELD
 *
 * @param present The 'present' field
 * @param fieldName The nmeaINFO_FIELD to absence presence of
 */
static INLINE void nmea_INFO_unset_present(uint32_t * present, nmeaINFO_FIELD fieldName) {
  if (present) {
    *present &= ~fieldName;
  }
}

/**
 * Reset the time to now
 *
 * @param utc The time
 * @param present The 'present' field (when non-NULL then the UTCDATE and
 * UTCTIME flags are set in it)
 */
void nmea_time_now(nmeaTIME *utc, uint32_t * present);

/**
 * Clear an info structure.
 *
 * Resets the time to now, sets up the signal as NMEA_SIG_INVALID, the FIX as
 * NMEA_FIX_BAD, and signals presence of these fields.
 *
 * Resets all other fields to 0.
 *
 * @param info The info structure
 */
void nmea_zero_INFO(nmeaINFO *info);

/**
 * Sanitise the NMEA info, make sure that:
 * - sig is in the range [NMEA_SIG_FIRST, NMEA_SIG_LAST],
 *   if this is not the case then sig is set to NMEA_SIG_INVALID
 * - fix is in the range [NMEA_FIX_FIRST, NMEA_FIX_LAST],
 *   if this is not the case then fix is set to NMEA_FIX_BAD
 * - DOPs are positive,
 * - latitude is in the range [-9000, 9000],
 * - longitude is in the range [-18000, 18000],
 * - speed is positive,
 * - track is in the range [0, 360>.
 * - mtrack is in the range [0, 360>.
 * - magvar is in the range [0, 360>.
 * - satinfo:
 *   - inuse and in_use are consistent (w.r.t. count)
 *   - inview and sat are consistent (w.r.t. count/id)
 *   - in_use and sat are consistent (w.r.t. count/id)
 *   - elv is in the range [0, 90]
 *   - azimuth is in the range [0, 359]
 *   - sig is in the range [0, 99]
 *
 * Time is set to the current time when not present.
 * Fields are reset to their defaults (0) when not signalled as being present.
 *
 * @param nmeaInfo
 * the NMEA info structure to sanitise
 */
void nmea_INFO_sanitise(nmeaINFO *nmeaInfo);

/**
 * Converts the position fields to degrees and DOP fields to meters so that
 * all fields use normal metric units.
 *
 * @param nmeaInfo The info structure
 */
void nmea_INFO_unit_conversion(nmeaINFO * nmeaInfo);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_INFO_H__ */
