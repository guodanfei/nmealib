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
#include <nmealib/util.h>

#include <CUnit/CUnit.h>
#include <stdlib.h>

/*
 * Forward declarations
 */

int contextSuiteSetup(void);

/*
 * Helpers
 */

static int nmeaTraceCalls = 0;
static int nmeaErrorCalls = 0;

static void traceFunction(const char *s __attribute__((unused)), size_t sz __attribute__((unused))) {
  nmeaTraceCalls++;
}

static void errorFunction(const char *s __attribute__((unused)), size_t sz __attribute__((unused))) {
  nmeaErrorCalls++;
}

static void reset(void) {
  nmeaTraceCalls = 0;
  nmeaErrorCalls = 0;
}

#define  validateContext(traces, errors) \
  { \
   CU_ASSERT_EQUAL(nmeaTraceCalls, traces); \
   CU_ASSERT_EQUAL(nmeaErrorCalls, errors); \
   reset(); \
  }

/*
 * Tests
 */

static void test_nmeaTrace(void) {
  const char * s = "some string";
  char * buf = NULL;
  nmeaPrintFunction prev = nmeaContextSetTraceFunction(NULL);

  CU_ASSERT_PTR_NOT_NULL(prev);

  reset();

  /* invalid inputs */

  nmeaTrace(NULL);
  validateContext(0, 0);

  /* no trace */

  nmeaTrace("%s", s);
  validateContext(0, 0);

  /* trace */

  nmeaContextSetTraceFunction(traceFunction);

  nmeaTrace("%s", s);
  validateContext(1, 0);

  /* empty string*/

  nmeaTrace("%s", "");
  validateContext(0, 0);

  /* very large string*/

  buf = malloc(2 * NMEALIB_BUFFER_CHUNK_SIZE);
  memset(buf, 'A', 2 * NMEALIB_BUFFER_CHUNK_SIZE);
  buf[(2 * NMEALIB_BUFFER_CHUNK_SIZE) - 1] = '\0';

  nmeaTrace("%s", buf);
  validateContext(1, 0);
}

static void test_nmeaError(void) {
  const char * s = "some string";
  char * buf = NULL;
  nmeaPrintFunction prev = nmeaContextSetErrorFunction(NULL);

  CU_ASSERT_PTR_NOT_NULL(prev);

  reset();

  /* invalid inputs */

  nmeaError(NULL);
  validateContext(0, 0);

  /* no error */

  nmeaError("%s", s);
  validateContext(0, 0);

  /* error */

  nmeaContextSetErrorFunction(errorFunction);

  nmeaError("%s", s);
  validateContext(0, 1);

  /* empty string*/

  nmeaError("%s", "");
  validateContext(0, 0);

  /* very large string*/

  buf = malloc(2 * NMEALIB_BUFFER_CHUNK_SIZE);
  memset(buf, 'A', 2 * NMEALIB_BUFFER_CHUNK_SIZE);
  buf[(2 * NMEALIB_BUFFER_CHUNK_SIZE) - 1] = '\0';

  nmeaError("%s", buf);
  validateContext(0, 1);
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

  nmeaTraceCalls = 0;
  nmeaErrorCalls = 0;
  return CUE_SUCCESS;
}

static int suiteClean(void) {
  nmeaContextSetErrorFunction(NULL);
  nmeaContextSetTraceFunction(NULL);
  nmeaTraceCalls = 0;
  nmeaErrorCalls = 0;
  return CUE_SUCCESS;
}

int contextSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("context", suiteInit, suiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "nmeaTrace", test_nmeaTrace)) //
      || (!CU_add_test(pSuite, "nmeaError", test_nmeaError)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
