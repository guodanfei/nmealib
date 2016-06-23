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

#ifndef __NMEALIB_GPGSV_H__
#define __NMEALIB_GPGSV_H__

#include <nmealib/info.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * GPGSV packet information structure (Satellites in view)
 *
 * <pre>
 * $GPGSV,sentences,sentence,satellites,prn1,elevation1,azimuth1,snr1,prn2,elevation2,azimuth2,snr2,prn3,elevation3,azimuth3,snr3,prn4,elevation4,azimuth4,snr4*checksum
 * </pre>
 *
 * | Field       | Description                                      | present    |
 * | :---------: | ------------------------------------------------ | :--------: |
 * | $GPGSV      | NMEA prefix                                      | -          |
 * | sentences   | The number of sentences for full data            | SATINVIEW  |
 * | sentence    | The current sentence number                      | SATINVIEW  |
 * | satellites  | The number of satellites in view                 | SATINVIEW  |
 * | prn1        | Satellite PRN number                             | SATINVIEW  |
 * | elevation1  | Elevation, in degrees                            | SATINVIEW  |
 * | azimuth1    | Azimuth, in degrees                              | SATINVIEW  |
 * | snr1        | Signal-Noise-Ration, in dB                       | SATINVIEW  |
 * | prn2        | Satellite PRN number                             | SATINVIEW  |
 * | elevation2  | Elevation, in degrees                            | SATINVIEW  |
 * | azimuth2    | Azimuth, in degrees                              | SATINVIEW  |
 * | snr2        | Signal-Noise-Ration, in dB                       | SATINVIEW  |
 * | prn3        | Satellite PRN number                             | SATINVIEW  |
 * | elevation3  | Elevation, in degrees                            | SATINVIEW  |
 * | azimuth3    | Azimuth, in degrees                              | SATINVIEW  |
 * | snr3        | Signal-Noise-Ration, in dB                       | SATINVIEW  |
 * | prn4        | Satellite PRN number                             | SATINVIEW  |
 * | elevation4  | Elevation, in degrees                            | SATINVIEW  |
 * | azimuth4    | Azimuth, in degrees                              | SATINVIEW  |
 * | snr4        | Signal-Noise-Ration, in dB                       | SATINVIEW  |
 * | checksum    | NMEA checksum                                    | -          |
 *
 * Shows data about the satellites that the unit might be able to find based on
 * its viewing mask and almanac data. It also shows current ability to track
 * this data. Note that one GPGSV sentence only can provide data for up to 4
 * satellites and thus there may need to be 3 sentences for the full
 * information. It is reasonable for the GPGSV sentence to contain more satellites
 * than GPGGA might indicate since GPGSV may include satellites that are not used as
 * part of the solution. It is not a requirement that the GPGSV sentences all
 * appear in sequence. To avoid overloading the data bandwidth some receivers
 * may place the various sentences in totally different samples since each
 * sentence identifies which one it is.
 *
 * The field called SNR (Signal to Noise Ratio) in the NMEA standard is often
 * referred to as signal strength. SNR is an indirect but more useful value than
 * raw signal strength. It can range from 0 to 99 and has units of dB according
 * to the NMEA standard, but the various manufacturers send different ranges of
 * numbers with different starting numbers so the values themselves cannot
 * necessarily be used to evaluate different units. The range of working values
 * in a given gps will usually show a difference of about 25 to 35 between the
 * lowest and highest values, however 0 is a special case and may be shown on
 * satellites that are in view but not being tracked.
 *
 * Example:
 *
 * <pre>
 * $GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75
 * </pre>
 */
typedef struct _nmeaGPGSV {
  uint32_t      present;
  int           sentences;
  int           sentence;
  int           satellites;
  nmeaSATELLITE satellite[NMEA_SATINPACK];
} nmeaGPGSV;

/**
 * Determine the number of GPGSV sentences needed for the specified number of
 * satellites
 *
 * @param satellites The number of satellites
 * @return The number of GPGSV sentences needed (at least 1)
 */
unsigned int nmeaGPGSVsatellitesToSentencesCount(unsigned int satellites);

/**
 * Parse a GPGSV sentence from a string
 *
 * @param s The string
 * @param sz The length of the string
 * @param pack Where the results should be stored
 * @return True if parsed successfully, or false otherwise
 */
bool nmeaGPGSVparse(const char *s, const size_t sz, nmeaGPGSV *pack);

/**
 * Update an unsanitised nmeaINFO structure from a GPGSV packet structure
 *
 * @param pack The GPGSV packet structure
 * @param info The nmeaINFO structure
 */
void nmeaGPGSVToInfo(const nmeaGPGSV *pack, nmeaINFO *info);

/**
 * Convert a sanitised nmeaINFO structure into a nmeaGPGSV structure
 *
 * @param info The nmeaINFO structure
 * @param pack The nmeaGPGSV structure
 * @param pack_idx The index of the nmeaGPGSV structure (zero based)
 */
void nmeaGPGSVFromInfo(const nmeaINFO *info, nmeaGPGSV *pack, unsigned int pack_idx);

/**
 * Generate a GPGSV sentence from an nmeaGPGSV structure
 *
 * @param s a pointer to the buffer to generate the string in
 * @param len the size of the buffer
 * @param pack the structure
 * @return the length of the generated sentence
 */
int nmea_gen_GPGSV(char *s, const int len, const nmeaGPGSV *pack);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_GPGSV_H__ */
