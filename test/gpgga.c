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

#include "testHelpers.h"

#include <nmealib/context.h>
#include <nmealib/gpgga.h>
#include <nmealib/info.h>
#include <nmealib/sentence.h>

#include <CUnit/Basic.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

int gpggaSuiteSetup(void);

/*
 * Tests
 */

static void test_nmeaGPGGAParse(void) {
  const char * s = "some string";
  nmeaGPGGA packEmpty;
  nmeaGPGGA pack;
  bool r;

  memset(&packEmpty, 0, sizeof(packEmpty));
  memset(&pack, 0, sizeof(pack));

  /* invalid inputs */

  r = nmeaGPGGAParse(NULL, 1, &pack);
  validateParsePack(&pack, r, false, 0, 0, true);

  r = nmeaGPGGAParse(s, 0, &pack);
  validateParsePack(&pack, r, false, 0, 0, true);

  r = nmeaGPGGAParse(s, strlen(s), NULL);
  validateParsePack(&pack, r, false, 0, 0, true);

  /* invalid sentence / not enough fields */

  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  /* all fields empty */

  s = "$GPGGA,,,,,,,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, true);

  /* time */

  s = "$GPGGA,invalid,,,,,,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 0, true);

  s = "$GPGGA,999999,,,,,,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,104559.64,,,,,,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, UTCTIME);
  CU_ASSERT_EQUAL(pack.time.hour, 10);
  CU_ASSERT_EQUAL(pack.time.min, 45);
  CU_ASSERT_EQUAL(pack.time.sec, 59);
  CU_ASSERT_EQUAL(pack.time.hsec, 64);

  /* lat */

  s = "$GPGGA,,1,,,,,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,1,!,,,,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,-1242.55,s,,,,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, LAT);
  CU_ASSERT_EQUAL(pack.latitude, 1242.55);
  CU_ASSERT_EQUAL(pack.ns, 'S');

  /* lon */

  s = "$GPGGA,,,,1,,,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,1,!,,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,-1242.55,e,,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, LON);
  CU_ASSERT_EQUAL(pack.longitude, 1242.55);
  CU_ASSERT_EQUAL(pack.ew, 'E');

  /* sig */

  s = "$GPGGA,,,,,,4242,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,,,2,,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, SIG);
  CU_ASSERT_EQUAL(pack.signal, 2);

  /* satellites */

  s = "$GPGGA,,,,,,,-8,,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, SATINVIEWCOUNT);
  CU_ASSERT_EQUAL(pack.satellitesInView, 8);

  /* hdop */

  s = "$GPGGA,,,,,,,,-12.128,,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, HDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.hdop, 12.128, DBL_EPSILON);

  /* elv */

  s = "$GPGGA,,,,,,,,,1,,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,,,,,,1,!,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,,,,,,-42,m,,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, ELV);
  CU_ASSERT_DOUBLE_EQUAL(pack.elevation, -42, DBL_EPSILON);
  CU_ASSERT_EQUAL(pack.elevationUnit, 'M');

  /* diff */

  s = "$GPGGA,,,,,,,,,,,1,,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,,,,,,,,1,!,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,,,,,,,,-42,m,,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, HEIGHT);
  CU_ASSERT_DOUBLE_EQUAL(pack.height, -42, DBL_EPSILON);
  CU_ASSERT_EQUAL(pack.heightUnit, 'M');

  /* dgpsAge */

  s = "$GPGGA,,,,,,,,,,,,,-1.250,*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, DGPSAGE);
  CU_ASSERT_DOUBLE_EQUAL(pack.dgpsAge, 1.250, DBL_EPSILON);

  /* dgpsSid */

  s = "$GPGGA,,,,,,,,,,,,,,-42*";
  r = nmeaGPGGAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, DGPSSID);
  CU_ASSERT_EQUAL(pack.dgpsSid, 42);
}

static void test_nmeaGPGGAToInfo(void) {
  nmeaGPGGA pack;
  nmeaINFO infoEmpty;
  nmeaINFO info;

  memset(&pack, 0, sizeof(pack));
  memset(&infoEmpty, 0, sizeof(infoEmpty));
  memset(&info, 0, sizeof(info));

  /* invalid inputs */

  nmeaGPGGAToInfo(NULL, &info);
  validatePackToInfo(&info, 0, 0, true);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  nmeaGPGGAToInfo(&pack, NULL);
  validatePackToInfo(&info, 0, 0, true);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* empty */

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* time */

  pack.time.hour = 12;
  pack.time.min = 42;
  pack.time.sec = 43;
  pack.time.hsec = 44;
  nmea_INFO_set_present(&pack.present, UTCTIME);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | UTCTIME);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_EQUAL(info.utc.hour, 12);
  CU_ASSERT_EQUAL(info.utc.min, 42);
  CU_ASSERT_EQUAL(info.utc.sec, 43);
  CU_ASSERT_EQUAL(info.utc.hsec, 44);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* latitude  */

  pack.latitude = -1232.5523;
  pack.ns = 'N';
  nmea_INFO_set_present(&pack.present, LAT);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | LAT);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.lat, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.latitude = -1232.5523;
  pack.ns = 'S';
  nmea_INFO_set_present(&pack.present, LAT);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | LAT);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.lat, -1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.latitude = 1232.5523;
  pack.ns = 'N';
  nmea_INFO_set_present(&pack.present, LAT);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | LAT);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.lat, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.latitude = 1232.5523;
  pack.ns = 'S';
  nmea_INFO_set_present(&pack.present, LAT);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | LAT);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.lat, -1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* longitude */

  pack.longitude = -1232.5523;
  pack.ew = 'E';
  nmea_INFO_set_present(&pack.present, LON);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | LON);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.lon, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.longitude = -1232.5523;
  pack.ew = 'W';
  nmea_INFO_set_present(&pack.present, LON);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | LON);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.lon, -1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.longitude = 1232.5523;
  pack.ew = 'E';
  nmea_INFO_set_present(&pack.present, LON);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | LON);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.lon, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.longitude = 1232.5523;
  pack.ew = 'W';
  nmea_INFO_set_present(&pack.present, LON);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | LON);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.lon, -1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* signal */

  pack.signal = NMEA_SIG_FLOAT_RTK;
  nmea_INFO_set_present(&pack.present, SIG);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | SIG);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_EQUAL(info.sig, NMEA_SIG_FLOAT_RTK);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* satellites */

  pack.satellitesInView = 42;
  nmea_INFO_set_present(&pack.present, SATINVIEWCOUNT);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | SATINVIEWCOUNT);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_EQUAL(info.satinfo.inview, 42);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* hdop */

  pack.hdop = -1232.5523;
  nmea_INFO_set_present(&pack.present, HDOP);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | HDOP);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.HDOP, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.hdop = 1232.5523;
  nmea_INFO_set_present(&pack.present, HDOP);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | HDOP);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.HDOP, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* elv */

  pack.elevation = -1232.5523;
  nmea_INFO_set_present(&pack.present, ELV);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | ELV);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.elv, -1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.elevation = 1232.5523;
  nmea_INFO_set_present(&pack.present, ELV);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | ELV);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.elv, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* height */

  pack.height = -1232.5523;
  nmea_INFO_set_present(&pack.present, HEIGHT);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | HEIGHT);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.height, -1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.height = 1232.5523;
  nmea_INFO_set_present(&pack.present, HEIGHT);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | HEIGHT);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.height, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* dgpsAge */

  pack.dgpsAge = -1232.5523;
  nmea_INFO_set_present(&pack.present, DGPSAGE);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | DGPSAGE);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.dgpsAge, -1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.dgpsAge = 1232.5523;
  nmea_INFO_set_present(&pack.present, DGPSAGE);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | DGPSAGE);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_DOUBLE_EQUAL(info.dgpsAge, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* dgpsSid */

  pack.dgpsSid = 42;
  nmea_INFO_set_present(&pack.present, DGPSSID);

  nmeaGPGGAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | DGPSSID);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_EQUAL(info.dgpsSid, 42);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));
}

static void test_nmeaGPGGAFromInfo(void) {
  nmeaINFO info;
  nmeaGPGGA packEmpty;
  nmeaGPGGA pack;

  memset(&info, 0, sizeof(info));
  memset(&packEmpty, 0, sizeof(packEmpty));
  memset(&pack, 0, sizeof(pack));

  /* invalid inputs */

  nmeaGPGGAFromInfo(NULL, &pack);
  validateInfoToPack(&pack, 0, 0, true);
  memset(&info, 0, sizeof(info));

  nmeaGPGGAFromInfo(&info, NULL);
  validateInfoToPack(&pack, 0, 0, true);
  memset(&info, 0, sizeof(info));

  /* empty */

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, true);
  memset(&info, 0, sizeof(info));

  /* time */

  info.utc.hour = 12;
  info.utc.min = 42;
  info.utc.sec = 43;
  info.utc.hsec = 44;
  nmea_INFO_set_present(&info.present, UTCTIME);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, UTCTIME);
  CU_ASSERT_EQUAL(pack.time.hour, 12);
  CU_ASSERT_EQUAL(pack.time.min, 42);
  CU_ASSERT_EQUAL(pack.time.sec, 43);
  CU_ASSERT_EQUAL(pack.time.hsec, 44);
  memset(&info, 0, sizeof(info));

  /* latitude  */

  info.lat = -1232.5523;
  nmea_INFO_set_present(&info.present, LAT);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, LAT);
  CU_ASSERT_DOUBLE_EQUAL(pack.latitude, 1232.5523, DBL_EPSILON);
  CU_ASSERT_EQUAL(pack.ns, 'S');
  memset(&info, 0, sizeof(info));

  info.lat = 1232.5523;
  nmea_INFO_set_present(&info.present, LAT);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, LAT);
  CU_ASSERT_DOUBLE_EQUAL(pack.latitude, 1232.5523, DBL_EPSILON);
  CU_ASSERT_EQUAL(pack.ns, 'N');
  memset(&info, 0, sizeof(info));

  /* longitude */

  info.lon = -1232.5523;
  nmea_INFO_set_present(&info.present, LON);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, LON);
  CU_ASSERT_DOUBLE_EQUAL(pack.longitude, 1232.5523, DBL_EPSILON);
  CU_ASSERT_EQUAL(pack.ew, 'W');
  memset(&info, 0, sizeof(info));

  info.lon = 1232.5523;
  nmea_INFO_set_present(&info.present, LON);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, LON);
  CU_ASSERT_DOUBLE_EQUAL(pack.longitude, 1232.5523, DBL_EPSILON);
  CU_ASSERT_EQUAL(pack.ew, 'E');
  memset(&info, 0, sizeof(info));

  /* signal */

  info.sig = NMEA_SIG_MANUAL;
  nmea_INFO_set_present(&info.present, SIG);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, SIG);
  CU_ASSERT_EQUAL(pack.signal, NMEA_SIG_MANUAL);
  memset(&info, 0, sizeof(info));

  /* satellites */

  info.satinfo.inview = 42;
  nmea_INFO_set_present(&info.present, SATINVIEWCOUNT);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, SATINVIEWCOUNT);
  CU_ASSERT_EQUAL(pack.satellitesInView, 42);
  memset(&info, 0, sizeof(info));

  /* hdop */

  info.HDOP = 1232.5523;
  nmea_INFO_set_present(&info.present, HDOP);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, HDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.hdop, 1232.5523, DBL_EPSILON);
  memset(&info, 0, sizeof(info));

  /* elv */

  info.elv = -1232.5523;
  nmea_INFO_set_present(&info.present, ELV);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, ELV);
  CU_ASSERT_DOUBLE_EQUAL(pack.elevation, -1232.5523, DBL_EPSILON);
  CU_ASSERT_EQUAL(pack.elevationUnit, 'M');
  memset(&info, 0, sizeof(info));

  /* height */

  info.height = -1232.5523;
  nmea_INFO_set_present(&info.present, HEIGHT);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, HEIGHT);
  CU_ASSERT_DOUBLE_EQUAL(pack.height, -1232.5523, DBL_EPSILON);
  CU_ASSERT_EQUAL(pack.heightUnit, 'M');
  memset(&info, 0, sizeof(info));

  /* dgpsAge */

  info.dgpsAge = 1232.5523;
  nmea_INFO_set_present(&info.present, DGPSAGE);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, DGPSAGE);
  CU_ASSERT_DOUBLE_EQUAL(pack.dgpsAge, 1232.5523, DBL_EPSILON);
  memset(&info, 0, sizeof(info));

  /* dgpsSid */

  info.dgpsSid = 42;
  nmea_INFO_set_present(&info.present, DGPSSID);

  nmeaGPGGAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, DGPSSID);
  CU_ASSERT_EQUAL(pack.dgpsSid, 42);
  memset(&info, 0, sizeof(info));
}

static void test_nmeaGPGGAGenerate(void) {
  char buf[256];
  nmeaGPGGA pack;
  int r;

  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* invalid inputs */

  r = nmeaGPGGAGenerate(NULL, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 0);
  CU_ASSERT_EQUAL(*buf, '\0');
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  r = nmeaGPGGAGenerate(buf, sizeof(buf), NULL);
  CU_ASSERT_EQUAL(r, 0);
  CU_ASSERT_EQUAL(*buf, '\0');
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* empty with 0 length */

  r = nmeaGPGGAGenerate(buf, 0, &pack);
  CU_ASSERT_EQUAL(r, 25);
  CU_ASSERT_EQUAL(*buf, '\0');
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* empty */

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 25);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,,,,,,,,,,,*56\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* time */

  pack.time.hour = 12;
  pack.time.min = 42;
  pack.time.sec = 43;
  pack.time.hsec = 44;
  nmea_INFO_set_present(&pack.present, UTCTIME);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 34);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,124243.44,,,,,,,,,,,,,*7A\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* latitude */

  pack.latitude = 1242.55;
  pack.ns = 'N';
  nmea_INFO_set_present(&pack.present, LAT);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 35);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,1242.5500,N,,,,,,,,,,,*33\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  pack.latitude = 1242.55;
  pack.ns = '\0';
  nmea_INFO_set_present(&pack.present, LAT);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 34);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,1242.5500,,,,,,,,,,,,*7D\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* longitude */

  pack.longitude = 1242.55;
  pack.ew = 'E';
  nmea_INFO_set_present(&pack.present, LON);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 36);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,01242.5500,E,,,,,,,,,*08\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  pack.longitude = 1242.55;
  pack.ew = '\0';
  nmea_INFO_set_present(&pack.present, LON);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 35);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,01242.5500,,,,,,,,,,*4D\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));


  /* signal */

  pack.signal = NMEA_SIG_MANUAL;
  nmea_INFO_set_present(&pack.present, SIG);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 26);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,,,7,,,,,,,,*61\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* satellites */

  pack.satellitesInView = 42;
  nmea_INFO_set_present(&pack.present, SATINVIEWCOUNT);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 27);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,,,,42,,,,,,,*50\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* hdop */

  pack.hdop = 42.64;
  nmea_INFO_set_present(&pack.present, HDOP);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 29);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,,,,,42.6,,,,,,*48\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* elv */

  pack.elevation = 42.64;
  pack.elevationUnit = 'M';
  nmea_INFO_set_present(&pack.present, ELV);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 30);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,,,,,,42.6,M,,,,*05\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  pack.elevation = 42.64;
  pack.elevationUnit = '\0';
  nmea_INFO_set_present(&pack.present, ELV);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 29);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,,,,,,42.6,,,,,*48\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* height */

  pack.height = 42.64;
  pack.heightUnit = 'M';
  nmea_INFO_set_present(&pack.present, HEIGHT);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 30);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,,,,,,,,42.6,M,,*05\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  pack.height = 42.64;
  pack.heightUnit = '\0';
  nmea_INFO_set_present(&pack.present, HEIGHT);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 29);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,,,,,,,,42.6,,,*48\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* dgpsAge */

  pack.dgpsAge = 42.64;
  nmea_INFO_set_present(&pack.present, DGPSAGE);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 29);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,,,,,,,,,,42.6,*48\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* dgpsSid */

  pack.dgpsSid = 42;
  nmea_INFO_set_present(&pack.present, DGPSSID);

  r = nmeaGPGGAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 27);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGGA,,,,,,,,,,,,,,42*50\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));
}

/*
 * Setup
 */

static int suiteInit(void) {
  nmeaPrintFunction prev;

  prev = nmeaContextSetTraceFunction(traceFunction);
  if (prev) {
    return CUE_SINIT_FAILED;
  }

  prev = nmeaContextSetErrorFunction(errorFunction);
  if (prev) {
    return CUE_SINIT_FAILED;
  }

  return CUE_SUCCESS;
}

static int suiteClean(void) {
  nmeaContextSetErrorFunction(NULL);
  nmeaContextSetTraceFunction(NULL);
  return CUE_SUCCESS;
}

int gpggaSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("gpgga", suiteInit, suiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "nmeaGPGGAParse", test_nmeaGPGGAParse)) //
      || (!CU_add_test(pSuite, "nmeaGPGGAToInfo", test_nmeaGPGGAToInfo)) //
      || (!CU_add_test(pSuite, "nmeaGPGGAFromInfo", test_nmeaGPGGAFromInfo)) //
      || (!CU_add_test(pSuite, "nmeaGPGGAGenerate", test_nmeaGPGGAGenerate)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
