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

  r = nmeaInfoFieldToString(SMASK);
  CU_ASSERT_STRING_EQUAL(r, "SMASK");

  r = nmeaInfoFieldToString(UTCDATE);
  CU_ASSERT_STRING_EQUAL(r, "UTCDATE");

  r = nmeaInfoFieldToString(UTCTIME);
  CU_ASSERT_STRING_EQUAL(r, "UTCTIME");

  r = nmeaInfoFieldToString(SIG);
  CU_ASSERT_STRING_EQUAL(r, "SIG");

  r = nmeaInfoFieldToString(FIX);
  CU_ASSERT_STRING_EQUAL(r, "FIX");

  r = nmeaInfoFieldToString(PDOP);
  CU_ASSERT_STRING_EQUAL(r, "PDOP");

  r = nmeaInfoFieldToString(HDOP);
  CU_ASSERT_STRING_EQUAL(r, "HDOP");

  r = nmeaInfoFieldToString(VDOP);
  CU_ASSERT_STRING_EQUAL(r, "VDOP");

  r = nmeaInfoFieldToString(LAT);
  CU_ASSERT_STRING_EQUAL(r, "LAT");

  r = nmeaInfoFieldToString(LON);
  CU_ASSERT_STRING_EQUAL(r, "LON");

  r = nmeaInfoFieldToString(ELV);
  CU_ASSERT_STRING_EQUAL(r, "ELV");

  r = nmeaInfoFieldToString(SPEED);
  CU_ASSERT_STRING_EQUAL(r, "SPEED");

  r = nmeaInfoFieldToString(TRACK);
  CU_ASSERT_STRING_EQUAL(r, "TRACK");

  r = nmeaInfoFieldToString(MTRACK);
  CU_ASSERT_STRING_EQUAL(r, "MTRACK");

  r = nmeaInfoFieldToString(MAGVAR);
  CU_ASSERT_STRING_EQUAL(r, "MAGVAR");

  r = nmeaInfoFieldToString(SATINUSECOUNT);
  CU_ASSERT_STRING_EQUAL(r, "SATINUSECOUNT");

  r = nmeaInfoFieldToString(SATINUSE);
  CU_ASSERT_STRING_EQUAL(r, "SATINUSE");

  r = nmeaInfoFieldToString(SATINVIEWCOUNT);
  CU_ASSERT_STRING_EQUAL(r, "SATINVIEWCOUNT");

  r = nmeaInfoFieldToString(SATINVIEW);
  CU_ASSERT_STRING_EQUAL(r, "SATINVIEW");

  r = nmeaInfoFieldToString(HEIGHT);
  CU_ASSERT_STRING_EQUAL(r, "HEIGHT");

  r = nmeaInfoFieldToString(DGPSAGE);
  CU_ASSERT_STRING_EQUAL(r, "DGPSAGE");

  r = nmeaInfoFieldToString(DGPSSID);
  CU_ASSERT_STRING_EQUAL(r, "DGPSSID");

  r = nmeaInfoFieldToString(_NmeaPresenceLast + 1);
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

static void test_nmeaInfoTimeSetNow(void) {
  // FIXME
}

static void test_nmeaInfoClear(void) {
  // FIXME
}

static void test_nmeaInfoSanitise(void) {
  // FIXME
}

static void test_nmeaInfoUnitConversion(void) {
  // FIXME
}

static void test_qsortComparePRN(void) {
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

  qsort(&prn, sizeof(prn) / sizeof(prn[0]), sizeof(prn[0]), qsortComparePRN);
  CU_ASSERT_EQUAL(memcmp(prn, exp, sizeof(prn)), 0);
}

static void test_qsortCompactPRN(void) {
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

  qsort(&prn, sizeof(prn) / sizeof(prn[0]), sizeof(prn[0]), qsortCompactPRN);
  CU_ASSERT_EQUAL(memcmp(prn, exp, sizeof(prn)), 0);
}

static void test_qsortCompareSatellite(void) {
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

  qsort(&prn, sizeof(prn) / sizeof(prn[0]), sizeof(prn[0]), qsortCompareSatellite);
  CU_ASSERT_EQUAL(memcmp(prn, exp, sizeof(prn)), 0);
}

static void test_qsortCompactSatellite(void) {
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

  qsort(&prn, sizeof(prn) / sizeof(prn[0]), sizeof(prn[0]), qsortCompactSatellite);
  CU_ASSERT_EQUAL(memcmp(prn, exp, sizeof(prn)), 0);
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

  mockContextReset();
  return CUE_SUCCESS;
}

static int suiteClean(void) {
  nmeaContextSetErrorFunction(NULL);
  nmeaContextSetTraceFunction(NULL);
  mockContextReset();
  return CUE_SUCCESS;
}

int infoSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("info", suiteInit, suiteClean);
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
      || (!CU_add_test(pSuite, "nmeaInfoTimeSetNow", test_nmeaInfoTimeSetNow)) //
      || (!CU_add_test(pSuite, "nmeaInfoClear", test_nmeaInfoClear)) //
      || (!CU_add_test(pSuite, "nmeaInfoSanitise", test_nmeaInfoSanitise)) //
      || (!CU_add_test(pSuite, "nmeaInfoUnitConversion", test_nmeaInfoUnitConversion)) //
      || (!CU_add_test(pSuite, "qsortComparePRN", test_qsortComparePRN)) //
      || (!CU_add_test(pSuite, "qsortCompactPRN", test_qsortCompactPRN)) //
      || (!CU_add_test(pSuite, "qsortCompareSatellite", test_qsortCompareSatellite)) //
      || (!CU_add_test(pSuite, "qsortCompactSatellite", test_qsortCompactSatellite)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
