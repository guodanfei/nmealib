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
#include <nmealib/gpgsa.h>
#include <nmealib/info.h>
#include <nmealib/sentence.h>

#include <CUnit/Basic.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

int gpgsaSuiteSetup(void);

/*
 * Tests
 */

static void test_nmeaGPGSAParse(void) {
  const char * s = "some string";
  nmeaGPGSA packEmpty;
  nmeaGPGSA pack;
  bool r;

  memset(&packEmpty, 0, sizeof(packEmpty));
  memset(&pack, 0, sizeof(pack));

  /* invalid inputs */

  r = nmeaGPGSAParse(NULL, 1, &pack);
  validateParsePack(&pack, r, false, 0, 0, true);

  r = nmeaGPGSAParse(s, 0, &pack);
  validateParsePack(&pack, r, false, 0, 0, true);

  r = nmeaGPGSAParse(s, strlen(s), NULL);
  validateParsePack(&pack, r, false, 0, 0, true);

  packEmpty.fix = NMEALIB_FIX_BAD;

  /* invalid sentence / not enough fields */

  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  /* all fields empty */

  s = "$GPGSA,,,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, true);

  /* sig */

  s = "$GPGSA,!,,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGSA,a,,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, SIG);
  CU_ASSERT_EQUAL(pack.sig, 'A');

  s = "$GPGSA,m,,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, SIG);
  CU_ASSERT_EQUAL(pack.sig, 'M');

  /* fix */

  s = "$GPGSA,,42,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, false, 1, 1, true);

  s = "$GPGSA,,3,,,,,,,,,,,,,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, FIX);
  CU_ASSERT_EQUAL(pack.fix, NMEALIB_FIX_3D);

  /* satPrn */

  s = "$GPGSA,,,12,11,10,5,,7,8,,4,3,2,1,,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, SATINUSE);
  CU_ASSERT_EQUAL(pack.satPrn[0], 12);
  CU_ASSERT_EQUAL(pack.satPrn[1], 11);
  CU_ASSERT_EQUAL(pack.satPrn[2], 10);
  CU_ASSERT_EQUAL(pack.satPrn[3], 5);
  CU_ASSERT_EQUAL(pack.satPrn[4], 7);
  CU_ASSERT_EQUAL(pack.satPrn[5], 8);
  CU_ASSERT_EQUAL(pack.satPrn[6], 4);
  CU_ASSERT_EQUAL(pack.satPrn[7], 3);
  CU_ASSERT_EQUAL(pack.satPrn[8], 2);
  CU_ASSERT_EQUAL(pack.satPrn[9], 1);
  CU_ASSERT_EQUAL(pack.satPrn[10], 0);
  CU_ASSERT_EQUAL(pack.satPrn[11], 0);

  /* pdop */

  s = "$GPGSA,,,,,,,,,,,,,,,-12.128,,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, PDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.pdop, 12.128, DBL_EPSILON);

  /* hdop */

  s = "$GPGSA,,,,,,,,,,,,,,,,-12.128,*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, HDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.hdop, 12.128, DBL_EPSILON);

  /* vdop */

  s = "$GPGSA,,,,,,,,,,,,,,,,,-12.128*";
  r = nmeaGPGSAParse(s, strlen(s), &pack);
  validateParsePack(&pack, r, true, 1, 0, false);
  CU_ASSERT_EQUAL(pack.present, VDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.vdop, 12.128, DBL_EPSILON);
}

static void test_nmeaGPGSAToInfo(void) {
  size_t i;
  nmeaGPGSA pack;
  NmeaInfo infoEmpty;
  NmeaInfo info;

  memset(&pack, 0, sizeof(pack));
  memset(&infoEmpty, 0, sizeof(infoEmpty));
  memset(&info, 0, sizeof(info));

  /* invalid inputs */

  nmeaGPGSAToInfo(NULL, &info);
  validatePackToInfo(&info, 0, 0, true);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  nmeaGPGSAToInfo(&pack, NULL);
  validatePackToInfo(&info, 0, 0, true);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* empty */

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* sig */

  pack.sig = '!';
  nmeaInfoSetPresent(&pack.present, SIG);
  info.sig = NMEALIB_SIG_FLOAT_RTK;

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.sig = '!';
  nmeaInfoSetPresent(&pack.present, SIG);
  info.sig = NMEALIB_SIG_INVALID;

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | SIG);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  CU_ASSERT_EQUAL(info.sig, NMEALIB_SIG_FIX);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.sig = 'A';
  nmeaInfoSetPresent(&pack.present, SIG);
  info.sig = NMEALIB_SIG_INVALID;

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | SIG);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  CU_ASSERT_EQUAL(info.sig, NMEALIB_SIG_FIX);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.sig = 'M';
  nmeaInfoSetPresent(&pack.present, SIG);
  info.sig = NMEALIB_SIG_INVALID;

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | SIG);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  CU_ASSERT_EQUAL(info.sig, NMEALIB_SIG_MANUAL);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* fix */

  pack.fix = NMEALIB_FIX_3D;
  nmeaInfoSetPresent(&pack.present, FIX);

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | FIX);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  CU_ASSERT_EQUAL(info.fix, NMEALIB_FIX_3D);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* satPrn */

  pack.satPrn[0] = 1;
  pack.satPrn[1] = 2;
  pack.satPrn[2] = 0;
  pack.satPrn[3] = 0;
  pack.satPrn[4] = 5;
  pack.satPrn[5] = 6;
  pack.satPrn[6] = 7;
  pack.satPrn[7] = 8;
  pack.satPrn[8] = 0;
  pack.satPrn[9] = 10;
  pack.satPrn[10] = 11;
  pack.satPrn[11] = 12;
  nmeaInfoSetPresent(&pack.present, SATINUSE);

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | SATINUSECOUNT | SATINUSE);
  CU_ASSERT_EQUAL(info.smask, GPGSA);

  CU_ASSERT_EQUAL(info.satinfo.inUse[0], 1);
  CU_ASSERT_EQUAL(info.satinfo.inUse[1], 2);
  CU_ASSERT_EQUAL(info.satinfo.inUse[2], 5);
  CU_ASSERT_EQUAL(info.satinfo.inUse[3], 6);
  CU_ASSERT_EQUAL(info.satinfo.inUse[4], 7);
  CU_ASSERT_EQUAL(info.satinfo.inUse[5], 8);
  CU_ASSERT_EQUAL(info.satinfo.inUse[6], 10);
  CU_ASSERT_EQUAL(info.satinfo.inUse[7], 11);
  CU_ASSERT_EQUAL(info.satinfo.inUse[8], 12);
  for (i = 9; i < NMEALIB_MAX_SATELLITES; i++) {
    CU_ASSERT_EQUAL(info.satinfo.inUse[i], 0);
  }

  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* pdop */

  pack.pdop = -1232.5523;
  nmeaInfoSetPresent(&pack.present, PDOP);

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | PDOP);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  CU_ASSERT_DOUBLE_EQUAL(info.pdop, -1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.pdop = 1232.5523;
  nmeaInfoSetPresent(&pack.present, PDOP);

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | PDOP);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  CU_ASSERT_DOUBLE_EQUAL(info.pdop, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* hdop */

  pack.hdop = -1232.5523;
  nmeaInfoSetPresent(&pack.present, HDOP);

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | HDOP);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  CU_ASSERT_DOUBLE_EQUAL(info.hdop, -1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.hdop = 1232.5523;
  nmeaInfoSetPresent(&pack.present, HDOP);

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | HDOP);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  CU_ASSERT_DOUBLE_EQUAL(info.hdop, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  /* vdop */

  pack.vdop = -1232.5523;
  nmeaInfoSetPresent(&pack.present, VDOP);

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | VDOP);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  CU_ASSERT_DOUBLE_EQUAL(info.vdop, -1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));

  pack.vdop = 1232.5523;
  nmeaInfoSetPresent(&pack.present, VDOP);

  nmeaGPGSAToInfo(&pack, &info);
  validatePackToInfo(&info, 0, 0, false);
  CU_ASSERT_EQUAL(info.present, SMASK | VDOP);
  CU_ASSERT_EQUAL(info.smask, GPGSA);
  CU_ASSERT_DOUBLE_EQUAL(info.vdop, 1232.5523, DBL_EPSILON);
  memset(&pack, 0, sizeof(pack));
  memset(&info, 0, sizeof(info));
}

static void test_nmeaGPGSAFromInfo(void) {
  NmeaInfo info;
  nmeaGPGSA packEmpty;
  nmeaGPGSA pack;

  memset(&info, 0, sizeof(info));
  memset(&packEmpty, 0, sizeof(packEmpty));
  memset(&pack, 0, sizeof(pack));

  /* invalid inputs */

  nmeaGPGSAFromInfo(NULL, &pack);
  validateInfoToPack(&pack, 0, 0, true);
  memset(&info, 0, sizeof(info));

  nmeaGPGSAFromInfo(&info, NULL);
  validateInfoToPack(&pack, 0, 0, true);
  memset(&info, 0, sizeof(info));

  packEmpty.fix = NMEALIB_FIX_BAD;

  /* empty */

  nmeaGPGSAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, true);
  memset(&info, 0, sizeof(info));

  /* sig */

  info.sig = NMEALIB_SIG_ESTIMATED;
  nmeaInfoSetPresent(&info.present, SIG);

  nmeaGPGSAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, SIG);
  CU_ASSERT_EQUAL(pack.sig, 'A');
  memset(&info, 0, sizeof(info));

  info.sig = NMEALIB_SIG_MANUAL;
  nmeaInfoSetPresent(&info.present, SIG);

  nmeaGPGSAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, SIG);
  CU_ASSERT_EQUAL(pack.sig, 'M');
  memset(&info, 0, sizeof(info));

  /* fix */

  info.fix = NMEALIB_FIX_2D;
  nmeaInfoSetPresent(&info.present, FIX);

  nmeaGPGSAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, FIX);
  CU_ASSERT_EQUAL(pack.fix, NMEALIB_FIX_2D);
  memset(&info, 0, sizeof(info));

  /* satPrn */

  info.satinfo.inUse[0] = 1;
  info.satinfo.inUse[1] = 2;
  info.satinfo.inUse[2] = 5;
  info.satinfo.inUse[3] = 6;
  info.satinfo.inUse[4] = 7;
  nmeaInfoSetPresent(&info.present, SATINUSE);

  nmeaGPGSAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, SATINUSE);
  CU_ASSERT_EQUAL(pack.satPrn[0], 1);
  CU_ASSERT_EQUAL(pack.satPrn[1], 2);
  CU_ASSERT_EQUAL(pack.satPrn[2], 5);
  CU_ASSERT_EQUAL(pack.satPrn[3], 6);
  CU_ASSERT_EQUAL(pack.satPrn[4], 7);
  CU_ASSERT_EQUAL(pack.satPrn[5], 0);
  CU_ASSERT_EQUAL(pack.satPrn[6], 0);
  CU_ASSERT_EQUAL(pack.satPrn[7], 0);
  CU_ASSERT_EQUAL(pack.satPrn[8], 0);
  CU_ASSERT_EQUAL(pack.satPrn[9], 0);
  CU_ASSERT_EQUAL(pack.satPrn[10], 0);
  CU_ASSERT_EQUAL(pack.satPrn[11], 0);
  memset(&info, 0, sizeof(info));

  info.satinfo.inUse[0] = 1;
  info.satinfo.inUse[1] = 2;
  info.satinfo.inUse[2] = 5;
  info.satinfo.inUse[3] = 6;
  info.satinfo.inUse[4] = 7;
  info.satinfo.inUse[5] = 9;
  info.satinfo.inUse[6] = 10;
  info.satinfo.inUse[7] = 11;
  info.satinfo.inUse[8] = 14;
  info.satinfo.inUse[9] = 15;
  info.satinfo.inUse[10] = 16;
  info.satinfo.inUse[11] = 17;
  info.satinfo.inUse[12] = 18;
  info.satinfo.inUse[13] = 19;
  nmeaInfoSetPresent(&info.present, SATINUSE);

  nmeaGPGSAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, SATINUSE);
  CU_ASSERT_EQUAL(pack.satPrn[0], 1);
  CU_ASSERT_EQUAL(pack.satPrn[1], 2);
  CU_ASSERT_EQUAL(pack.satPrn[2], 5);
  CU_ASSERT_EQUAL(pack.satPrn[3], 6);
  CU_ASSERT_EQUAL(pack.satPrn[4], 7);
  CU_ASSERT_EQUAL(pack.satPrn[5], 9);
  CU_ASSERT_EQUAL(pack.satPrn[6], 10);
  CU_ASSERT_EQUAL(pack.satPrn[7], 11);
  CU_ASSERT_EQUAL(pack.satPrn[8], 14);
  CU_ASSERT_EQUAL(pack.satPrn[9], 15);
  CU_ASSERT_EQUAL(pack.satPrn[10], 16);
  CU_ASSERT_EQUAL(pack.satPrn[11], 17);
  memset(&info, 0, sizeof(info));

  /* pdop */

  info.pdop = 1232.5523;
  nmeaInfoSetPresent(&info.present, PDOP);

  nmeaGPGSAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, PDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.pdop, 1232.5523, DBL_EPSILON);
  memset(&info, 0, sizeof(info));

  /* hdop */

  info.hdop = 1232.5523;
  nmeaInfoSetPresent(&info.present, HDOP);

  nmeaGPGSAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, HDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.hdop, 1232.5523, DBL_EPSILON);
  memset(&info, 0, sizeof(info));

  /* vdop */

  info.vdop = 1232.5523;
  nmeaInfoSetPresent(&info.present, VDOP);

  nmeaGPGSAFromInfo(&info, &pack);
  validateInfoToPack(&pack, 0, 0, false);
  CU_ASSERT_EQUAL(pack.present, VDOP);
  CU_ASSERT_DOUBLE_EQUAL(pack.vdop, 1232.5523, DBL_EPSILON);
  memset(&info, 0, sizeof(info));
}

static void test_nmeaGPGSAGenerate(void) {
  char buf[256];
  nmeaGPGSA pack;
  size_t r;

  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* invalid inputs */

  r = nmeaGPGSAGenerate(NULL, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 0);
  CU_ASSERT_EQUAL(*buf, '\0');
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  r = nmeaGPGSAGenerate(buf, sizeof(buf), NULL);
  CU_ASSERT_EQUAL(r, 0);
  CU_ASSERT_EQUAL(*buf, '\0');
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* empty with 0 length */

  r = nmeaGPGSAGenerate(buf, 0, &pack);
  CU_ASSERT_EQUAL(r, 28);
  CU_ASSERT_EQUAL(*buf, '\0');
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* empty */

  r = nmeaGPGSAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 28);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGSA,,,,,,,,,,,,,,,,,*6E\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* sig */

  pack.sig = 'A';
  nmeaInfoSetPresent(&pack.present, SIG);

  r = nmeaGPGSAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 29);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGSA,A,,,,,,,,,,,,,,,,*2F\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  pack.sig = '\0';
  nmeaInfoSetPresent(&pack.present, SIG);

  r = nmeaGPGSAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 28);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGSA,,,,,,,,,,,,,,,,,*6E\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* fix */

  pack.fix = NMEALIB_FIX_3D;
  nmeaInfoSetPresent(&pack.present, FIX);

  r = nmeaGPGSAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 29);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGSA,,3,,,,,,,,,,,,,,,*5D\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* satPrn */

  pack.satPrn[0] = 1;
  pack.satPrn[1] = 2;
  pack.satPrn[2] = 3;
  pack.satPrn[4] = 5;
  pack.satPrn[5] = 6;
  pack.satPrn[6] = 7;
  pack.satPrn[9] = 10;
  pack.satPrn[10] = 11;
  pack.satPrn[11] = 12;
  nmeaInfoSetPresent(&pack.present, SATINUSE);

  r = nmeaGPGSAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 40);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGSA,,,1,2,3,,5,6,7,,,10,11,12,,,*68\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* pdop */

  pack.pdop = 42.64;
  nmeaInfoSetPresent(&pack.present, PDOP);

  r = nmeaGPGSAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 32);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGSA,,,,,,,,,,,,,,,42.6,,*70\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* hdop */

  pack.hdop = 42.64;
  nmeaInfoSetPresent(&pack.present, HDOP);

  r = nmeaGPGSAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 32);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGSA,,,,,,,,,,,,,,,,42.6,*70\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));

  /* vdop */

  pack.vdop = 42.64;
  nmeaInfoSetPresent(&pack.present, VDOP);

  r = nmeaGPGSAGenerate(buf, sizeof(buf), &pack);
  CU_ASSERT_EQUAL(r, 32);
  CU_ASSERT_STRING_EQUAL(buf, "$GPGSA,,,,,,,,,,,,,,,,,42.6*70\r\n");
  memset(buf, 0, sizeof(buf));
  memset(&pack, 0, sizeof(pack));
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

  mockContextReset();
  return CUE_SUCCESS;
}

static int suiteClean(void) {
  nmeaContextSetErrorFunction(NULL);
  nmeaContextSetTraceFunction(NULL);
  mockContextReset();
  return CUE_SUCCESS;
}

int gpgsaSuiteSetup(void) {
  CU_pSuite pSuite = CU_add_suite("gpgsa", suiteInit, suiteClean);
  if (!pSuite) {
    return CU_get_error();
  }

  if ( //
      (!CU_add_test(pSuite, "nmeaGPGSAParse", test_nmeaGPGSAParse)) //
      || (!CU_add_test(pSuite, "nmeaGPGSAToInfo", test_nmeaGPGSAToInfo)) //
      || (!CU_add_test(pSuite, "nmeaGPGSAFromInfo", test_nmeaGPGSAFromInfo)) //
      || (!CU_add_test(pSuite, "nmeaGPGSAGenerate", test_nmeaGPGSAGenerate)) //
      ) {
    return CU_get_error();
  }

  return CUE_SUCCESS;
}
