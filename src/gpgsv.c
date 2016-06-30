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

#include <nmealib/gpgsv.h>

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

size_t nmeaGPGSVsatellitesToSentencesCount(const size_t satellites) {
  size_t pack_count = satellites >> NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT;

  if (satellites & NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_MOD_MASK) {
    pack_count++;
  }

  if (!pack_count) {
    pack_count++;
  }

  return pack_count;
}

bool nmeaGPGSVParse(const char *s, const size_t sz, nmeaGPGSV *pack) {
  size_t fieldCount;
  size_t fieldCountMinimum;
  unsigned int i;

  if (!pack) {
    return false;
  }

  if (!s) {
    goto err;
  }

  nmeaTraceBuffer(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  memset(pack, 0, sizeof(*pack));
  pack->sentences = INT_MAX;
  pack->sentence = INT_MAX;
  pack->satellites = INT_MAX;

  /* parse */
  fieldCount = nmeaScanf(s, sz, //
      "$" NMEA_PREFIX_GPGSV ",%d,%d,%d" //
      ",%d,%d,%d,%d"//
      ",%d,%d,%d,%d"//
      ",%d,%d,%d,%d"//
      ",%d,%d,%d,%d*",//
      &pack->sentences, &pack->sentence, &pack->satellites, //
      &pack->satellite[0].id, &pack->satellite[0].elv, &pack->satellite[0].azimuth, &pack->satellite[0].sig, //
      &pack->satellite[1].id, &pack->satellite[1].elv, &pack->satellite[1].azimuth, &pack->satellite[1].sig, //
      &pack->satellite[2].id, &pack->satellite[2].elv, &pack->satellite[2].azimuth, &pack->satellite[2].sig, //
      &pack->satellite[3].id, &pack->satellite[3].elv, &pack->satellite[3].azimuth, &pack->satellite[3].sig);

  if ((pack->sentences == INT_MAX) //
      || (pack->sentence == INT_MAX) //
      || (pack->satellites == INT_MAX)) {
    goto clear;
  }

  /* check data */
  if (pack->sentence <= 0) {
    nmeaError(NMEA_PREFIX_GPGSV " parse error: sentence index %d is invalid", pack->sentence);
    goto err;
  }
  if (pack->sentence > pack->sentences) {
    nmeaError(NMEA_PREFIX_GPGSV " parse error: sentence index %d is larger than the number of sentences %d",
        pack->sentence, pack->sentences);
    goto err;
  }
  if (nmeaGPGSVsatellitesToSentencesCount((pack->sentences << NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT))
      != nmeaGPGSVsatellitesToSentencesCount(pack->satellites)) {
    nmeaError(NMEA_PREFIX_GPGSV " parse error: sentence count %d does not correspond to satellite count %d",
        pack->sentences, pack->satellites);
    goto err;
  }

  /* see that there are enough tokens */
  if (pack->sentence != pack->sentences) {
    fieldCountMinimum = 19;
  } else {
    fieldCountMinimum = 3 + (4 * //
        (NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE - //
            ((pack->sentences << NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT) - pack->satellites)));
  }
  if ((fieldCount != fieldCountMinimum) && (fieldCount != 19)) {
    nmeaError(NMEA_PREFIX_GPGSV " parse error: need %lu (or 19) tokens, got %lu in '%s'",
        (long unsigned) fieldCountMinimum, (long unsigned) fieldCount, s);
    goto err;
  }

  /* validate all satellite settings and count the number of satellites in the sentence */
  for (i = 0; i < NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE; i++) {
    nmeaSATELLITE *sat = &pack->satellite[i];
    if (!sat->id) {
      /* no satellite PRN */
      memset(sat, 0, sizeof(*sat));
    } else {
      if (!nmeaValidateSatellite(sat, NMEA_PREFIX_GPGSV, s)) {
        memset(sat, 0, sizeof(*sat));
      }
    }
  }

  nmea_INFO_set_present(&pack->present, SATINVIEWCOUNT | SATINVIEW);

  return true;

err:
  memset(pack, 0, sizeof(*pack));
  return false;

clear:
  memset(pack, 0, sizeof(*pack));
  return true;
}

void nmeaGPGSVToInfo(const nmeaGPGSV *pack, nmeaINFO *info) {
  if (!pack || !info) {
    return;
  }

  if (pack->sentence < 1) {
    return;
  }

  if (pack->sentence > pack->sentences) {
    return;
  }

  if (pack->sentence > (int) NMEALIB_GPGSV_MAX_SENTENCES) {
    return;
  }

  if (nmeaGPGSVsatellitesToSentencesCount((pack->sentences << NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT))
      != nmeaGPGSVsatellitesToSentencesCount(pack->satellites)) {
    return;
  }

  nmea_INFO_set_present(&info->present, SMASK);

  info->smask |= GPGSV;

  if (nmea_INFO_is_present(pack->present, SATINVIEWCOUNT)) {
    info->satinfo.inview = pack->satellites;
    nmea_INFO_set_present(&info->present, SATINVIEWCOUNT);
  }

  if (nmea_INFO_is_present(pack->present, SATINVIEW)) {
    int i;
    int offset;
    int max;

    if (pack->sentence == 1) {
      /* first sentence; clear info satellites */
      memset(info->satinfo.sat, 0, sizeof(info->satinfo.sat));
    }

    /* index of 1st sat in pack */
    offset = (pack->sentence - 1) << NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT;

    if (pack->sentence != pack->sentences) {
      max = NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE;
    } else {
      max = pack->satellites - offset;
    }

    for (i = 0; i < max; i++) {
      const nmeaSATELLITE *src = &pack->satellite[i];
      nmeaSATELLITE *dst = &info->satinfo.sat[offset + i];
      if (src->id) {
        *dst = *src;
      }
    }

    nmea_INFO_set_present(&info->present, SATINVIEW);
  }
}

void nmeaGPGSVFromInfo(const nmeaINFO *info, nmeaGPGSV *pack, unsigned int pack_idx) {
  if (!pack || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (nmea_INFO_is_present(info->present, SATINVIEWCOUNT)) {
    pack->satellites = (info->satinfo.inview < (int) NMEALIB_MAX_SATELLITES) ?
        info->satinfo.inview :
        (int) NMEALIB_MAX_SATELLITES;

    pack->sentences = nmeaGPGSVsatellitesToSentencesCount(pack->satellites);

    nmea_INFO_set_present(&pack->present, SATINVIEWCOUNT);
  }

  if (nmea_INFO_is_present(info->present, SATINVIEW)) {
    size_t offset = 0;
    size_t i = 0;

    if ((int) pack_idx >= pack->sentences) {
      pack->sentence = pack->sentences;
    } else {
      pack->sentence = pack_idx + 1;
    }

    /* now skip the first ((pack->pack_index - 1) * NMEA_SATINPACK) in view sats */
    offset = ((pack->sentence - 1) << NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT);

    for (i = 0; (i < NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE) && (offset < NMEALIB_MAX_SATELLITES); i++, offset++) {
      if (info->satinfo.sat[offset].id) {
        pack->satellite[i] = info->satinfo.sat[offset];
      }
    }

    nmea_INFO_set_present(&pack->present, SATINVIEW);
  }
}

int nmeaGPGSVGenerate(char *s, const size_t sz, const nmeaGPGSV *pack) {

#define dst       (&s[chars])
#define available ((size_t) MAX((long) sz - 1 - chars, 0))

  int satCount = 0;
  int i = 0;
  int chars = 0;
  int max = 0;

  if (!s || !sz || !pack) {
    return 0;
  }

  if (nmea_INFO_is_present(pack->present, SATINVIEWCOUNT)) {
    satCount = pack->satellites;
  }

  if (satCount <= 0) {
    return 0;
  }

  chars += snprintf(dst, available, "$" NMEA_PREFIX_GPGSV ",%d,%d", pack->sentences, pack->sentence);

  if (nmea_INFO_is_present(pack->present, SATINVIEWCOUNT)) {
    chars += snprintf(dst, available, ",%d", pack->satellites);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (pack->sentence != pack->sentences) {
    max = NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE;
  } else {
    max = NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE
        - ((pack->sentence << NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT) - pack->satellites);
  }

  if (nmea_INFO_is_present(pack->present, SATINVIEW)) {
    for (i = 0; i < max; i++) {
      const nmeaSATELLITE *sat = &pack->satellite[i];
      if (sat->id) {
        chars += snprintf(dst, available, ",%d,%d,%d,%d", sat->id, sat->elv, sat->azimuth, sat->sig);
      } else {
        chars += snprintf(dst, available, ",,,,");
      }
    }
  }

  /* checksum */
  chars += nmeaAppendChecksum(s, sz, chars);

  return chars;
}
