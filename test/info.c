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
#include <nmealib/gmath.h>
#include <nmealib/gpvtg.h>
#include <nmealib/info.h>
#include <nmealib/sentence.h>

#include <CUnit/Basic.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

int infoSuiteSetup(void);

/*
 * Tests
 */

static void test_nmeaInfoSigToString(void) {
  const char * r;

  r = nmeaInfoSigToString(NMEALIB_SIG_INVALID);
  CU_ASSERT_STRING_EQUAL(r, "INVALID");

  r = nmeaInfoSigToString(NMEALIB_SIG_FIX);
  CU_ASSERT_STRING_EQUAL(r, "FIX");

  r = nmeaInfoSigToString(NMEALIB_SIG_DIFFERENTIAL);
  CU_ASSERT_STRING_EQUAL(r, "DIFFERENTIAL");

  r = nmeaInfoSigToString(NMEALIB_SIG_SENSITIVE);
  CU_ASSERT_STRING_EQUAL(r, "SENSITIVE");

  r = nmeaInfoSigToString(NMEALIB_SIG_RTKIN);
  CU_ASSERT_STRING_EQUAL(r, "REAL TIME KINEMATIC");

  r = nmeaInfoSigToString(NMEALIB_SIG_FLOAT_RTK);
  CU_ASSERT_STRING_EQUAL(r, "FLOAT REAL TIME KINEMATIC");

  r = nmeaInfoSigToString(NMEALIB_SIG_ESTIMATED);
  CU_ASSERT_STRING_EQUAL(r, "ESTIMATED (DEAD RECKONING)");

  r = nmeaInfoSigToString(NMEALIB_SIG_MANUAL);
  CU_ASSERT_STRING_EQUAL(r, "MANUAL");

  r = nmeaInfoSigToString(NMEALIB_SIG_SIMULATION);
  CU_ASSERT_STRING_EQUAL(r, "SIMULATION");

  r = nmeaInfoSigToString(NMEALIB_SIG_LAST + 1);
  CU_ASSERT_PTR_NULL(r);
}

static void test_nmeaInfoModeToSig(void) {
  int r;

  r = nmeaInfoModeToSig('N');
  CU_ASSERT_EQUAL(r, NMEALIB_SIG_INVALID);

  r = nmeaInfoModeToSig('A');
  CU_ASSERT_EQUAL(r, NMEALIB_SIG_FIX);

  r = nmeaInfoModeToSig('D');
  CU_ASSERT_EQUAL(r, NMEALIB_SIG_DIFFERENTIAL);

  r = nmeaInfoModeToSig('P');
  CU_ASSERT_EQUAL(r, NMEALIB_SIG_SENSITIVE);

  r = nmeaInfoModeToSig('R');
  CU_ASSERT_EQUAL(r, NMEALIB_SIG_RTKIN);

  r = nmeaInfoModeToSig('F');
  CU_ASSERT_EQUAL(r, NMEALIB_SIG_FLOAT_RTK);

  r = nmeaInfoModeToSig('E');
  CU_ASSERT_EQUAL(r, NMEALIB_SIG_ESTIMATED);

  r = nmeaInfoModeToSig('M');
  CU_ASSERT_EQUAL(r, NMEALIB_SIG_MANUAL);

  r = nmeaInfoModeToSig('S');
  CU_ASSERT_EQUAL(r, NMEALIB_SIG_SIMULATION);

  r = nmeaInfoModeToSig('\1');
  CU_ASSERT_EQUAL(r, NMEALIB_SIG_INVALID);
}

static void test_nmeaInfoSigToMode(void) {
  char r;

  r = nmeaInfoSigToMode( NMEALIB_SIG_INVALID);
  CU_ASSERT_EQUAL(r, 'N');

  r = nmeaInfoSigToMode( NMEALIB_SIG_FIX);
  CU_ASSERT_EQUAL(r, 'A');

  r = nmeaInfoSigToMode( NMEALIB_SIG_DIFFERENTIAL);
  CU_ASSERT_EQUAL(r, 'D');

  r = nmeaInfoSigToMode( NMEALIB_SIG_SENSITIVE);
  CU_ASSERT_EQUAL(r, 'P');

  r = nmeaInfoSigToMode( NMEALIB_SIG_RTKIN);
  CU_ASSERT_EQUAL(r, 'R');

  r = nmeaInfoSigToMode( NMEALIB_SIG_FLOAT_RTK);
  CU_ASSERT_EQUAL(r, 'F');

  r = nmeaInfoSigToMode( NMEALIB_SIG_ESTIMATED);
  CU_ASSERT_EQUAL(r, 'E');

  r = nmeaInfoSigToMode( NMEALIB_SIG_MANUAL);
  CU_ASSERT_EQUAL(r, 'M');

  r = nmeaInfoSigToMode( NMEALIB_SIG_SIMULATION);
  CU_ASSERT_EQUAL(r, 'S');

  r = nmeaInfoSigToMode( NMEALIB_SIG_LAST + 1);
  CU_ASSERT_EQUAL(r, 'N');
}

static void test_nmeaInfoFixToString(void) {
  const char * r;

  r = nmeaInfoFixToString( NMEALIB_FIX_FIRST - 1);
  CU_ASSERT_PTR_NULL(r);

  r = nmeaInfoFixToString( NMEALIB_FIX_BAD);
  CU_ASSERT_STRING_EQUAL(r, "BAD");

  r = nmeaInfoFixToString( NMEALIB_FIX_2D);
  CU_ASSERT_STRING_EQUAL(r, "2D");

  r = nmeaInfoFixToString( NMEALIB_FIX_3D);
  CU_ASSERT_STRING_EQUAL(r, "3D");

  r = nmeaInfoFixToString( NMEALIB_FIX_LAST + 1);
  CU_ASSERT_PTR_NULL(r);
}

static void test_nmeaInfoFieldToString(void) {
  const char * r;

  r = nmeaInfoFieldToString(0);
  CU_ASSERT_PTR_NULL(r);

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_SMASK);
  CU_ASSERT_STRING_EQUAL(r, "SMASK");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_UTCDATE);
  CU_ASSERT_STRING_EQUAL(r, "UTCDATE");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_UTCTIME);
  CU_ASSERT_STRING_EQUAL(r, "UTCTIME");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_SIG);
  CU_ASSERT_STRING_EQUAL(r, "SIG");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_FIX);
  CU_ASSERT_STRING_EQUAL(r, "FIX");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_PDOP);
  CU_ASSERT_STRING_EQUAL(r, "PDOP");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_HDOP);
  CU_ASSERT_STRING_EQUAL(r, "HDOP");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_VDOP);
  CU_ASSERT_STRING_EQUAL(r, "VDOP");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_LAT);
  CU_ASSERT_STRING_EQUAL(r, "LAT");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_LON);
  CU_ASSERT_STRING_EQUAL(r, "LON");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_ELV);
  CU_ASSERT_STRING_EQUAL(r, "ELV");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_SPEED);
  CU_ASSERT_STRING_EQUAL(r, "SPEED");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_TRACK);
  CU_ASSERT_STRING_EQUAL(r, "TRACK");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_MTRACK);
  CU_ASSERT_STRING_EQUAL(r, "MTRACK");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_MAGVAR);
  CU_ASSERT_STRING_EQUAL(r, "MAGVAR");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_SATINUSECOUNT);
  CU_ASSERT_STRING_EQUAL(r, "SATINUSECOUNT");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_SATINUSE);
  CU_ASSERT_STRING_EQUAL(r, "SATINUSE");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_SATINVIEWCOUNT);
  CU_ASSERT_STRING_EQUAL(r, "SATINVIEWCOUNT");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_SATINVIEW);
  CU_ASSERT_STRING_EQUAL(r, "SATINVIEW");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_HEIGHT);
  CU_ASSERT_STRING_EQUAL(r, "HEIGHT");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_DGPSAGE);
  CU_ASSERT_STRING_EQUAL(r, "DGPSAGE");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_DGPSSID);
  CU_ASSERT_STRING_EQUAL(r, "DGPSSID");

  r = nmeaInfoFieldToString(NMEALIB_PRESENT_LAST + 1);
  CU_ASSERT_PTR_NULL(r);
}

static void test_nmeaInfoIsPresentAll(void) {
  bool r;

  r = nmeaInfoIsPresentAll(0xa, 0xa);
  CU_ASSERT_EQUAL(r, true);

  r = nmeaInfoIsPresentAll(0xa, 0x3);
  CU_ASSERT_EQUAL(r, false);

  r = nmeaInfoIsPresentAll(0xa, 0x2);
  CU_ASSERT_EQUAL(r, true);

  r = nmeaInfoIsPresentAll(0xa, 0x5);
  CU_ASSERT_EQUAL(r, false);
}

static void test_nmeaInfoIsPresentAny(void) {
  bool r;

  r = nmeaInfoIsPresentAny(0xa, 0xa);
  CU_ASSERT_EQUAL(r, true);

  r = nmeaInfoIsPresentAny(0xa, 0x3);
  CU_ASSERT_EQUAL(r, true);

  r = nmeaInfoIsPresentAny(0xa, 0x2);
  CU_ASSERT_EQUAL(r, true);

  r = nmeaInfoIsPresentAny(0xa, 0x5);
  CU_ASSERT_EQUAL(r, false);
}

static void test_nmeaInfoSetPresent(void) {
  uint32_t r;

  r = 0x5;
  nmeaInfoSetPresent(&r, 0xa);
  CU_ASSERT_EQUAL(r, 0xf);

  r = 0x5;
  nmeaInfoSetPresent(&r, 0x3);
  CU_ASSERT_EQUAL(r, 0x7);

  r = 0x5;
  nmeaInfoSetPresent(&r, 0x2);
  CU_ASSERT_EQUAL(r, 0x7);

  r = 0x5;
  nmeaInfoSetPresent(&r, 0x5);
  CU_ASSERT_EQUAL(r, 0x5);
}

static void test_nmeaInfoUnsetPresent(void) {
  uint32_t r;

  r = 0xa;
  nmeaInfoUnsetPresent(&r, 0xa);
  CU_ASSERT_EQUAL(r, 0x0);

  r = 0xa;
  nmeaInfoUnsetPresent(&r, 0x3);
  CU_ASSERT_EQUAL(r, 0x8);

  r = 0xa;
  nmeaInfoUnsetPresent(&r, 0x2);
  CU_ASSERT_EQUAL(r, 0x8);

  r = 0xa;
  nmeaInfoUnsetPresent(&r, 0x5);
  CU_ASSERT_EQUAL(r, 0xa);
}

static void test_nmeaTimeSet(void) {
  NmeaTime utcClean;
  uint32_t present;
  struct timeval timeval;
  struct tm tt;
  NmeaTime utc;
  NmeaTime utcExpected;

  utcClean.day = 23;
  utcClean.mon = 12;
  utcClean.year = 2016;
  utcClean.hour = 21;
  utcClean.min = 42;
  utcClean.sec = 51;
  utcClean.hsec = 88;
  present = 0;
  memset(&timeval, 0, sizeof(timeval));

  /* invalid inputs */

  present = 0;
  memcpy(&utc, &utcClean, sizeof(utcClean));
  memcpy(&utcExpected, &utcClean, sizeof(utcClean));
  nmeaTimeSet(NULL, &present, &timeval);
  CU_ASSERT_EQUAL(memcmp(&utc, &utcExpected, sizeof(utc)), 0);
  CU_ASSERT_EQUAL(present, 0);

  /* with timeval */

  present = 0;
  memcpy(&utc, &utcClean, sizeof(utcClean));
  memcpy(&utcExpected, &utcClean, sizeof(utcClean));
  timeval.tv_sec = 0;
  timeval.tv_usec = 420000;
  utcExpected.year = 1970;
  utcExpected.mon = 1;
  utcExpected.day = 1;
  utcExpected.hour = 0;
  utcExpected.min = 0;
  utcExpected.sec = 0;
  utcExpected.hsec = 42;
  nmeaTimeSet(&utc, NULL, &timeval);
  CU_ASSERT_EQUAL(memcmp(&utc, &utcExpected, sizeof(utc)), 0);
  CU_ASSERT_EQUAL(present, 0);

  present = 0;
  memcpy(&utc, &utcClean, sizeof(utcClean));
  memcpy(&utcExpected, &utcClean, sizeof(utcClean));
  timeval.tv_sec = 0;
  timeval.tv_usec = 420000;
  utcExpected.year = 1970;
  utcExpected.mon = 1;
  utcExpected.day = 1;
  utcExpected.hour = 0;
  utcExpected.min = 0;
  utcExpected.sec = 0;
  utcExpected.hsec = 42;
  nmeaTimeSet(&utc, &present, &timeval);
  CU_ASSERT_EQUAL(memcmp(&utc, &utcExpected, sizeof(utc)), 0);
  CU_ASSERT_EQUAL(present, NMEALIB_PRESENT_UTCDATE | NMEALIB_PRESENT_UTCTIME);

  /* without timeval */

  present = 0;
  memcpy(&utc, &utcClean, sizeof(utcClean));
  memcpy(&utcExpected, &utcClean, sizeof(utcClean));
  gettimeofday(&timeval, NULL);
  nmeaTimeSet(&utc, NULL, NULL);
  gmtime_r(&timeval.tv_sec, &tt);
  utcExpected.year = (unsigned int) tt.tm_year + 1900;
  utcExpected.mon = (unsigned int) tt.tm_mon + 1;
  utcExpected.day = (unsigned int) tt.tm_mday;
  utcExpected.hour = (unsigned int) tt.tm_hour;
  utcExpected.min = (unsigned int) tt.tm_min;
  utcExpected.sec = (unsigned int) tt.tm_sec;
  utcExpected.hsec = (unsigned int) (timeval.tv_usec / 10000);
  CU_ASSERT_EQUAL(memcmp(&utc, &utcExpected, sizeof(utc)), 0);
  CU_ASSERT_EQUAL(present, 0);

  present = 0;
  memcpy(&utc, &utcClean, sizeof(utcClean));
  memcpy(&utcExpected, &utcClean, sizeof(utcClean));
  gettimeofday(&timeval, NULL);
  nmeaTimeSet(&utc, &present, NULL);
  gmtime_r(&timeval.tv_sec, &tt);
  utcExpected.year = (unsigned int) tt.tm_year + 1900;
  utcExpected.mon = (unsigned int) tt.tm_mon + 1;
  utcExpected.day = (unsigned int) tt.tm_mday;
  utcExpected.hour = (unsigned int) tt.tm_hour;
  utcExpected.min = (unsigned int) tt.tm_min;
  utcExpected.sec = (unsigned int) tt.tm_sec;
  utcExpected.hsec = (unsigned int) (timeval.tv_usec / 10000);
  CU_ASSERT_EQUAL(memcmp(&utc, &utcExpected, sizeof(utc)), 0);
  CU_ASSERT_EQUAL(present, NMEALIB_PRESENT_UTCDATE | NMEALIB_PRESENT_UTCTIME);
}

static void test_nmeaInfoClear(void) {
  NmeaInfo info;
  NmeaInfo infoExpected;

  /* invalid inputs */

  memset(&info, 0xaa, sizeof(info));
  memcpy(&infoExpected, &info, sizeof(info));
  nmeaInfoClear(NULL);
  CU_ASSERT_EQUAL(memcmp(&info, &infoExpected, sizeof(info)), 0);

  /* normal */

  memset(&info, 0xaa, sizeof(info));
  memset(&infoExpected, 0, sizeof(infoExpected));
  infoExpected.sig = NMEALIB_SIG_INVALID;
  infoExpected.fix = NMEALIB_FIX_BAD;
  infoExpected.present = NMEALIB_PRESENT_SIG | NMEALIB_PRESENT_FIX;
  nmeaInfoClear(&info);
  CU_ASSERT_EQUAL(memcmp(&info, &infoExpected, sizeof(info)), 0);
}

static void test_nmeaInfoSanitise(void) {
  // FIXME
}

static void test_nmeaInfoUnitConversion(void) {
  NmeaInfo info;
  NmeaInfo infoClean;
  NmeaInfo infoExpected;

  infoClean.present = NMEALIB_INFO_PRESENT_MASK;
  infoClean.smask = NMEALIB_SENTENCE_MASK;
  infoClean.utc.day = 23;
  infoClean.utc.mon = 12;
  infoClean.utc.year = 2016;
  infoClean.utc.hour = 21;
  infoClean.utc.min = 42;
  infoClean.utc.sec = 51;
  infoClean.utc.hsec = 88;
  infoClean.sig = NMEALIB_SIG_FLOAT_RTK;
  infoClean.fix = NMEALIB_FIX_3D;
  infoClean.pdop = 1.1;
  infoClean.hdop = 2.2;
  infoClean.vdop = 3.3;
  infoClean.lat = 4.4;
  infoClean.lon = 5.5;
  infoClean.elv = 6.6;
  infoClean.height = 7.7;
  infoClean.speed = 8.8;
  infoClean.track = 9.9;
  infoClean.mtrack = 11.11;
  infoClean.magvar = 12.12;
  infoClean.dgpsAge = 13.13;
  infoClean.dgpsSid = 114;
  memset(&infoClean.satinfo, 0xaa, sizeof(infoClean.satinfo));
  infoClean.progress.gpgsvInProgress = true;


  /* invalid inputs */

  memcpy(&info, &infoClean, sizeof(infoClean));
  info.metric = false;
  memcpy(&infoExpected, &info, sizeof(info));
  nmeaInfoUnitConversion(NULL, true);
  CU_ASSERT_EQUAL(memcmp(&info, &infoExpected, sizeof(info)), 0);

  /* already metric */

  memcpy(&info, &infoClean, sizeof(infoClean));
  info.metric = true;
  memcpy(&infoExpected, &info, sizeof(info));
  nmeaInfoUnitConversion(&info, true);
  CU_ASSERT_EQUAL(memcmp(&info, &infoExpected, sizeof(info)), 0);

  /* already non-metric */

  memcpy(&info, &infoClean, sizeof(infoClean));
  info.metric = false;
  memcpy(&infoExpected, &info, sizeof(info));
  nmeaInfoUnitConversion(&info, false);
  CU_ASSERT_EQUAL(memcmp(&info, &infoExpected, sizeof(info)), 0);

  /* to metric */

  memcpy(&info, &infoClean, sizeof(infoClean));
  info.metric = false;
  memcpy(&infoExpected, &info, sizeof(info));
  infoExpected.metric = true;
  infoExpected.pdop = nmeaDopToMeters(infoExpected.pdop);
  infoExpected.hdop = nmeaDopToMeters(infoExpected.hdop);
  infoExpected.vdop = nmeaDopToMeters(infoExpected.vdop);
  infoExpected.lat = nmeaNdegToDegree(infoExpected.lat);
  infoExpected.lon = nmeaNdegToDegree(infoExpected.lon);
  nmeaInfoUnitConversion(&info, true);
  CU_ASSERT_EQUAL(memcmp(&info, &infoExpected, sizeof(info)), 0);

  /* to non-metric */

  memcpy(&info, &infoClean, sizeof(infoClean));
  info.metric = true;
  memcpy(&infoExpected, &info, sizeof(info));
  infoExpected.metric = false;
  infoExpected.pdop = nmeaMetersToDop(infoExpected.pdop);
  infoExpected.hdop = nmeaMetersToDop(infoExpected.hdop);
  infoExpected.vdop = nmeaMetersToDop(infoExpected.vdop);
  infoExpected.lat = nmeaDegreeToNdeg(infoExpected.lat);
  infoExpected.lon = nmeaDegreeToNdeg(infoExpected.lon);
  nmeaInfoUnitConversion(&info, false);
  CU_ASSERT_EQUAL(memcmp(&info, &infoExpected, sizeof(info)), 0);


  /* to metric, but nothing present */

  memcpy(&info, &infoClean, sizeof(infoClean));
  info.present = 0;
  info.metric = false;
  memcpy(&infoExpected, &info, sizeof(info));
  infoExpected.present = 0;
  infoExpected.metric = true;
  nmeaInfoUnitConversion(&info, true);
  CU_ASSERT_EQUAL(memcmp(&info, &infoExpected, sizeof(info)), 0);
}

static void test_nmeaQsortPRNCompare(void) {
  int prn[] = {
      2,
      3,
      4,
      0,
      1,
      11,
      0,
      14,
      10,
      9,
      8,
      0,
      7,
      0 };
  int exp[] = {
      1,
      2,
      3,
      4,
      7,
      8,
      9,
      10,
      11,
      14,
      0,
      0,
      0,
      0 };

  qsort(&prn, sizeof(prn) / sizeof(prn[0]), sizeof(prn[0]), nmeaQsortPRNCompare);
  CU_ASSERT_EQUAL(memcmp(prn, exp, sizeof(prn)), 0);
}

static void test_nmeaQsortPRNCompact(void) {
  int prn[] = {
      2,
      3,
      4,
      0,
      1,
      11,
      0,
      14,
      10,
      9,
      8,
      0,
      7,
      0 };
  int exp[] = {
      2,
      3,
      4,
      1,
      11,
      14,
      10,
      9,
      8,
      7,
      0,
      0,
      0,
      0 };

  qsort(&prn, sizeof(prn) / sizeof(prn[0]), sizeof(prn[0]), nmeaQsortPRNCompact);
  CU_ASSERT_EQUAL(memcmp(prn, exp, sizeof(prn)), 0);
}

static void test_nmeaQsortSatelliteCompare(void) {
  NmeaSatellite prn[] = {
      {
          2,
          0,
          0,
          0 },
      {
          3,
          0,
          0,
          0 },
      {
          4,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          1,
          0,
          0,
          0 },
      {
          11,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          14,
          0,
          0,
          0 },
      {
          10,
          0,
          0,
          0 },
      {
          9,
          0,
          0,
          0 },
      {
          8,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          7,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 } };
  NmeaSatellite exp[] = {
      {
          1,
          0,
          0,
          0 },
      {
          2,
          0,
          0,
          0 },
      {
          3,
          0,
          0,
          0 },
      {
          4,
          0,
          0,
          0 },
      {
          7,
          0,
          0,
          0 },
      {
          8,
          0,
          0,
          0 },
      {
          9,
          0,
          0,
          0 },
      {
          10,
          0,
          0,
          0 },
      {
          11,
          0,
          0,
          0 },
      {
          14,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 } };

  qsort(&prn, sizeof(prn) / sizeof(prn[0]), sizeof(prn[0]), nmeaQsortSatelliteCompare);
  CU_ASSERT_EQUAL(memcmp(prn, exp, sizeof(prn)), 0);
}

static void test_nmeaQsortSatelliteCompact(void) {
  NmeaSatellite prn[] = {
      {
          2,
          0,
          0,
          0 },
      {
          3,
          0,
          0,
          0 },
      {
          4,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          1,
          0,
          0,
          0 },
      {
          11,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          14,
          0,
          0,
          0 },
      {
          10,
          0,
          0,
          0 },
      {
          9,
          0,
          0,
          0 },
      {
          8,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          7,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 } };
  NmeaSatellite exp[] = {
      {
          2,
          0,
          0,
          0 },
      {
          3,
          0,
          0,
          0 },
      {
          4,
          0,
          0,
          0 },
      {
          1,
          0,
          0,
          0 },
      {
          11,
          0,
          0,
          0 },
      {
          14,
          0,
          0,
          0 },
      {
          10,
          0,
          0,
          0 },
      {
          9,
          0,
          0,
          0 },
      {
          8,
          0,
          0,
          0 },
      {
          7,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 },
      {
          0,
          0,
          0,
          0 } };

  qsort(&prn, sizeof(prn) / sizeof(prn[0]), sizeof(prn[0]), nmeaQsortSatelliteCompact);
  CU_ASSERT_EQUAL(memcmp(prn, exp, sizeof(prn)), 0);
}

/*
 * Setup
 */

int infoSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("info", mockContextSuiteInit, mockContextSuiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "nmeaInfoSigToString", test_nmeaInfoSigToString)) //
      || (!CU_add_test(pSuite, "nmeaInfoModeToSig", test_nmeaInfoModeToSig)) //
      || (!CU_add_test(pSuite, "nmeaInfoSigToMode", test_nmeaInfoSigToMode)) //
      || (!CU_add_test(pSuite, "nmeaInfoFixToString", test_nmeaInfoFixToString)) //
      || (!CU_add_test(pSuite, "nmeaInfoFieldToString", test_nmeaInfoFieldToString)) //
      || (!CU_add_test(pSuite, "nmeaInfoIsPresentAll", test_nmeaInfoIsPresentAll)) //
      || (!CU_add_test(pSuite, "nmeaInfoIsPresentAny", test_nmeaInfoIsPresentAny)) //
      || (!CU_add_test(pSuite, "nmeaInfoSetPresent", test_nmeaInfoSetPresent)) //
      || (!CU_add_test(pSuite, "nmeaInfoUnsetPresent", test_nmeaInfoUnsetPresent)) //
      || (!CU_add_test(pSuite, "nmeaTimeSet", test_nmeaTimeSet)) //
      || (!CU_add_test(pSuite, "nmeaInfoClear", test_nmeaInfoClear)) //
      || (!CU_add_test(pSuite, "nmeaInfoSanitise", test_nmeaInfoSanitise)) //
      || (!CU_add_test(pSuite, "nmeaInfoUnitConversion", test_nmeaInfoUnitConversion)) //
      || (!CU_add_test(pSuite, "nmeaQsortPRNCompare", test_nmeaQsortPRNCompare)) //
      || (!CU_add_test(pSuite, "nmeaQsortPRNCompact", test_nmeaQsortPRNCompact)) //
      || (!CU_add_test(pSuite, "nmeaQsortSatelliteCompare", test_nmeaQsortSatelliteCompare)) //
      || (!CU_add_test(pSuite, "nmeaQsortSatelliteCompact", test_nmeaQsortSatelliteCompact)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
