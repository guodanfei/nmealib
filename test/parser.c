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

extern void nmeaParserReset(NmeaParser * parser, NmeaParserSentenceState new_state);
extern bool nmeaParserIsHexCharacter(char c);
extern bool nmeaParserProcessCharacter(NmeaParser *parser, const char * c);

/*
 * Tests
 */

static void test_nmeaParserReset(void) {
  NmeaParser parserEmpty;
  NmeaParser parser;

  memset(&parserEmpty, 0xff, sizeof(parserEmpty));
  memset(&parser, 0xff, sizeof(parser));

  /* invalid input */

  nmeaParserReset(NULL, 0);
  CU_ASSERT_EQUAL(memcmp(&parser, &parserEmpty, sizeof(parser)), 0);
  memset(&parser, 0, sizeof(parser));

  /* normal */

  memset(&parserEmpty, 0, sizeof(parserEmpty));
  parserEmpty.sentence.state = NMEALIB_SENTENCE_STATE_READ_CHECKSUM;

  nmeaParserReset(&parser, NMEALIB_SENTENCE_STATE_READ_CHECKSUM);
  CU_ASSERT_EQUAL(memcmp(&parser, &parserEmpty, sizeof(parser)), 0);
  memset(&parser, 0, sizeof(parser));
}

static void test_nmeaParserIsHexCharacter(void) {
  bool r;

  r = nmeaParserIsHexCharacter('\0');
  CU_ASSERT_EQUAL(r, false);

  r = nmeaParserIsHexCharacter(' ');
  CU_ASSERT_EQUAL(r, false);

  r = nmeaParserIsHexCharacter('0');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('1');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('2');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('3');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('4');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('5');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('6');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('7');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('8');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('9');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('a');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('b');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('c');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('d');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('e');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('f');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('A');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('B');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('C');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('D');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('E');
  CU_ASSERT_EQUAL(r, true);

  r = nmeaParserIsHexCharacter('F');
  CU_ASSERT_EQUAL(r, true);
}

static void test_nmeaParserInit(void) {
  NmeaParser parser;
  bool r;

  memset(&parser, 0, sizeof(parser));

  /* invalid input */

  r = nmeaParserInit(NULL);
  CU_ASSERT_EQUAL(r, false);
  memset(&parser, 0, sizeof(parser));

  /* normal */

  r = nmeaParserInit(&parser);
  CU_ASSERT_EQUAL(r, true);
  CU_ASSERT_EQUAL(parser.sentence.state, NMEALIB_SENTENCE_STATE_SKIP_UNTIL_START);
  memset(&parser, 0, sizeof(parser));
}

static void test_nmeaParserProcessCharacter(void) {
  // FIXME test_nmeaParserProcessCharacter
}

static void test_nmeaParserParse(void) {
  // FIXME test_nmeaParserParse
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
