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

#ifndef __NMEALIB_CONVERSIONS_H__
#define __NMEALIB_CONVERSIONS_H__

#include <nmea/sentence.h>
#include <nmea/info.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Determine the number of GSV sentences needed for the specified number of
 * satellites
 *
 * @param satellites The number of satellites
 * @return The number of GSV sentences needed (at least 1)
 */
unsigned int nmea_gsv_npack(unsigned int satellites);

/**
 * Update an unsanitised nmeaINFO structure from a GSV packet structure
 *
 * @param pack The GSV packet structure
 * @param info The nmeaINFO structure
 */
void nmea_GPGSV2info(const nmeaGPGSV *pack, nmeaINFO *info);

/**
 * Convert a sanitised nmeaINFO structure into a nmeaGPGSV structure
 *
 * @param info The nmeaINFO structure
 * @param pack The nmeaGPGSV structure
 * @param pack_idx The index of the nmeaGPGSV structure (zero based)
 */
void nmea_info2GPGSV(const nmeaINFO *info, nmeaGPGSV *pack, unsigned int pack_idx);

/**
 * Update an unsanitised nmeaINFO structure from a RMC packet structure
 *
 * @param pack The RMC packet structure
 * @param info The nmeaINFO structure
 */
void nmea_GPRMC2info(const nmeaGPRMC *pack, nmeaINFO *info);

/**
 * Convert a sanitised nmeaINFO structure into a nmeaGPRMC structure
 *
 * @param info The nmeaINFO structure
 * @param pack The nmeaGPRMC structure
 */
void nmea_info2GPRMC(const nmeaINFO *info, nmeaGPRMC *pack);

/**
 * Update an unsanitised nmeaINFO structure from a VTG packet structure
 *
 * @param pack The VTG packet structure
 * @param info The nmeaINFO structure
 */
void nmea_GPVTG2info(const nmeaGPVTG *pack, nmeaINFO *info);

/**
 * Convert a sanitised nmeaINFO structure into a nmeaGPVTG structure
 *
 * @param info The nmeaINFO structure
 * @param pack The nmeaGPVTG structure
 */
void nmea_info2GPVTG(const nmeaINFO *info, nmeaGPVTG *pack);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_CONVERSIONS_H__ */
