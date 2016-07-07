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
#include <CUnit/Basic.h>
#include <float.h>
#include <nmealib/math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int mathSuiteSetup(void);

/*
 * Tests
 */

static void test_nmeaMathDegreeToRadian(void) {
  // FIXME test_nmeaMathDegreeToRadian
}

static void test_nmeaMathRadianToDegree(void) {
  // FIXME test_nmeaMathRadianToDegree
}

static void test_nmeaMathNdegToDegree(void) {
  // FIXME test_nmeaMathNdegToDegree
}

static void test_nmeaMathDegreeToNdeg(void) {
  // FIXME test_nmeaMathDegreeToNdeg
}

static void test_nmeaMathNdegToRadian(void) {
  // FIXME test_nmeaMathNdegToRadian
}

static void test_nmeaMathRadianToNdeg(void) {
  // FIXME test_nmeaMathRadianToNdeg
}

static void test_nmeaMathPdopCalculate(void) {
  // FIXME test_nmeaMathPdopCalculate
}

static void test_nmeaMathDopToMeters(void) {
  // FIXME test_nmeaMathDopToMeters
}

static void test_nmeaMathMetersToDop(void) {
  // FIXME test_nmeaMathMetersToDop
}

static void test_nmeaMathInfoToPosition(void) {
  // FIXME test_nmeaMathInfoToPosition
}

static void test_nmeaMathPositionToInfo(void) {
  // FIXME test_nmeaMathPositionToInfo
}

static void test_nmeaMathDistance(void) {
  // FIXME test_nmeaMathDistance
}

static void test_nmeaMathDistanceEllipsoid(void) {
  // FIXME test_nmeaMathDistanceEllipsoid
}

static void test_nmeaMathMoveFlat(void) {
  // FIXME test_nmeaMathMoveFlat
}

static void test_nmeaMathMoveFlatEllipsoid(void) {
  // FIXME test_nmeaMathMoveFlatEllipsoid
}

/*
 * Setup
 */

int mathSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("math", mockContextSuiteInit, mockContextSuiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "nmeaMathDegreeToRadian", test_nmeaMathDegreeToRadian)) //
      || (!CU_add_test(pSuite, "nmeaMathRadianToDegree", test_nmeaMathRadianToDegree)) //
      || (!CU_add_test(pSuite, "nmeaMathNdegToDegree", test_nmeaMathNdegToDegree)) //
      || (!CU_add_test(pSuite, "nmeaMathDegreeToNdeg", test_nmeaMathDegreeToNdeg)) //
      || (!CU_add_test(pSuite, "nmeaMathNdegToRadian", test_nmeaMathNdegToRadian)) //
      || (!CU_add_test(pSuite, "nmeaMathRadianToNdeg", test_nmeaMathRadianToNdeg)) //
      || (!CU_add_test(pSuite, "nmeaMathPdopCalculate", test_nmeaMathPdopCalculate)) //
      || (!CU_add_test(pSuite, "nmeaMathDopToMeters", test_nmeaMathDopToMeters)) //
      || (!CU_add_test(pSuite, "nmeaMathMetersToDop", test_nmeaMathMetersToDop)) //
      || (!CU_add_test(pSuite, "nmeaMathInfoToPosition", test_nmeaMathInfoToPosition)) //
      || (!CU_add_test(pSuite, "nmeaMathPositionToInfo", test_nmeaMathPositionToInfo)) //
      || (!CU_add_test(pSuite, "nmeaMathDistance", test_nmeaMathDistance)) //
      || (!CU_add_test(pSuite, "nmeaMathDistanceEllipsoid", test_nmeaMathDistanceEllipsoid)) //
      || (!CU_add_test(pSuite, "nmeaMathDistanceEllipsoid", test_nmeaMathDistanceEllipsoid)) //
      || (!CU_add_test(pSuite, "nmeaMathMoveFlat", test_nmeaMathMoveFlat)) //
      || (!CU_add_test(pSuite, "nmeaMathMoveFlatEllipsoid", test_nmeaMathMoveFlatEllipsoid)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
