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
#include <nmealib/tok.h>

#include <CUnit/Basic.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int tokSuiteSetup(void);

/*
 * Tests
 */

static void test_nmeaCalculateCRC(void) {
  // FIXME
}

static void test_nmeaStringToInteger(void) {
  // FIXME
}

static void test_nmeaStringToUnsignedInteger(void) {
  // FIXME
}

static void test_nmeaStringToLong(void) {
  // FIXME
}

static void test_nmeaStringToUnsignedLong(void) {
  // FIXME
}

static void test_nmeaStringToDouble(void) {
  // FIXME
}

static void test_nmeaAppendChecksum(void) {
  // FIXME
}

static void test_nmeaPrintf(void) {
  // FIXME
}

static void test_nmeaScanf(void) {
  // FIXME
}

/*
 * Setup
 */

int tokSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("tok", mockContextSuiteInit, mockContextSuiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "nmeaCalculateCRC", test_nmeaCalculateCRC)) //
      || (!CU_add_test(pSuite, "nmeaStringToInteger", test_nmeaStringToInteger)) //
      || (!CU_add_test(pSuite, "nmeaStringToUnsignedInteger", test_nmeaStringToUnsignedInteger)) //
      || (!CU_add_test(pSuite, "nmeaStringToLong", test_nmeaStringToLong)) //
      || (!CU_add_test(pSuite, "nmeaStringToUnsignedLong", test_nmeaStringToUnsignedLong)) //
      || (!CU_add_test(pSuite, "nmeaStringToDouble", test_nmeaStringToDouble)) //
      || (!CU_add_test(pSuite, "nmeaAppendChecksum", test_nmeaAppendChecksum)) //
      || (!CU_add_test(pSuite, "nmeaPrintf", test_nmeaPrintf)) //
      || (!CU_add_test(pSuite, "nmeaScanf", test_nmeaScanf)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
