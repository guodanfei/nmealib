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

#include <nmealib/generator.h>
#include <CUnit/Basic.h>
#include <stddef.h>

int generatorSuiteSetup(void);

extern bool nmeaGeneratorLoopNoise(NmeaGenerator *gen, NmeaInfo *info);

extern bool nmeaGeneratorLoopStatic(NmeaGenerator *gen, NmeaInfo *info);
extern bool nmeaGeneratorResetStatic(NmeaGenerator *gen, NmeaInfo *info);
extern bool nmeaGeneratorInitStatic(NmeaGenerator *gen, NmeaInfo *info);

extern bool nmeaGeneratorLoopRotate(NmeaGenerator *gen, NmeaInfo *info);
extern bool nmeaGeneratorResetRotate(NmeaGenerator *gen, NmeaInfo *info);
extern bool nmeaGeneratorInitRotate(NmeaGenerator *gen, NmeaInfo *info);

extern bool nmeaGeneratorInitRandomMove(NmeaGenerator *gen, NmeaInfo *info);
extern bool nmeaGeneratorLoopRandomMove(NmeaGenerator *gen, NmeaInfo *info);

/*
 * Tests
 */

static void test_nmeaGeneratorLoopNoise(void) {
  // FIXME
}

static void test_nmeaGeneratorLoopStatic(void) {
  // FIXME
}

static void test_nmeaGeneratorResetStatic(void) {
  // FIXME
}

static void test_nmeaGeneratorInitStatic(void) {
  // FIXME
}

static void test_nmeaGeneratorLoopRotate(void) {
  // FIXME
}

static void test_nmeaGeneratorResetRotate(void) {
  // FIXME
}

static void test_nmeaGeneratorInitRotate(void) {
  // FIXME
}

static void test_nmeaGeneratorInitRandomMove(void) {
  // FIXME
}

static void test_nmeaGeneratorLoopRandomMove(void) {
  // FIXME
}

static void test_nmeaGeneratorLoop(void) {
  // FIXME
}

static void test_nmeaGeneratorReset(void) {
  // FIXME
}

static void test_nmeaGeneratorDestroy(void) {
  // FIXME
}

static void test_nmeaGeneratorAppend(void) {
  // FIXME
}

static void test_nmeaGeneratorGenerateFrom(void) {
  // FIXME
}

static void test_nmeaGeneratorCreate(void) {
  // FIXME
}

/*
 * Setup
 */

int generatorSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("generator", mockContextSuiteInit, mockContextSuiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "nmeaGeneratorLoopNoise", test_nmeaGeneratorLoopNoise)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorLoopStatic", test_nmeaGeneratorLoopStatic)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorResetStatic", test_nmeaGeneratorResetStatic)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorInitStatic", test_nmeaGeneratorInitStatic)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorLoopRotate", test_nmeaGeneratorLoopRotate)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorResetRotate", test_nmeaGeneratorResetRotate)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorInitRotate", test_nmeaGeneratorInitRotate)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorInitRandomMove", test_nmeaGeneratorInitRandomMove)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorLoopRandomMove", test_nmeaGeneratorLoopRandomMove)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorLoop", test_nmeaGeneratorLoop)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorReset", test_nmeaGeneratorReset)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorDestroy", test_nmeaGeneratorDestroy)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorAppend", test_nmeaGeneratorAppend)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorGenerateFrom", test_nmeaGeneratorGenerateFrom)) //
      || (!CU_add_test(pSuite, "nmeaGeneratorCreate", test_nmeaGeneratorCreate)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
