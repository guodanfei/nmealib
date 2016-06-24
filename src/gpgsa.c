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

#include <nmealib/gpgsa.h>

#include <nmealib/context.h>
#include <nmealib/info.h>
#include <nmealib/parse.h>
#include <nmealib/sentence.h>
#include <nmealib/tok.h>
#include <nmealib/validate.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Compare 2 integers, but put zeroes last (consider them to be 1000)
 *
 * @param p1 The first integer
 * @param p2 The second integer
 * @return 0 when both are equal, a negative value when p1 < p2, a positive
 * value otherwise
 */
static int cmp_int(const void *p1, const void *p2) {
  int prn1 = *((const int *) p1);
  int prn2 = *((const int *) p2);

  if (!prn1) {
    prn1 += 1000;
  }
  if (!prn2) {
    prn2 += 1000;
  }

  return (prn1 - prn2);
}

bool nmeaGPGSAparse(const char *s, const size_t sz, nmeaGPGSA *pack) {
  int fieldCount;
  int i;

  if (!pack) {
    return false;
  }

  if (!s) {
    goto err;
  }

  nmeaTraceBuffer(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  memset(pack, 0, sizeof(*pack));
  pack->fix = INT_MAX;
  pack->PDOP = NAN;
  pack->HDOP = NAN;
  pack->VDOP = NAN;

  /* parse */
  fieldCount = nmeaScanf(s, sz, //
      "$" NMEA_PREFIX_GPGSA ",%c,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f*", //
      &pack->sig, //
      &pack->fix, //
      &pack->sat_prn[0], //
      &pack->sat_prn[1], //
      &pack->sat_prn[2], //
      &pack->sat_prn[3], //
      &pack->sat_prn[4], //
      &pack->sat_prn[5], //
      &pack->sat_prn[6], //
      &pack->sat_prn[7], //
      &pack->sat_prn[8], //
      &pack->sat_prn[9], //
      &pack->sat_prn[10], //
      &pack->sat_prn[11], //
      &pack->PDOP, //
      &pack->HDOP, //
      &pack->VDOP);

  /* see that there are enough tokens */
  if (fieldCount != 17) {
    nmeaError(NMEA_PREFIX_GPGSA " parse error: need 17 tokens, got %d in '%s'", fieldCount, s);
    goto err;
  }

  /* determine which fields are present and validate them */

  if (pack->sig) {
    pack->sig = toupper(pack->sig);
    if (!((pack->sig == 'A') || (pack->sig == 'M'))) {
      nmeaError(NMEA_PREFIX_GPGSA " parse error: invalid selection mode '%c' in '%s'", pack->sig, s);
      goto err;
    }

    nmea_INFO_set_present(&pack->present, SIG);
  } else {
    pack->sig = '\0';
  }

  if (pack->fix != INT_MAX) {
    if (!nmeaValidateFix(&pack->fix, NMEA_PREFIX_GPGSA, s)) {
      goto err;
    }

    nmea_INFO_set_present(&pack->present, FIX);
  } else {
    pack->fix = NMEA_FIX_BAD;
  }

  for (i = 0; i < GPGSA_SAT_COUNT; i++) {
    if (pack->sat_prn[i] != 0) {
      qsort(pack->sat_prn, GPGSA_SAT_COUNT, sizeof(int), cmp_int);

      nmea_INFO_set_present(&pack->present, SATINUSE);
      break;
    }
  }
  if (!nmea_INFO_is_present(pack->present, SATINUSE)) {
    memset(pack->sat_prn, 0, sizeof(pack->sat_prn));
  }

  if (!isnan(pack->PDOP)) {
    pack->PDOP = fabs(pack->PDOP);
    nmea_INFO_set_present(&pack->present, PDOP);
  } else {
    pack->PDOP = 0.0;
  }

  if (!isnan(pack->HDOP)) {
    pack->HDOP = fabs(pack->HDOP);
    nmea_INFO_set_present(&pack->present, HDOP);
  } else {
    pack->HDOP = 0.0;
  }

  if (!isnan(pack->VDOP)) {
    pack->VDOP = fabs(pack->VDOP);
    nmea_INFO_set_present(&pack->present, VDOP);
  } else {
    pack->VDOP = 0.0;
  }

  return true;

  err: memset(pack, 0, sizeof(*pack));
  pack->fix = NMEA_FIX_BAD;
  return false;
}

void nmeaGPGSAToInfo(const nmeaGPGSA *pack, nmeaINFO *info) {
  if (!pack || !info) {
    return;
  }

  nmea_INFO_set_present(&info->present, SMASK);

  info->smask |= GPGSA;

  if (nmea_INFO_is_present(pack->present, SIG)) {
    if (pack->sig == 'M') {
      info->sig = NMEA_SIG_MANUAL;
    } else if (nmea_INFO_is_present(pack->present, FIX)) {
      info->sig = NMEA_SIG_FIX;
    } else {
      info->sig = NMEA_SIG_INVALID;
    }

    nmea_INFO_set_present(&info->present, SIG);
  }

  if (nmea_INFO_is_present(pack->present, FIX)) {
    info->fix = pack->fix;
    nmea_INFO_set_present(&info->present, FIX);
  }

  if (nmea_INFO_is_present(pack->present, SATINUSE)) {
    unsigned int packIndex = 0;
    unsigned int infoIndex = 0;

    info->satinfo.inuse = 0;
    memset(&info->satinfo.in_use, 0, sizeof(info->satinfo.in_use[0]));

    for (packIndex = 0; (infoIndex < NMEALIB_MAX_SATELLITES) && (packIndex < GPGSA_SAT_COUNT); packIndex++) {
      int prn = pack->sat_prn[packIndex];
      if (prn) {
        info->satinfo.in_use[infoIndex++] = prn;
        info->satinfo.inuse++;
      }
    }
    nmea_INFO_set_present(&info->present, (SATINUSECOUNT | SATINUSE));
  }

  if (nmea_INFO_is_present(pack->present, PDOP)) {
    info->PDOP = pack->PDOP;
    nmea_INFO_set_present(&info->present, PDOP);
  }

  if (nmea_INFO_is_present(pack->present, HDOP)) {
    info->HDOP = pack->HDOP;
    nmea_INFO_set_present(&info->present, HDOP);
  }

  if (nmea_INFO_is_present(pack->present, VDOP)) {
    info->VDOP = pack->VDOP;
    nmea_INFO_set_present(&info->present, VDOP);
  }
}

void nmeaGPGSAFromInfo(const nmeaINFO *info, nmeaGPGSA *pack) {
  if (!pack || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));
  pack->fix = NMEA_FIX_BAD;

  if (nmea_INFO_is_present(info->present, SIG)) {
    if (info->sig == NMEA_SIG_MANUAL) {
      pack->sig = 'M';
    } else {
      pack->sig = 'A';
    }

    nmea_INFO_set_present(&pack->present, SIG);
  }

  if (nmea_INFO_is_present(info->present, FIX)) {
    pack->fix = info->fix;
    nmea_INFO_set_present(&pack->present, FIX);
  }

  if (nmea_INFO_is_present(info->present, SATINUSE)) {
    unsigned int infoIndex = 0;
    unsigned int packIndex = 0;

    for (infoIndex = 0; (infoIndex < NMEALIB_MAX_SATELLITES) && (packIndex < GPGSA_SAT_COUNT); infoIndex++) {
      int prn = info->satinfo.in_use[infoIndex];
      if (prn) {
        pack->sat_prn[packIndex++] = prn;
      }
    }

    nmea_INFO_set_present(&pack->present, SATINUSE);
  }

  if (nmea_INFO_is_present(info->present, PDOP)) {
    pack->PDOP = info->PDOP;
    nmea_INFO_set_present(&pack->present, PDOP);
  }

  if (nmea_INFO_is_present(info->present, HDOP)) {
    pack->HDOP = info->HDOP;
    nmea_INFO_set_present(&pack->present, HDOP);
  }

  if (nmea_INFO_is_present(info->present, VDOP)) {
    pack->VDOP = info->VDOP;
    nmea_INFO_set_present(&pack->present, VDOP);
  }
}

int nmeaGPGSAgenerate(char *s, const size_t sz, const nmeaGPGSA *pack) {
  int i;
  char sFixMode[2];
  char sFixType[2];
  char sSatPrn[(NMEALIB_MAX_SATELLITES * 4) + 1];
  char sPdop[16];
  char sHdop[16];
  char sVdop[16];

  char * psSatPrn = &sSatPrn[0];
  int ssSatPrn = sizeof(sSatPrn) - 1;

  bool satinuse = nmea_INFO_is_present(pack->present, SATINUSE);

  sFixMode[0] = sFixMode[1] = 0;
  sFixType[0] = sFixType[1] = 0;
  sSatPrn[0] = 0;
  sPdop[0] = 0;
  sHdop[0] = 0;
  sVdop[0] = 0;

  if (nmea_INFO_is_present(pack->present, FIX)) {
    sFixMode[0] = pack->sig;
    snprintf(&sFixType[0], sizeof(sFixType), "%1d", pack->fix);
  }

  for (i = 0; (i < NMEALIB_MAX_SATELLITES) && (i < GPGSA_SAT_COUNT); i++) {
    if (satinuse && pack->sat_prn[i]) {
      int cnt = snprintf(psSatPrn, ssSatPrn, "%d", pack->sat_prn[i]);
      if (cnt >= ssSatPrn) {
        ssSatPrn = 0;
        psSatPrn = &sSatPrn[sizeof(sSatPrn) - 1];
        *psSatPrn = '\0';
        break;
      } else {
        ssSatPrn -= cnt;
        psSatPrn += cnt;
      }
    }
    if (i < (NMEALIB_MAX_SATELLITES - 1)) {
      *psSatPrn = ',';
      psSatPrn++;
      ssSatPrn--;
      *psSatPrn = '\0';
    }
  }

  if (nmea_INFO_is_present(pack->present, PDOP)) {
    snprintf(&sPdop[0], sizeof(sPdop), "%03.1f", pack->PDOP);
  }
  if (nmea_INFO_is_present(pack->present, HDOP)) {
    snprintf(&sHdop[0], sizeof(sHdop), "%03.1f", pack->HDOP);
  }
  if (nmea_INFO_is_present(pack->present, VDOP)) {
    snprintf(&sVdop[0], sizeof(sVdop), "%03.1f", pack->VDOP);
  }

  return nmeaPrintf(s, sz, "$" NMEA_PREFIX_GPGSA ",%s,%s,%s,%s,%s,%s", &sFixMode[0], &sFixType[0], &sSatPrn[0], &sPdop[0], &sHdop[0],
      &sVdop[0]);
}
