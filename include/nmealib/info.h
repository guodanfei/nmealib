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
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * SIG
 */

/**
 * Enumeration for the signal names.
 * The values are used in the 'sig' field.
 */
typedef enum _NmeaSignal {
  NMEALIB_SIG_INVALID      = 0u,
  NMEALIB_SIG_FIRST        = NMEALIB_SIG_INVALID,
  NMEALIB_SIG_FIX          = 1u,
  NMEALIB_SIG_DIFFERENTIAL = 2u,
  NMEALIB_SIG_SENSITIVE    = 3u,
  NMEALIB_SIG_RTKIN        = 4u,
  NMEALIB_SIG_FLOAT_RTK    = 5u,
  NMEALIB_SIG_ESTIMATED    = 6u,
  NMEALIB_SIG_MANUAL       = 7u,
  NMEALIB_SIG_SIMULATION   = 8u,
  NMEALIB_SIG_LAST         = NMEALIB_SIG_SIMULATION
} NmeaSignal;

/**
 * Convert a NMEALIB_SIG_* define into a string
 *
 * @param sig The NMEALIB_SIG_* define
 * @return The corresponding string, or NULL when the define is unknown
 */
const char * nmeaInfoSigToString(NmeaSignal sig);

/**
 * Convert a mode character into the corresponding NMEALIB_SIG_* define
 *
 * @param mode The mode character
 * @return The corresponding NMEALIB_SIG_* define, or NMEALIB_SIG_INVALID when the
 * mode is unknown
 */
NmeaSignal nmeaInfoModeToSig(char mode);

/**
 * Convert a NMEALIB_SIG_* define into the corresponding mode character
 *
 * @param sig The NMEALIB_SIG_* define
 * @return The corresponding mode character, or 'N' when the NMEALIB_SIG_* define
 * is unknown
 */
char nmeaInfoSigToMode(NmeaSignal sig);

/*
 * FIX
 */

typedef enum _NmeaFix {
  NMEALIB_FIX_BAD   = 1u,
  NMEALIB_FIX_FIRST = NMEALIB_FIX_BAD,
  NMEALIB_FIX_2D    = 2u,
  NMEALIB_FIX_3D    = 3u,
  NMEALIB_FIX_LAST  = NMEALIB_FIX_3D
} NmeaFix;

/**
 * Convert a NMEALIB_FIX_* define into a string
 *
 * @param fix The NMEALIB_FIX_* define
 * @return The corresponding string, or NULL when the NMEALIB_FIX_* define is
 * unknown
 */
const char * nmeaInfoFixToString(NmeaFix fix);

/*
 * Limits and defaults
 */

/** The maximum number of satellites, must be a multiple of NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE */
#define NMEALIB_MAX_SATELLITES              (72u)

/** The default latitude */
#define NMEALIB_LATITUDE_DEFAULT_NDEG       (0.0)

/** The default longitude */
#define NMEALIB_LONGITUDE_DEFAULT_NDEG      (0.0)

/**
 * Date and time data
 */
typedef struct _NmeaTime {
  unsigned int year; /**< Years                    - [1900, 2089]                 */
  unsigned int mon;  /**< Months                   - [   1,   12]                 */
  unsigned int day;  /**< Day of the month         - [   1,   31]                 */
  unsigned int hour; /**< Hours since midnight     - [   0,   23]                 */
  unsigned int min;  /**< Minutes after the hour   - [   0,   59]                 */
  unsigned int sec;  /**< Seconds after the minute - [   0,   60] (1 leap second) */
  unsigned int hsec; /**< Hundredth part of second - [   0,   99]                 */
} NmeaTime;

/**
 * Parse a NMEA time into a nmeaTIME structure (time only, no date).
 *
 * The format that is used (HHMMSS, HHMMSS.t, HHMMSS.hh or HHMMSS.mmm) is
 * determined by the length of the string.
 *
 * @param s The time
 * @param time The structure in which to store the parsed time
 * @return True on success
 */
bool nmeaTimeParseTime(const char *s, NmeaTime *time);

/**
 * Parse a NMEA date into a nmeaTIME structure (date only, no time).
 *
 * The month is adjusted (decremented by 1) to comply with the nmeaTIME month
 * range of [0, 11]. The year is adjusted (incremented by 100) for years
 * before 90 to comply with the nmeaTIME year range of [90, 189].
 *
 * @param s The date (DDMMYY)
 * @param date The structure in which to store the parsed date
 * @return True on success
 */
bool nmeaTimeParseDate(const char *s, NmeaTime *date);

/**
 * Position data in fractional degrees or radians
 */
typedef struct _NmeaPosition {
  double lat; /**< Latitude  */
  double lon; /**< Longitude */
} NmeaPosition;

/**
 * Information about satellite
 */
typedef struct _NmeaSatellite {
  unsigned int prn;       /**< Satellite PRN number             - [1, inf) */
  int          elevation; /**< Elevation, in degrees            - [0,  90] */
  unsigned int azimuth;   /**< Azimuth, degrees from true north - [0, 359] */
  unsigned int snr;       /**< Signal-to-Noise-Ratio            - [0,  99] */
} NmeaSatellite;

/**
 * Information about all tracked satellites
 */
typedef struct _NmeaSatellites {
  unsigned int  inUseCount;                     /**< The number of satellites in use (not those in view) */
  unsigned int  inUse[NMEALIB_MAX_SATELLITES];  /**< The PRNs of satellites in use   (not those in view) */
  unsigned int  inViewCount;                    /**< The number of satellites in view                    */
  NmeaSatellite inView[NMEALIB_MAX_SATELLITES]; /**< Satellites information (in view)                    */
} NmeaSatellites;

/**
 * Information about progress on non-atomic sentences
 */
typedef struct _NmeaProgress {
  bool gpgsvInProgress; /**< true when gpgsv is in progress */
} NmeaProgress;

/**
 * GPS information from all supported sentences, used also for generating NMEA sentences
 */
typedef struct _NmeaInfo {
  uint32_t       present;  /**< Bit-mask specifying which fields are present                    */
  uint32_t       smask;    /**< Bit-mask specifying from which sentences data has been obtained */
  NmeaTime       utc;      /**< UTC of the position data                                        */
  NmeaSignal     sig;      /**< Signal quality, see NMEALIB_SIG_* defines                       */
  NmeaFix        fix;      /**< Operating mode, see NMEALIB_FIX_* defines                       */
  double         pdop;     /**< Position Dilution Of Precision                                  */
  double         hdop;     /**< Horizontal Dilution Of Precision                                */
  double         vdop;     /**< Vertical Dilution Of Precision                                  */
  double         lat;      /**< Latitude,  in NDEG: +/-[degree][min].[sec/60]                   */
  double         lon;      /**< Longitude, in NDEG: +/-[degree][min].[sec/60]                   */
  double         elv;      /**< Elevation above/below mean sea level (geoid), in meters         */
  double         height;   /**< Height of geoid (elv) above WGS84 ellipsoid, in meters          */
  double         speed;    /**< Speed over the ground in kph                                    */
  double         track;    /**< Track angle in degrees true north                               */
  double         mtrack;   /**< Magnetic Track angle in degrees true north                      */
  double         magvar;   /**< Magnetic variation degrees                                      */
  double         dgpsAge;  /**< Time since last DGPS update, in seconds                         */
  unsigned int   dgpsSid;  /**< DGPS station ID number                                          */
  NmeaSatellites satinfo;  /**< Satellites information                                          */
  NmeaProgress   progress; /**< Progress information                                            */
  bool           metric;   /**< When true then units are metric                                 */
} NmeaInfo;

/**
 * Enumeration for the fields names of a nmeaINFO structure.
 * The values are used in the 'present' bit-mask.
 */
typedef enum _NmeaPresence {
  NMEALIB_PRESENT_SMASK          = (1u << 0),  /* 0x00000001 */
  NMEALIB_PRESENT_FIRST          = NMEALIB_PRESENT_SMASK,
  NMEALIB_PRESENT_UTCDATE        = (1u << 1),  /* 0x00000002 */
  NMEALIB_PRESENT_UTCTIME        = (1u << 2),  /* 0x00000004 */
  NMEALIB_PRESENT_SIG            = (1u << 3),  /* 0x00000008 */

  NMEALIB_PRESENT_FIX            = (1u << 4),  /* 0x00000010 */
  NMEALIB_PRESENT_PDOP           = (1u << 5),  /* 0x00000020 */
  NMEALIB_PRESENT_HDOP           = (1u << 6),  /* 0x00000040 */
  NMEALIB_PRESENT_VDOP           = (1u << 7),  /* 0x00000080 */

  NMEALIB_PRESENT_LAT            = (1u << 8),  /* 0x00000100 */
  NMEALIB_PRESENT_LON            = (1u << 9),  /* 0x00000200 */
  NMEALIB_PRESENT_ELV            = (1u << 10), /* 0x00000400 */
  NMEALIB_PRESENT_SPEED          = (1u << 11), /* 0x00000800 */

  NMEALIB_PRESENT_TRACK          = (1u << 12), /* 0x00001000 */
  NMEALIB_PRESENT_MTRACK         = (1u << 13), /* 0x00002000 */
  NMEALIB_PRESENT_MAGVAR         = (1u << 14), /* 0x00004000 */
  NMEALIB_PRESENT_SATINUSECOUNT  = (1u << 15), /* 0x00008000 */

  NMEALIB_PRESENT_SATINUSE       = (1u << 16), /* 0x00010000 */
  NMEALIB_PRESENT_SATINVIEWCOUNT = (1u << 17), /* 0x00020000 */
  NMEALIB_PRESENT_SATINVIEW      = (1u << 18), /* 0x00040000 */
  NMEALIB_PRESENT_HEIGHT         = (1u << 19), /* 0x00080000 */

  NMEALIB_PRESENT_DGPSAGE        = (1u << 20), /* 0x00100000 */
  NMEALIB_PRESENT_DGPSSID        = (1u << 21), /* 0x00200000 */

  NMEALIB_PRESENT_LAST           = NMEALIB_PRESENT_DGPSSID
} NmeaPresence;

/** The bit-mask of all supported field name bits */
#define NMEALIB_INFO_PRESENT_MASK ((NMEALIB_PRESENT_LAST << 1) - 1)

/**
 * Convert a nmeaINFO_FIELD into a string
 *
 * @param field The nmeaINFO_FIELD
 * @return The corresponding string, or NULL when the nmeaINFO_FIELD is
 * unknown
 */
const char * nmeaInfoFieldToString(NmeaPresence field);

/**
 * Determine if a 'present' bit-mask indicates presence of a certain
 * nmeaINFO_FIELD
 *
 * @param present The 'present' field
 * @param fieldName The nmeaINFO_FIELD to check for presence
 * @return True when the nmeaINFO_FIELD is present
 */
static INLINE bool nmeaInfoIsPresentAll(uint32_t present, NmeaPresence fieldName) {
  return ((present & fieldName) == fieldName);
}

/**
 * Determine if a 'present' bit-mask indicates presence of any of the
 * indicated nmeaINFO_FIELD field names
 *
 * @param present The 'present' field
 * @param fieldName The nmeaINFO_FIELD bit-mask to check for presence
 * @return True when any of the nmeaINFO_FIELD field names is present
 */
static INLINE bool nmeaInfoIsPresentAny(uint32_t present, NmeaPresence fieldName) {
  return ((present & fieldName) != 0);
}

/**
 * Adjust a 'present' bit-mask to indicate presence of a certain
 * nmeaINFO_FIELD
 *
 * @param present The 'present' field
 * @param fieldName The nmeaINFO_FIELD to indicate presence of
 */
static INLINE void nmeaInfoSetPresent(uint32_t * present, NmeaPresence fieldName) {
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
static INLINE void nmeaInfoUnsetPresent(uint32_t * present, NmeaPresence fieldName) {
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
 * @param timeval If non-NULL then use this provided time, otherwise use 'gettimeofday'
 */
void nmeaTimeSet(NmeaTime *utc, uint32_t * present, struct timeval *timeval);

/**
 * Clear an info structure.
 *
 * Sets up the signal as NMEALIB_SIG_INVALID, the FIX as
 * NMEALIB_FIX_BAD, and signals presence of these fields.
 *
 * Resets all other fields to 0.
 *
 * @param info The info structure
 */
void nmeaInfoClear(NmeaInfo *info);

/**
 * Sanitise the NMEA info, make sure that:
 * - all information is in the original units
 * - utc date is set to the current date when not present
 * - utc time is set to the current time when not present
 * - sig is in the range [NMEALIB_SIG_FIRST, NMEALIB_SIG_LAST],
 *   if this is not the case then sig is set to NMEALIB_SIG_INVALID
 * - fix is in the range [NMEALIB_FIX_FIRST, NMEALIB_FIX_LAST],
 *   if this is not the case then fix is set to NMEALIB_FIX_BAD
 * - DOPs are positive
 * - latitude is in the range [-9000, 9000]
 * - longitude is in the range [-18000, 18000]
 * - speed is positive
 * - track is in the range [0, 360>
 * - mtrack is in the range [0, 360>
 * - magvar is in the range [0, 360>
 * - dgpsAge is positive
 * - dgpsSid is positive
 * - satinfo:
 *   - inuseCount is positive
 *   - each inUse satellite has a positive PRN
 *   - inUse satellites are compacted (satellites with a zero PRN last)
 *   - inViewCount is positive
 *   - inView (only when GPGSV is not 'in progress'):
 *     - prn is positive
 *     - elevation is in the range [-90, 90]
 *     - azimuth is in the range [0, 359]
 *     - snr is in the range [0, 99]
 *
 * Fields are reset to their defaults (0) when not signalled as being present.
 *
 * @param info The NMEA info structure to sanitise
 */
void nmeaInfoSanitise(NmeaInfo *info);

/**
 * Converts the position fields to degrees and DOP fields to meters so that
 * all fields use normal metric units or original units.
 *
 * @param info The nmeaINFO
 * @param toMetric Convert to metric units (from original units) when true,
 * convert to original units (from metric units) when false
 */
void nmeaInfoUnitConversion(NmeaInfo * info, bool toMetric);

/**
 * Compare 2 satellite PRNs and put zeroes last (consider those to be 1000)
 *
 * @param p1 The first satellite PRN
 * @param p2 The second satellite PRN
 * @return 0 when both are equal, a negative value when PRN1 < PRN2, a
 * positive value otherwise
 */
int nmeaQsortPRNCompare(const void *p1, const void *p2);

/**
 * Compact 2 satellite PRNs (do not reorder) and put zeroes last (consider
 * those to be 1000)
 *
 * @param p1 The first satellite PRN
 * @param p2 The second satellite PRN
 * @return 0 when both are non-zero or are equal, a negative value when
 * PRN1 < PRN2, a positive value otherwise
 */
int nmeaQsortPRNCompact(const void *p1, const void *p2);

/**
 * Compare 2 satellite PRNs and put zeroes last (consider those to be 1000)
 *
 * @param s1 The first satellite
 * @param s2 The second satellite
 * @return 0 when both are equal, a negative value when PRN1 < PRN2, a
 * positive value otherwise
 */
int nmeaQsortSatelliteCompare(const void *s1, const void *s2);

/**
 * Compact 2 satellite PRNs (do not reorder) and put zeroes last (consider
 * those to be 1000)
 *
 * @param s1 The first satellite
 * @param s2 The second satellite
 * @return 0 when both are non-zero or are equal, a negative value when
 * PRN1 < PRN2, a positive value otherwise
 */
int nmeaQsortSatelliteCompact(const void *s1, const void *s2);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_INFO_H__ */
