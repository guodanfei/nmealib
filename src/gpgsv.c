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

unsigned int nmeaGPGSVsatellitesToSentencesCount(unsigned int satellites) {
  unsigned int pack_count = satellites / NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE;

  if ((satellites % NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE) > 0) {
    pack_count++;
  }

  if (!pack_count) {
    pack_count++;
  }

  return pack_count;
}

bool nmeaGPGSVparse(const char *s, const size_t sz, nmeaGPGSV *pack) {
  int fieldCount;
  unsigned int i;
  int fieldCountMinimum;

  if (!pack) {
    return false;
  }

  if (!s) {
    goto err;
  }

  nmeaTraceBuffer(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  pack->present = 0;
  pack->sentences = INT_MAX;
  pack->sentence = INT_MAX;
  pack->satellites = INT_MAX;
  for (i = 0; i < NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE; i++) {
    pack->satellite[i].id = INT_MAX;
    pack->satellite[i].elv = 0;
    pack->satellite[i].azimuth = 0;
    pack->satellite[i].sig = 0;
  }

  /* parse */
  fieldCount = nmeaScanf(s, sz, //
      "$" NMEA_PREFIX_GPGSV ",%d,%d,%d," //
          "%d,%d,%d,%d,"//
          "%d,%d,%d,%d,"//
          "%d,%d,%d,%d,"//
          "%d,%d,%d,%d*",//
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
    nmeaError(NMEA_PREFIX_GPGSV " parse error: sentence index %d is larger than the number of sentences %d", pack->sentence,
        pack->sentences);
    goto err;
  }
  if (nmeaGPGSVsatellitesToSentencesCount((pack->sentences * NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE))
      != nmeaGPGSVsatellitesToSentencesCount(pack->satellites)) {
    nmeaError(NMEA_PREFIX_GPGSV " parse error: sentence count %d does not correspond to satellite count %d", pack->sentences,
        pack->satellites);
    goto err;
  }

  /* see that there are enough tokens */
  if (pack->sentence != pack->sentences) {
    fieldCountMinimum = 19;
  } else {
    fieldCountMinimum = 3 + (4 * //
        (NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE - //
            ((pack->sentences * NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE) - pack->satellites)));
  }
  if ((fieldCount != fieldCountMinimum) && (fieldCount != 19)) {
    nmeaError(NMEA_PREFIX_GPGSV " parse error: need %d (or 19) tokens, got %d in '%s'", fieldCountMinimum, fieldCount, s);
    goto err;
  }

  /* validate all satellite settings and count the number of satellites in the sentence */
  for (i = 0; i < NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE; i++) {
    nmeaSATELLITE *sat = &pack->satellite[i];
    if (sat->id == INT_MAX) {
      /* no satellite PRN */
      memset(sat, 0, sizeof(*sat));
    } else {
      /* satellite PRN */

      if ((sat->id <= 0)) {
        nmeaError(NMEA_PREFIX_GPGSV " parse error: invalid satellite PRN %d in '%s'", sat->id, s);
        memset(sat, 0, sizeof(*sat));
      } else if ((sat->elv < -180) || (sat->elv > 180)) {
        nmeaError(NMEA_PREFIX_GPGSV " parse error: invalid satellite elevation %d in '%s'", sat->elv, s);
        memset(sat, 0, sizeof(*sat));
      } else if ((sat->azimuth < 0) || (sat->azimuth >= 360)) {
        nmeaError(NMEA_PREFIX_GPGSV " parse error: invalid satellite azimuth %d in '%s'", sat->azimuth, s);
        memset(sat, 0, sizeof(*sat));
      } else if ((sat->sig < 0) || (sat->sig > 99)) {
        nmeaError(NMEA_PREFIX_GPGSV " parse error: invalid satellite signal %d in '%s'", sat->sig, s);
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

  if (pack->sentence > NMEALIB_GPGSV_MAX_SENTENCES) {
    return;
  }

  if (nmeaGPGSVsatellitesToSentencesCount((pack->sentences * NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE))
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
    offset = (pack->sentence - 1) * NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE;

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
    pack->satellites = (info->satinfo.inview < NMEALIB_MAX_SATELLITES) ?
        info->satinfo.inview :
        NMEALIB_MAX_SATELLITES;

    nmea_INFO_set_present(&pack->present, SATINVIEWCOUNT);
  }

  if (nmea_INFO_is_present(info->present, SATINVIEW)) {
    int offset = 0;
    int i = 0;
    int skipCount = 0;
    int satellites = 0;

    for (i = 0; i < NMEALIB_MAX_SATELLITES; i++) {
      if (info->satinfo.sat[i].id) {
        satellites++;
      }
    }

    pack->sentences = nmeaGPGSVsatellitesToSentencesCount(satellites);

    if ((int) pack_idx >= pack->sentences) {
      pack->sentence = pack->sentences;
    } else {
      pack->sentence = pack_idx + 1;
    }

    /* now skip the first ((pack->pack_index - 1) * NMEA_SATINPACK) in view sats */
    skipCount = ((pack->sentence - 1) * NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE);

    offset = 0;
    while ((skipCount > 0) && (offset < NMEALIB_MAX_SATELLITES)) {
      if (info->satinfo.sat[offset].id) {
        skipCount--;
      }
      offset++;
    }

    for (i = 0; (i < NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE) && (offset < NMEALIB_MAX_SATELLITES); offset++) {
      if (info->satinfo.sat[offset].id) {
        pack->satellite[i] = info->satinfo.sat[offset];
        i++;
      }
    }

    nmea_INFO_set_present(&pack->present, SATINVIEW);
  }
}

int nmeaGPGSVgenerate(char *s, const size_t sz, const nmeaGPGSV *pack) {
  char sentence[256];
  int sentencesInPack = pack->satellites - ((pack->sentence - 1) * NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE);
  char * pSentence = sentence;
  int sentenceLength = sizeof(sentence);
  int writeCount;
  int i;

  sentence[0] = '\0';

  writeCount = snprintf(pSentence, sentenceLength, "$" NMEA_PREFIX_GPGSV ",%d,%d,%d", pack->sentences, pack->sentence,
      pack->satellites);
  pSentence += writeCount;
  sentenceLength -= writeCount;

  for (i = 0; i < NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE; i++) {
    if (i < sentencesInPack) {
      writeCount = snprintf(pSentence, sentenceLength, ",%02d,%02d,%03d,%02d", pack->satellite[i].id,
          pack->satellite[i].elv, pack->satellite[i].azimuth, pack->satellite[i].sig);
    } else {
      writeCount = snprintf(pSentence, sentenceLength, ",,,,");
    }
    pSentence += writeCount;
    sentenceLength -= writeCount;
  }

  return nmeaPrintf(s, sz, "%s", sentence);
}
