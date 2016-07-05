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
#include <nmealib/sentence.h>

#include <CUnit/Basic.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int sentenceSuiteSetup(void);

/*
 * Tests
 */

static void test_nmeaSentenceToPrefix(void) {
  const char *r;

  r = nmeaSentenceToPrefix(GPNON - 1);
  CU_ASSERT_PTR_NULL(r);

  r = nmeaSentenceToPrefix(GPNON);
  CU_ASSERT_PTR_NULL(r);

  r = nmeaSentenceToPrefix(GPGGA);
  CU_ASSERT_STRING_EQUAL(r, NMEALIB_PREFIX_GPGGA);

  r = nmeaSentenceToPrefix(GPGSA);
  CU_ASSERT_STRING_EQUAL(r, NMEALIB_PREFIX_GPGSA);

  r = nmeaSentenceToPrefix(GPGSV);
  CU_ASSERT_STRING_EQUAL(r, NMEALIB_PREFIX_GPGSV);

  r = nmeaSentenceToPrefix(GPRMC);
  CU_ASSERT_STRING_EQUAL(r, NMEALIB_PREFIX_GPRMC);

  r = nmeaSentenceToPrefix(GPVTG);
  CU_ASSERT_STRING_EQUAL(r, NMEALIB_PREFIX_GPVTG);

  r = nmeaSentenceToPrefix(_NmeaSentenceLast + 1);
  CU_ASSERT_PTR_NULL(r);
}

static void test_nmeaPrefixToSentence(void) {
  enum NmeaSentence r;
  const char *s;

  s = NULL;
  r = nmeaPrefixToSentence(s, 1);
  CU_ASSERT_EQUAL(r, GPNON);

  s = "";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPNON);

  s = "$GPGG";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPNON);

  s = "$GPGGA,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPGGA);

  s = "GPGGA,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPGGA);

  s = "$GPGSA,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPGSA);

  s = "GPGSA,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPGSA);

  s = "$GPGSV,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPGSV);

  s = "GPGSV,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPGSV);

  s = "$GPRMC,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPRMC);

  s = "GPRMC,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPRMC);

  s = "$GPVTG,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPVTG);

  s = "GPVTG,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPVTG);

  s = "$UNKNW,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPNON);

  s = "UNKNW,blah";
  r = nmeaPrefixToSentence(s, strlen(s));
  CU_ASSERT_EQUAL(r, GPNON);

}

static void test_nmeaSentenceToInfo(void) {
  NmeaInfo infoEmpty;
  NmeaInfo info;
  const char *s;
  bool r;

  memset(&infoEmpty, 0, sizeof(infoEmpty));
  memset(&info, 0, sizeof(info));

  mockContextReset();

  /* NULL sentence */

  r = nmeaSentenceToInfo(NULL, 1, &info);
  CU_ASSERT_EQUAL(r, false);
  validatePackToInfo(&info, 0, 0, true);
  memset(&info, 0, sizeof(info));

  /* invalid sentence */

  s = "$GPXXX,blah";
  r = nmeaSentenceToInfo(s, strlen(s), &info);
  CU_ASSERT_EQUAL(r, false);
  validatePackToInfo(&info, 0, 0, true);
  memset(&info, 0, sizeof(info));

  /* GPGGA */

  s = "$GPGGA,invalid";
  r = nmeaSentenceToInfo(s, strlen(s), &info);
  CU_ASSERT_EQUAL(r, false);
  validatePackToInfo(&info, 1, 1, true);
  memset(&info, 0, sizeof(info));

  s = "$GPGGA,104559.64,,,,,,,,,,,,,*";
  r = nmeaSentenceToInfo(s, strlen(s), &info);
  CU_ASSERT_EQUAL(r, true);
  validatePackToInfo(&info, 1, 0, false);
  CU_ASSERT_EQUAL(info.present, UTCTIME | SMASK);
  CU_ASSERT_EQUAL(info.smask, GPGGA);
  CU_ASSERT_EQUAL(info.utc.hour, 10);
  CU_ASSERT_EQUAL(info.utc.min, 45);
  CU_ASSERT_EQUAL(info.utc.sec, 59);
  CU_ASSERT_EQUAL(info.utc.hsec, 64);
  memset(&info, 0, sizeof(info));

  /* GPGSA */

  s = "$GPGSA,invalid";
  r = nmeaSentenceToInfo(s, strlen(s), &info);
  CU_ASSERT_EQUAL(r, false);
  validatePackToInfo(&info, 1, 1, true);
  memset(&info, 0, sizeof(info));

  s = "$GPGSA,,3,,,,,,,,,,,,,,,*";
  r = nmeaSentenceToInfo(s, strlen(s), &info);
  CU_ASSERT_EQUAL(r, true);
  validatePackToInfo(&info, 1, 0, false);
  CU_ASSERT_EQUAL(info.present, FIX | SMASK);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  CU_ASSERT_EQUAL(info.fix, NMEALIB_FIX_3D);
  memset(&info, 0, sizeof(info));

  /* GPGSV */

  s = "$GPGSV,invalid";
  r = nmeaSentenceToInfo(s, strlen(s), &info);
  CU_ASSERT_EQUAL(r, false);
  validatePackToInfo(&info, 1, 1, true);
  memset(&info, 0, sizeof(info));

  s = "$GPGSV,1,1,4,11,,,45,,,,,12,13,,,,,,*";
  r = nmeaSentenceToInfo(s, strlen(s), &info);
  CU_ASSERT_EQUAL(r, true);
  validatePackToInfo(&info, 1, 0, false);
  CU_ASSERT_EQUAL(info.present, SATINVIEWCOUNT | SATINVIEW | SMASK);
  CU_ASSERT_EQUAL(info.smask, GPGSV);
  CU_ASSERT_EQUAL(info.satinfo.inViewCount, 4);
  CU_ASSERT_EQUAL(info.satinfo.inView[0].prn, 11);
  CU_ASSERT_EQUAL(info.satinfo.inView[0].elevation, 0);
  CU_ASSERT_EQUAL(info.satinfo.inView[0].azimuth, 0);
  CU_ASSERT_EQUAL(info.satinfo.inView[0].snr, 45);
  CU_ASSERT_EQUAL(info.satinfo.inView[1].prn, 12);
  CU_ASSERT_EQUAL(info.satinfo.inView[1].elevation, 13);
  CU_ASSERT_EQUAL(info.satinfo.inView[1].azimuth, 0);
  CU_ASSERT_EQUAL(info.satinfo.inView[1].snr, 0);
  checkSatellitesEmpty(info.satinfo.inView, 2, 3, 0);
  memset(&info, 0, sizeof(info));

  /* GPRMC */

  s = "$GPRMC,invalid";
  r = nmeaSentenceToInfo(s, strlen(s), &info);
  CU_ASSERT_EQUAL(r, false);
  validatePackToInfo(&info, 1, 1, true);
  memset(&info, 0, sizeof(info));

  s = "$GPRMC,104559.64,,,,,,,,,,,*";
  r = nmeaSentenceToInfo(s, strlen(s), &info);
  CU_ASSERT_EQUAL(r, true);
  validatePackToInfo(&info, 1, 0, false);
  CU_ASSERT_EQUAL(info.present, UTCTIME | SMASK);
  CU_ASSERT_EQUAL(info.smask, GPRMC);
  CU_ASSERT_EQUAL(info.utc.hour, 10);
  CU_ASSERT_EQUAL(info.utc.min, 45);
  CU_ASSERT_EQUAL(info.utc.sec, 59);
  CU_ASSERT_EQUAL(info.utc.hsec, 64);
  memset(&info, 0, sizeof(info));

  /* GPVTG */

  s = "$GPVTG,1,q";
  r = nmeaSentenceToInfo(s, strlen(s), &info);
  CU_ASSERT_EQUAL(r, false);
  validatePackToInfo(&info, 1, 1, true);
  memset(&info, 0, sizeof(info));

  s = "$GPVTG,,,,,,,4.25,k*";
  r = nmeaSentenceToInfo(s, strlen(s), &info);
  CU_ASSERT_EQUAL(r, true);
  validatePackToInfo(&info, 1, 0, false);
  CU_ASSERT_EQUAL(info.present, SPEED | SMASK);
  CU_ASSERT_EQUAL(info.smask, GPVTG);
  CU_ASSERT_DOUBLE_EQUAL(info.speed, 4.25, DBL_EPSILON);
  memset(&info, 0, sizeof(info));
}

static void test_nmeaSentenceFromInfo(void) {
  //
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

int sentenceSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("sentence", suiteInit, suiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "nmeaSentenceToPrefix", test_nmeaSentenceToPrefix)) //
      || (!CU_add_test(pSuite, "nmeaPrefixToSentence", test_nmeaPrefixToSentence)) //
      || (!CU_add_test(pSuite, "nmeaSentenceToInfo", test_nmeaSentenceToInfo)) //
      || (!CU_add_test(pSuite, "nmeaSentenceFromInfo", test_nmeaSentenceFromInfo)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
