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
#include <nmealib/parse.h>

#include <CUnit/Basic.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int parseSuiteSetup(void);

/*
 * Tests
 */

static void test_nmeaTIMEparseTime(void) {
  bool r;
  const char *time;
  NmeaTime t;

  /* invalid inputs */

  r = nmeaTimeParseTime(NULL, &t);
  CU_ASSERT_FALSE(r);

  time = "invalid";
  r = nmeaTimeParseTime(time, NULL);
  CU_ASSERT_FALSE(r);

  /* internal whitespace */

  memset(&t, 0xff, sizeof(t));
  time = "12 456";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_FALSE(r);

  /* length 5 */

  memset(&t, 0xff, sizeof(t));
  time = " 12345";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_FALSE(r);

  /* length 6 with external whitespace */

  memset(&t, 0xff, sizeof(t));
  time = " 123456 ";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_TRUE(r);
  CU_ASSERT_EQUAL(t.hour, 12);
  CU_ASSERT_EQUAL(t.min, 34);
  CU_ASSERT_EQUAL(t.sec, 56);
  CU_ASSERT_EQUAL(t.hsec, 00);

  memset(&t, 0xff, sizeof(t));
  time = " 12qq56 ";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_FALSE(r);

  /* length 7 */

  memset(&t, 0xff, sizeof(t));
  time = " 123456.";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_FALSE(r);

  /* length 8 */

  memset(&t, 0xff, sizeof(t));
  time = "123456.7";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_TRUE(r);
  CU_ASSERT_EQUAL(t.hour, 12);
  CU_ASSERT_EQUAL(t.min, 34);
  CU_ASSERT_EQUAL(t.sec, 56);
  CU_ASSERT_EQUAL(t.hsec, 70);

  memset(&t, 0xff, sizeof(t));
  time = "12q456.7";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_FALSE(r);

  /* length 9 */

  memset(&t, 0xff, sizeof(t));
  time = "123456.78";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_TRUE(r);
  CU_ASSERT_EQUAL(t.hour, 12);
  CU_ASSERT_EQUAL(t.min, 34);
  CU_ASSERT_EQUAL(t.sec, 56);
  CU_ASSERT_EQUAL(t.hsec, 78);

  memset(&t, 0xff, sizeof(t));
  time = "123456.q8";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_FALSE(r);

  /* length 10 */

  memset(&t, 0xff, sizeof(t));
  time = "123456.789";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_TRUE(r);
  CU_ASSERT_EQUAL(t.hour, 12);
  CU_ASSERT_EQUAL(t.min, 34);
  CU_ASSERT_EQUAL(t.sec, 56);
  CU_ASSERT_EQUAL(t.hsec, 79);

  memset(&t, 0xff, sizeof(t));
  time = "123456.q89";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_FALSE(r);

  /* length 11 */

  memset(&t, 0xff, sizeof(t));
  time = "123456.7891";
  r = nmeaTimeParseTime(time, &t);
  CU_ASSERT_FALSE(r);
}

static void test_nmeaTIMEparseDate(void) {
  bool r;
  const char *date;
  NmeaTime d;

  /* invalid inputs */

  r = nmeaTimeParseDate(NULL, &d);
  CU_ASSERT_FALSE(r);

  date = "invalid";
  r = nmeaTimeParseDate(date, NULL);
  CU_ASSERT_FALSE(r);

  /* internal whitespace */

  memset(&d, 0xff, sizeof(d));
  date = "12 456";
  r = nmeaTimeParseDate(date, &d);
  CU_ASSERT_FALSE(r);

  /* length 5 */

  memset(&d, 0xff, sizeof(d));
  date = " 12345";
  r = nmeaTimeParseDate(date, &d);
  CU_ASSERT_FALSE(r);

  /* length 6 with external whitespace */

  memset(&d, 0xff, sizeof(d));
  date = " 123456 ";
  r = nmeaTimeParseDate(date, &d);
  CU_ASSERT_TRUE(r);
  CU_ASSERT_EQUAL(d.day, 12);
  CU_ASSERT_EQUAL(d.mon, 34);
  CU_ASSERT_EQUAL(d.year, 2056);

  memset(&d, 0xff, sizeof(d));
  date = " 123492 ";
  r = nmeaTimeParseDate(date, &d);
  CU_ASSERT_TRUE(r);
  CU_ASSERT_EQUAL(d.day, 12);
  CU_ASSERT_EQUAL(d.mon, 34);
  CU_ASSERT_EQUAL(d.year, 1992);

  memset(&d, 0xff, sizeof(d));
  date = " 12qq56 ";
  r = nmeaTimeParseDate(date, &d);
  CU_ASSERT_FALSE(r);
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

int parseSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("parse", suiteInit, suiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "nmeaTIMEparseTime", test_nmeaTIMEparseTime)) //
      || (!CU_add_test(pSuite, "nmeaTIMEparseDate", test_nmeaTIMEparseDate)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
