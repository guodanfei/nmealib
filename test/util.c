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

#include <nmealib/util.h>

#include <CUnit/CUnit.h>

/*
 * Forward declarations
 */

int utilSuiteSetup(void);

/*
 * Tests
 */

static void test_nmeaUtilRoundUpToPowerOfTwo(void) {
  size_t s;

  /* underflow */

  s = nmeaUtilRoundUpToPowerOfTwo(0, 10, 16);
  CU_ASSERT_EQUAL(s, 16);

  s = nmeaUtilRoundUpToPowerOfTwo(256, 0, 16);
  CU_ASSERT_EQUAL(s, 16);

  s = nmeaUtilRoundUpToPowerOfTwo(256, 10, 0);
  CU_ASSERT_EQUAL(s, 0);

  s = nmeaUtilRoundUpToPowerOfTwo(30, 10, 32);
  CU_ASSERT_EQUAL(s, 32);

  /* no overflow */

  s = nmeaUtilRoundUpToPowerOfTwo(32, 10, 16);
  CU_ASSERT_EQUAL(s, 16);

  /* overflow */

  s = nmeaUtilRoundUpToPowerOfTwo(32, 31, 16);
  CU_ASSERT_EQUAL(s, 32);

  s = nmeaUtilRoundUpToPowerOfTwo(32, 44, 16);
  CU_ASSERT_EQUAL(s, 32);
}

/*
 * Setup
 */

static int suiteInit(void) {
  return CUE_SUCCESS;
}

static int suiteClean(void) {
  return CUE_SUCCESS;
}

int utilSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("util", suiteInit, suiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "nmeaUtilRoundUpToPowerOfTwo", test_nmeaUtilRoundUpToPowerOfTwo)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
