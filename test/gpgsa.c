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
#include <nmealib/gpgsa.h>
#include <nmealib/info.h>
#include <nmealib/sentence.h>

#include <CUnit/Basic.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

int gpgsaSuiteSetup(void);

/*
 * Tests
 */

static void test_comparePRN(void) {
  int p1;
  int p2;
  int r;

  p1 = 1;
  p2 = 1;
  r = comparePRN(&p1, &p2);
  CU_ASSERT_EQUAL(r, 0);

  p1 = 1;
  p2 = 2;
  r = comparePRN(&p1, &p2);
  CU_ASSERT_TRUE(r < 0);

  p1 = 2;
  p2 = 1;
  r = comparePRN(&p1, &p2);
  CU_ASSERT_TRUE(r > 0);

  p1 = 0;
  p2 = 1;
  r = comparePRN(&p1, &p2);
  CU_ASSERT_TRUE(r > 0);

  p1 = 1;
  p2 = 0;
  r = comparePRN(&p1, &p2);
  CU_ASSERT_TRUE(r < 0);
}

static void test_nmeaGPGSAParse(void) {
  const char * s = "some string";
  nmeaGPGSA packEmpty;
  nmeaGPGSA pack;
  bool r;

  memset(&packEmpty, 0, sizeof(packEmpty));
  memset(&pack, 0, sizeof(pack));

  /* invalid inputs */

  r = nmeaGPGSAParse(NULL, 1, &pack);
  validateParsePack(&pack, r, false, 0, 0, true);

  r = nmeaGPGSAParse(s, 0, &pack);
  validateParsePack(&pack, r, false, 0, 0, true);

  r = nmeaGPGSAParse(s, strlen(s), NULL);
  validateParsePack(&pack, r, false, 0, 0, true);

  packEmpty.fix = NMEA_FIX_BAD;

  /* invalid sentence / not enough fields */

  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  /* all fields empty */

  s = "$GPGSA,,,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, true);

  /* sig */

  s = "$GPGSA,!,,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGSA,a,,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, SIG);
  CU_ASSERT_EQUAL(pack.sig, 'A');

  s = "$GPGSA,m,,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, SIG);
  CU_ASSERT_EQUAL(pack.sig, 'M');

  /* fix */

  s = "$GPGSA,,42,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGSA,,3,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, FIX);
  CU_ASSERT_EQUAL(pack.fix, NMEA_FIX_3D);

  /* satPrn */

  s = "$GPGSA,,,12,11,10,5,6,7,8,9,4,3,2,1,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, SATINUSE);
  CU_ASSERT_EQUAL(pack.satPrn[0], 1);
  CU_ASSERT_EQUAL(pack.satPrn[1], 2);
  CU_ASSERT_EQUAL(pack.satPrn[2], 3);
  CU_ASSERT_EQUAL(pack.satPrn[3], 4);
  CU_ASSERT_EQUAL(pack.satPrn[4], 5);
  CU_ASSERT_EQUAL(pack.satPrn[5], 6);
  CU_ASSERT_EQUAL(pack.satPrn[6], 7);
  CU_ASSERT_EQUAL(pack.satPrn[7], 8);
  CU_ASSERT_EQUAL(pack.satPrn[8], 9);
  CU_ASSERT_EQUAL(pack.satPrn[9], 10);
  CU_ASSERT_EQUAL(pack.satPrn[10], 11);
  CU_ASSERT_EQUAL(pack.satPrn[11], 12);

  /* pdop */

  s = "$GPGSA,,,,,,,,,,,,,,,-12.128,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, PDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.pdop, 12.128, DBL_EPSILON);

  /* hdop */

  s = "$GPGSA,,,,,,,,,,,,,,,,-12.128,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, HDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.hdop, 12.128, DBL_EPSILON);

  /* vdop */

  s = "$GPGSA,,,,,,,,,,,,,,,,,-12.128*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, VDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.vdop, 12.128, DBL_EPSILON);
}

static void test_nmeaGPGSAToInfo(void) {
  // FIXME
}

static void test_nmeaGPGSAFromInfo(void) {
  // FIXME
}

static void test_nmeaGPGSAGenerate(void) {
  // FIXME
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

int gpgsaSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("gpgsa", suiteInit, suiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "comparePRN", test_comparePRN)) //
      || (!CU_add_test(pSuite, "nmeaGPGSAParse", test_nmeaGPGSAParse)) //
      || (!CU_add_test(pSuite, "nmeaGPGSAToInfo", test_nmeaGPGSAToInfo)) //
      || (!CU_add_test(pSuite, "nmeaGPGSAFromInfo", test_nmeaGPGSAFromInfo)) //
      || (!CU_add_test(pSuite, "nmeaGPGSAGenerate", test_nmeaGPGSAGenerate)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
