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

  /* invalid sentence / not enough fields */

//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 1, true);
//
//  /* all fields empty */
//
//  s = "$GPGSA,,,,,,,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, true, 1, 0, true);
//
//  /* time */
//
//  s = "$GPGSA,invalid,,,,,,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 0, true);
//
//  s = "$GPGSA,999999,,,,,,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 1, true);
//
//  s = "$GPGSA,104559.64,,,,,,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, true, 1, 0, false);
//  CU_ASSERT_EQUAL(pack.present, UTCTIME);
//  CU_ASSERT_EQUAL(pack.time.hour, 10);
//  CU_ASSERT_EQUAL(pack.time.min, 45);
//  CU_ASSERT_EQUAL(pack.time.sec, 59);
//  CU_ASSERT_EQUAL(pack.time.hsec, 64);
//
//  /* lat */
//
//  s = "$GPGSA,,1,,,,,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 1, true);
//
//  s = "$GPGSA,,1,!,,,,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 1, true);
//
//  s = "$GPGSA,,-1242.55,s,,,,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, true, 1, 0, false);
//  CU_ASSERT_EQUAL(pack.present, LAT);
//  CU_ASSERT_EQUAL(pack.latitude, 1242.55);
//  CU_ASSERT_EQUAL(pack.ns, 'S');
//
//  /* lon */
//
//  s = "$GPGSA,,,,1,,,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 1, true);
//
//  s = "$GPGSA,,,,1,!,,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 1, true);
//
//  s = "$GPGSA,,,,-1242.55,e,,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, true, 1, 0, false);
//  CU_ASSERT_EQUAL(pack.present, LON);
//  CU_ASSERT_EQUAL(pack.longitude, 1242.55);
//  CU_ASSERT_EQUAL(pack.ew, 'E');
//
//  /* sig */
//
//  s = "$GPGSA,,,,,,4242,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 1, true);
//
//  s = "$GPGSA,,,,,,2,,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, true, 1, 0, false);
//  CU_ASSERT_EQUAL(pack.present, SIG);
//  CU_ASSERT_EQUAL(pack.signal, 2);
//
//  /* satellites */
//
//  s = "$GPGSA,,,,,,,-8,,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, true, 1, 0, false);
//  CU_ASSERT_EQUAL(pack.present, SATINVIEWCOUNT);
//  CU_ASSERT_EQUAL(pack.satellites, 8);
//
//  /* hdop */
//
//  s = "$GPGSA,,,,,,,,-12.128,,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, true, 1, 0, false);
//  CU_ASSERT_EQUAL(pack.present, HDOP);
//  CU_ASSERT_DOUBLE_EQUAL(pack.hdop, 12.128, DBL_EPSILON);
//
//  /* elv */
//
//  s = "$GPGSA,,,,,,,,,1,,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 1, true);
//
//  s = "$GPGSA,,,,,,,,,1,!,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 1, true);
//
//  s = "$GPGSA,,,,,,,,,-42,m,,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, true, 1, 0, false);
//  CU_ASSERT_EQUAL(pack.present, ELV);
//  CU_ASSERT_DOUBLE_EQUAL(pack.elv, -42, DBL_EPSILON);
//  CU_ASSERT_EQUAL(pack.elvUnit, 'M');
//
//  /* diff */
//
//  s = "$GPGSA,,,,,,,,,,,1,,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 1, true);
//
//  s = "$GPGSA,,,,,,,,,,,1,!,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, false, 1, 1, true);
//
//  s = "$GPGSA,,,,,,,,,,,-42,m,,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, true, 1, 0, false);
//  CU_ASSERT_EQUAL(pack.present, HEIGHT);
//  CU_ASSERT_DOUBLE_EQUAL(pack.diff, -42, DBL_EPSILON);
//  CU_ASSERT_EQUAL(pack.diffUnit, 'M');
//
//  /* dgpsAge */
//
//  s = "$GPGSA,,,,,,,,,,,,,-1.250,*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, true, 1, 0, false);
//  CU_ASSERT_EQUAL(pack.present, DGPSAGE);
//  CU_ASSERT_DOUBLE_EQUAL(pack.dgpsAge, 1.250, DBL_EPSILON);
//
//  /* dgpsSid */
//
//  s = "$GPGSA,,,,,,,,,,,,,,-42*";
//  r = nmeaGPGSAParse(s, strlen(s), &pack);
//  validateParsePack(&pack, r, true, 1, 0, false);
//  CU_ASSERT_EQUAL(pack.present, DGPSSID);
//  CU_ASSERT_EQUAL(pack.dgpsSid, 42);
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
