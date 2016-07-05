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

#ifndef __NMEALIB_GMATH_H__
#define __NMEALIB_GMATH_H__

#include <nmealib/info.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NMEALIB_TUD_YARDS              (1.0936133)                        /**< Yards, meter * NMEALIB_TUD_YARDS = yard */
#define NMEALIB_TUD_KNOTS              (1.852)                            /**< Knots, kilometer / NMEALIB_TUD_KNOTS = knot */
#define NMEALIB_TUD_MILES              (1.609344)                         /**< Miles, kilometer / NMEALIB_TUD_MILES = mile */
#define NMEALIB_TUS_MS                 (3.6)                              /**< Meters per seconds, (k/h) / NMEALIB_TUS_MS= (m/s) */
#define NMEALIB_PI                     (3.141592653589793)                /**< PI value */
#define NMEALIB_PI180                  (NMEALIB_PI / 180)                 /**< PI division by 180 */
#define NMEALIB_EARTHRADIUS_KM         (6378)                             /**< Earth's mean radius in km */
#define NMEALIB_EARTHRADIUS_M          (NMEALIB_EARTHRADIUS_KM * 1000)    /**< Earth's mean radius in m */
#define NMEALIB_EARTH_SEMIMAJORAXIS_M  (6378137.0)                        /**< Earth's semi-major axis in m according WGS84 */
#define NMEALIB_EARTH_SEMIMAJORAXIS_KM (NMEALIB_EARTHMAJORAXIS_KM / 1000) /**< Earth's semi-major axis in km according WGS 84 */
#define NMEALIB_EARTH_FLATTENING       (1 / 298.257223563)                /**< Earth's flattening according WGS 84 */
#define NMEALIB_DOP_FACTOR             (5)                                /**< Factor for translating DOP to meters */

/*
 * Degree and Radians
 */

/**
 * Convert degrees to radians
 *
 * @param val degrees
 * @return radians
 */
double nmeaDegreeToRadian(const double val);

/**
 * Convert radians to degrees
 *
 * @param val radians
 * @return degrees
 */
double nmeaRadianToDegree(const double val);

/*
 * NDEG (NMEA degree)
 */

/**
 * Convert NDEG (NMEA degrees) to decimal (fractional) degrees
 *
 * @param val NDEG (NMEA degrees)
 * @return decimal (fractional) degrees
 */
double nmeaNdegToDegree(const double val);

/**
 * Convert decimal (fractional) degrees to NDEG (NMEA degrees)
 *
 * @param val decimal (fractional) degrees
 * @return NDEG (NMEA degrees)
 */
double nmeaDegreeToNdeg(const double val);

/**
 * Convert NDEG (NMEA degrees) to radians
 *
 * @param val NDEG (NMEA degrees)
 * @return radians
 */
double nmeaNdegToRadian(const double val);

/**
 * Convert radians to NDEG (NMEA degrees)
 *
 * @param val radians
 * @return NDEG (NMEA degrees)
 */
double nmeaRadianToNdeg(const double val);

/*
 * DOP
 */

/**
 * Calculate PDOP (Position Dilution Of Precision) factor from HDOP and VDOP
 *
 * @param hdop HDOP
 * @param vdop VDOP
 * @return PDOP
 */
double nmeaPdopCalculate(const double hdop, const double vdop);

/**
 * Convert DOP to meters, using the NMEALIB_DOP_FACTOR factor
 *
 * @param dop the DOP
 * @return the DOP in meters
 */
double nmeaDopToMeters(const double dop);

/**
 * Convert DOP in meters to plain DOP, using the NMEALIB_DOP_FACTOR factor
 *
 * @param meters the DOP in meters
 * @return the plain DOP
 */
double nmeaMetersToDop(const double meters);

/*
 * Positions
 */

/**
 * Convert a position from INFO to radians position
 *
 * @param info a pointer to the INFO position
 * @param pos a pointer to the radians position (output)
 */
void nmeaInfoToPosition(const NmeaInfo *info, NmeaPosition *pos);

/**
 * Convert a radians position to a position from INFO
 *
 * @param pos a pointer to the radians position
 * @param info a pointer to the INFO position (output)
 */
void nmeaPositionToInfo(const NmeaPosition *pos, NmeaInfo *info);

/**
 * Calculate distance between two points
 *
 * @param from_pos a pointer to the from position (in radians)
 * @param to_pos a pointer to the to position (in radians)
 * @return distance in meters
 */
double nmeaDistance(const NmeaPosition *from_pos, const NmeaPosition *to_pos);

/**
 * Calculate the distance between two points.
 * This function uses an algorithm for an oblate spheroid earth model.
 * The algorithm is described here:
 * http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
 *
 * @param from_pos a pointer to the from position (in radians)
 * @param to_pos a pointer to the to position (in radians)
 * @param from_azimuth a pointer to the azimuth at "from" position (in radians) (output)
 * @param to_azimuth a pointer to the azimuth at "to" position (in radians) (output)
 * @return distance in meters
 */
double nmeaDistanceEllipsoid(const NmeaPosition *from_pos, const NmeaPosition *to_pos, double *from_azimuth,
    double *to_azimuth);

/**
 * Perform a flat (horizontal) move.
 *
 * @param start_pos a pointer to the start position (in radians)
 * @param end_pos a pointer to the end position (in radians) (output)
 * @param azimuth azimuth (in degrees, [0, 359])
 * @param distance the distance (in km)
 * @return 1 (true) on success, 0 (false) on failure
 */
int nmeaMoveFlat(const NmeaPosition *start_pos, NmeaPosition *end_pos, double azimuth, double distance);

/**
 * Perform a flat (horizontal) move against the ellipsoid.
 *
 * This function uses an algorithm for an oblate spheroid earth model.
 * The algorithm is described here:
 * http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
 *
 * @param start_pos a pointer to the start position (in radians)
 * @param end_pos a pointer to the end position (in radians) (output)
 * @param azimuth azimuth (in radians)
 * @param distance the distance (in km)
 * @param end_azimuth azimuth at end position (in radians) (output)
 * @return 1 (true) on success, 0 (false) on failure
 */
int nmeaMoveFlatEllipsoid(const NmeaPosition *start_pos, NmeaPosition *end_pos, double azimuth, double distance,
    double *end_azimuth);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_GMATH_H__ */
