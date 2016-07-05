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
#include <nmealib/util.h>

#include <CUnit/Basic.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int utilSuiteSetup(void);

/*
 * Tests
 */

static void test_Min(void) {
  int r;

  r = MIN(1, 1);
  CU_ASSERT_EQUAL(r, 1);

  r = MIN(1, 2);
  CU_ASSERT_EQUAL(r, 1);

  r = MIN(2, 1);
  CU_ASSERT_EQUAL(r, 1);

  r = MIN(-1, -2);
  CU_ASSERT_EQUAL(r, -2);

  r = MIN(-2, -1);
  CU_ASSERT_EQUAL(r, -2);
}

static void test_Max(void) {
  int r;

  r = MAX(1, 1);
  CU_ASSERT_EQUAL(r, 1);

  r = MAX(1, 2);
  CU_ASSERT_EQUAL(r, 2);

  r = MAX(2, 1);
  CU_ASSERT_EQUAL(r, 2);

  r = MAX(-1, -2);
  CU_ASSERT_EQUAL(r, -1);

  r = MAX(-2, -1);
  CU_ASSERT_EQUAL(r, -1);
}

static void test_nmeaStringTrim(void) {
  const char *s = NULL;
  size_t sz;

  sz = nmeaStringTrim(NULL);
  CU_ASSERT_EQUAL(sz, 0);

  sz = nmeaStringTrim(&s);
  CU_ASSERT_EQUAL(sz, 0);

  s = "";
  sz = nmeaStringTrim(&s);
  CU_ASSERT_EQUAL(sz, 0);

  s = "    \t   ";
  sz = nmeaStringTrim(&s);
  CU_ASSERT_EQUAL(sz, 0);

  s = "123456";
  sz = nmeaStringTrim(&s);
  CU_ASSERT_EQUAL(sz, 6);

  s = "     \t  123456";
  sz = nmeaStringTrim(&s);
  CU_ASSERT_EQUAL(sz, 6);

  s = "123456   \t   ";
  sz = nmeaStringTrim(&s);
  CU_ASSERT_EQUAL(sz, 6);
}

static void test_nmeaStringContainsWhitespace(void) {
  const char *s = NULL;
  size_t sz = 0;
  bool r;

  r = nmeaStringContainsWhitespace(NULL, 1);
  CU_ASSERT_EQUAL(r, false);

  r = nmeaStringContainsWhitespace(s, sz);
  CU_ASSERT_EQUAL(r, false);

  s = "123456";
  r = nmeaStringContainsWhitespace(s, strlen(s));
  CU_ASSERT_EQUAL(r, false);

  s = "123     456";
  r = nmeaStringContainsWhitespace(s, strlen(s));
  CU_ASSERT_EQUAL(r, true);

  s = "123\t   456";
  r = nmeaStringContainsWhitespace(s, strlen(s));
  CU_ASSERT_EQUAL(r, true);
}

/*
 * Setup
 */

int utilSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("util", mockContextSuiteInit, mockContextSuiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "MIN", test_Min)) //
      || (!CU_add_test(pSuite, "MAX", test_Max)) //
      || (!CU_add_test(pSuite, "nmeaStringTrim", test_nmeaStringTrim)) //
      || (!CU_add_test(pSuite, "nmeaStringContainsWhitespace", test_nmeaStringContainsWhitespace)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
