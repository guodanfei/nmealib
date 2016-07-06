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

bool nmeaGPGSAParse(const char *s, const size_t sz, nmeaGPGSA *pack) {
  size_t fieldCount;

  if (!s //
      || !sz //
      || !pack) {
    return false;
  }

  nmeaTraceBuffer(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  memset(pack, 0, sizeof(*pack));
  pack->fix = INT_MAX;
  pack->pdop = NAN;
  pack->hdop = NAN;
  pack->vdop = NAN;

  /* parse */
  fieldCount = nmeaScanf(s, sz, //
      "$" NMEALIB_PREFIX_GPGSA ",%C,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%F,%F,%F*", //
      &pack->sig, //
      &pack->fix, //
      &pack->satPrn[0], //
      &pack->satPrn[1], //
      &pack->satPrn[2], //
      &pack->satPrn[3], //
      &pack->satPrn[4], //
      &pack->satPrn[5], //
      &pack->satPrn[6], //
      &pack->satPrn[7], //
      &pack->satPrn[8], //
      &pack->satPrn[9], //
      &pack->satPrn[10], //
      &pack->satPrn[11], //
      &pack->pdop, //
      &pack->hdop, //
      &pack->vdop);

  /* see that there are enough tokens */
  if (fieldCount != 17) {
    nmeaError(NMEALIB_PREFIX_GPGSA " parse error: need 17 tokens, got %lu in '%s'", (long unsigned) fieldCount, s);
    goto err;
  }

  /* determine which fields are present and validate them */

  if (pack->sig) {
    if (!((pack->sig == 'A') //
        || (pack->sig == 'M'))) {
      nmeaError(NMEALIB_PREFIX_GPGSA " parse error: invalid selection mode '%c' in '%s'", pack->sig, s);
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, SIG);
  } else {
    pack->sig = '\0';
  }

  if (pack->fix != INT_MAX) {
    if (!nmeaValidateFix(pack->fix, NMEALIB_PREFIX_GPGSA, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, FIX);
  } else {
    pack->fix = NMEALIB_FIX_BAD;
  }

  qsort(pack->satPrn, NMEALIB_GPGSA_SATS_IN_SENTENCE, sizeof(int), qsortCompactPRN);
  if (!pack->satPrn[0]) {
    memset(pack->satPrn, 0, sizeof(pack->satPrn));
  } else {
    nmeaInfoSetPresent(&pack->present, SATINUSE);
  }

  if (!isnan(pack->pdop)) {
    nmeaInfoSetPresent(&pack->present, PDOP);
  } else {
    pack->pdop = 0.0;
  }

  if (!isnan(pack->hdop)) {
    nmeaInfoSetPresent(&pack->present, HDOP);
  } else {
    pack->hdop = 0.0;
  }

  if (!isnan(pack->vdop)) {
    nmeaInfoSetPresent(&pack->present, VDOP);
  } else {
    pack->vdop = 0.0;
  }

  return true;

err:
  memset(pack, 0, sizeof(*pack));
  pack->fix = NMEALIB_FIX_BAD;
  return false;
}

void nmeaGPGSAToInfo(const nmeaGPGSA *pack, NmeaInfo *info) {
  if (!pack //
      || !info) {
    return;
  }

  nmeaInfoSetPresent(&info->present, SMASK);

  info->smask |= GPGSA;

  if (nmeaInfoIsPresentAll(pack->present, SIG) //
      && (info->sig == NMEALIB_SIG_INVALID)) {
    if (pack->sig == 'M') {
      info->sig = NMEALIB_SIG_MANUAL;
    } else {
      info->sig = NMEALIB_SIG_FIX;
    }

    nmeaInfoSetPresent(&info->present, SIG);
  }

  if (nmeaInfoIsPresentAll(pack->present, FIX)) {
    info->fix = pack->fix;
    nmeaInfoSetPresent(&info->present, FIX);
  }

  if (nmeaInfoIsPresentAll(pack->present, SATINUSE)) {
    size_t packIndex = 0;
    size_t infoIndex = 0;

    info->satinfo.inUseCount = 0;
    memset(&info->satinfo.inUse, 0, sizeof(info->satinfo.inUse[0]));

    for (packIndex = 0; (packIndex < NMEALIB_GPGSA_SATS_IN_SENTENCE) && (infoIndex < NMEALIB_MAX_SATELLITES); packIndex++) {
      int prn = pack->satPrn[packIndex];
      if (prn) {
        info->satinfo.inUse[infoIndex++] = prn;
        info->satinfo.inUseCount++;
      }
    }

    nmeaInfoSetPresent(&info->present, SATINUSECOUNT | SATINUSE);
  }

  if (nmeaInfoIsPresentAll(pack->present, PDOP)) {
    info->pdop = pack->pdop;
    nmeaInfoSetPresent(&info->present, PDOP);
  }

  if (nmeaInfoIsPresentAll(pack->present, HDOP)) {
    info->hdop = pack->hdop;
    nmeaInfoSetPresent(&info->present, HDOP);
  }

  if (nmeaInfoIsPresentAll(pack->present, VDOP)) {
    info->vdop = pack->vdop;
    nmeaInfoSetPresent(&info->present, VDOP);
  }
}

void nmeaGPGSAFromInfo(const NmeaInfo *info, nmeaGPGSA *pack) {
  if (!pack //
      || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));
  pack->fix = NMEALIB_FIX_BAD;

  if (nmeaInfoIsPresentAll(info->present, SIG)) {
    if (info->sig == NMEALIB_SIG_MANUAL) {
      pack->sig = 'M';
    } else {
      pack->sig = 'A';
    }

    nmeaInfoSetPresent(&pack->present, SIG);
  }

  if (nmeaInfoIsPresentAll(info->present, FIX)) {
    pack->fix = info->fix;
    nmeaInfoSetPresent(&pack->present, FIX);
  }

  if (nmeaInfoIsPresentAll(info->present, SATINUSE)) {
    size_t infoIndex = 0;
    size_t packIndex = 0;

    for (infoIndex = 0; (infoIndex < NMEALIB_MAX_SATELLITES) && (packIndex < NMEALIB_GPGSA_SATS_IN_SENTENCE); infoIndex++) {
      int prn = info->satinfo.inUse[infoIndex];
      if (prn) {
        pack->satPrn[packIndex++] = prn;
      }
    }

    nmeaInfoSetPresent(&pack->present, SATINUSE);
  }

  if (nmeaInfoIsPresentAll(info->present, PDOP)) {
    pack->pdop = info->pdop;
    nmeaInfoSetPresent(&pack->present, PDOP);
  }

  if (nmeaInfoIsPresentAll(info->present, HDOP)) {
    pack->hdop = info->hdop;
    nmeaInfoSetPresent(&pack->present, HDOP);
  }

  if (nmeaInfoIsPresentAll(info->present, VDOP)) {
    pack->vdop = info->vdop;
    nmeaInfoSetPresent(&pack->present, VDOP);
  }
}

size_t nmeaGPGSAGenerate(char *s, const size_t sz, const nmeaGPGSA *pack) {

#define dst       (&s[chars])
#define available ((sz <= (size_t) chars) ? 0 : (sz - (size_t) chars))

  int chars = 0;
  bool satInUse;
  size_t i;

  if (!s //
      || !pack) {
    return 0;
  }

  chars += snprintf(dst, available, "$" NMEALIB_PREFIX_GPGSA);

  if (nmeaInfoIsPresentAll(pack->present, SIG) //
      && pack->sig) {
    chars += snprintf(dst, available, ",%c", pack->sig);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, FIX)) {
    chars += snprintf(dst, available, ",%d", pack->fix);
  } else {
    chars += snprintf(dst, available, ",");
  }

  satInUse = nmeaInfoIsPresentAll(pack->present, SATINUSE);
  for (i = 0; i < NMEALIB_GPGSA_SATS_IN_SENTENCE; i++) {
    int prn = pack->satPrn[i];
    if (satInUse && prn) {
      chars += snprintf(dst, available, ",%d", prn);
    } else {
      chars += snprintf(dst, available, ",");
    }
  }

  if (nmeaInfoIsPresentAll(pack->present, PDOP)) {
    chars += snprintf(dst, available, ",%03.1f", pack->pdop);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, HDOP)) {
    chars += snprintf(dst, available, ",%03.1f", pack->hdop);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, VDOP)) {
    chars += snprintf(dst, available, ",%03.1f", pack->vdop);
  } else {
    chars += snprintf(dst, available, ",");
  }

  /* checksum */
  chars += nmeaAppendChecksum(s, sz, (size_t) chars);

  return (size_t) chars;

#undef available
#undef dst

}
