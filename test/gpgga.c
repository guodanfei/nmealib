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

#include <nmealib/context.h>
#include <nmealib/gpgga.h>
#include <nmealib/info.h>

#include <CUnit/Basic.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

int gpggaSuiteSetup(void);

static int nmeaTraceCalls = 0;
static int nmeaErrorCalls = 0;

static void reset(void) {
  nmeaTraceCalls = 0;
  nmeaErrorCalls = 0;
}

static void traceFunction(const char *s __attribute__((unused)), size_t sz __attribute__((unused))) {
  nmeaTraceCalls++;
}

static void errorFunction(const char *s __attribute__((unused)), size_t sz __attribute__((unused))) {
  nmeaErrorCalls++;
}

/*
 * Helpers
 */

#define validateParsePack(pack, r, rexp, traces, errors, empty) \
  {CU_ASSERT_EQUAL(r, rexp); \
   CU_ASSERT_EQUAL(nmeaTraceCalls, traces); \
   CU_ASSERT_EQUAL(nmeaErrorCalls, errors); \
   if (empty) { \
     CU_ASSERT_TRUE(memcmp(pack, &packEmpty, sizeof(*pack)) == 0); \
   } else { \
     CU_ASSERT_TRUE(memcmp(pack, &packEmpty, sizeof(*pack)) != 0); \
   } \
   reset();}

/*
 * Tests
 */

static void test_nmeaGPGGAparse(void) {
  const char * s = "some string";
  nmeaGPGGA packEmpty;
  nmeaGPGGA pack;
  bool r;

  reset();

  memset(&packEmpty, 0, sizeof(packEmpty));
  memset(&pack, 0, sizeof(pack));

  /* invalid inputs */

  r = nmeaGPGGAparse(NULL, 1, &pack);
  validateParsePack(&pack, r, false, 0, 0, true);

  r = nmeaGPGGAparse(s, 0, &pack);
  validateParsePack(&pack, r, false, 0, 0, true);

  r = nmeaGPGGAparse(s, strlen(s), NULL);
  validateParsePack(&pack, r, false, 0, 0, true);

  /* invalid sentence / not enough fields */

  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  /* all fields empty */

  s = "$GPGGA,,,,,,,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, true);

  /* time */

  s = "$GPGGA,invalid,,,,,,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 0, true);

  s = "$GPGGA,999999,,,,,,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,104559.64,,,,,,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, UTCTIME);
  CU_ASSERT_EQUAL(pack.time.hour, 10);
  CU_ASSERT_EQUAL(pack.time.min, 45);
  CU_ASSERT_EQUAL(pack.time.sec, 59);
  CU_ASSERT_EQUAL(pack.time.hsec, 64);
  reset();

  /* lat */

  s = "$GPGGA,,1,,,,,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,1,!,,,,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,-1242.55,s,,,,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, LAT);
  CU_ASSERT_EQUAL(pack.latitude, 1242.55);
  CU_ASSERT_EQUAL(pack.ns, 'S');

  /* lon */

  s = "$GPGGA,,,,1,,,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,1,!,,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,-1242.55,e,,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, LON);
  CU_ASSERT_EQUAL(pack.longitude, 1242.55);
  CU_ASSERT_EQUAL(pack.ew, 'E');

  /* sig */

  s = "$GPGGA,,,,,,4242,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,,,2,,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, SIG);
  CU_ASSERT_EQUAL(pack.signal, 2);

  /* satellites */

  s = "$GPGGA,,,,,,,-8,,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, SATINVIEWCOUNT);
  CU_ASSERT_EQUAL(pack.satellites, 8);

  /* hdop */

  s = "$GPGGA,,,,,,,,-12.128,,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, HDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.hdop, 12.128, DBL_EPSILON);

  /* elv */

  s = "$GPGGA,,,,,,,,,1,,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,,,,,,1,!,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,,,,,,-42,m,,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, ELV);
  CU_ASSERT_DOUBLE_EQUAL(pack.elv, -42, DBL_EPSILON);
  CU_ASSERT_EQUAL(pack.elvUnit, 'M');

  /* diff */

  s = "$GPGGA,,,,,,,,,,,1,,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,,,,,,,,1,!,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGGA,,,,,,,,,,,-42,m,,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  //CU_ASSERT_EQUAL(pack.present, HEIGHT); not supported yet
  CU_ASSERT_DOUBLE_EQUAL(pack.diff, -42, DBL_EPSILON);
  CU_ASSERT_EQUAL(pack.diffUnit, 'M');

  /* dgpsAge: not supported yet */

  s = "$GPGGA,,,,,,,,,,,,,-1.250,*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  //CU_ASSERT_EQUAL(pack.present, HEIGHT); not supported yet
  CU_ASSERT_DOUBLE_EQUAL(pack.dgpsAge, 1.250, DBL_EPSILON);

  /* dgpsSid: not supported yet */

  s = "$GPGGA,,,,,,,,,,,,,,-42*";
  r = nmeaGPGGAparse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  //CU_ASSERT_EQUAL(pack.present, HEIGHT); not supported yet
  CU_ASSERT_EQUAL(pack.dgpsSid, 42);
}

static void test_nmeaGPGGAToInfo(void) {
}

static void test_nmeaGPGGAFromInfo(void) {
}

static void test_nmeaGPGGAgenerate(void) {
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
      (!CU_add_test(pSuite, "nmeaGPGGAparse", test_nmeaGPGGAparse)) //
      || (!CU_add_test(pSuite, "nmeaGPGGAToInfo", test_nmeaGPGGAToInfo)) //
      || (!CU_add_test(pSuite, "nmeaGPGGAFromInfo", test_nmeaGPGGAFromInfo)) //
      || (!CU_add_test(pSuite, "nmeaGPGGAgenerate", test_nmeaGPGGAgenerate)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
