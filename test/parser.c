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
#include <nmealib/parser.h>

#include <CUnit/Basic.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int parserSuiteSetup(void);

/*
 * Tests
 */

static void test_nmeaParserReset(void) {
  // FIXME
}

static void test_nmeaParserIsHexCharacter(void) {
  // FIXME
}

static void test_nmeaParserInit(void) {
  // FIXME
}

static void test_nmeaParserProcessCharacter(void) {
  // FIXME
}

static void test_nmeaParserParse(void) {
  // FIXME
}

/*
 * Setup
 */

int parserSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("parser", mockContextSuiteInit, mockContextSuiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "nmeaParserReset", test_nmeaParserReset)) //
      || (!CU_add_test(pSuite, "nmeaParserIsHexCharacter", test_nmeaParserIsHexCharacter)) //
      || (!CU_add_test(pSuite, "nmeaParserInit", test_nmeaParserInit)) //
      || (!CU_add_test(pSuite, "nmeaParserProcessCharacter", test_nmeaParserProcessCharacter)) //
      || (!CU_add_test(pSuite, "nmeaParserParse", test_nmeaParserParse)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
