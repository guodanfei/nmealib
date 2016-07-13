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
#include <nmealib/sentence.h>
#include <nmealib/validate.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t nmeaGPGSVsatellitesToSentencesCount(const size_t satellites) {
  size_t pack_count;

  if (!satellites) {
    return 1;
  }

  pack_count = satellites >> NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT;

  if (satellites & NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_MOD_MASK) {
    pack_count++;
  }

  return pack_count;
}

bool nmeaGPGSVParse(const char *s, const size_t sz, NmeaGPGSV *pack) {

#define pSat0 pack->inView[0]
#define pSat1 pack->inView[1]
#define pSat2 pack->inView[2]
#define pSat3 pack->inView[3]

  size_t fieldCount;
  size_t fieldCountExpected;
  size_t satellitesInSentence;
  size_t satCount;
  size_t i;

  if (!pack) {
    return false;
  }

  memset(pack, 0, sizeof(*pack));

  if (!s //
      || !sz) {
    return false;
  }

  nmeaContextTraceBuffer(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  pack->sentenceCount = UINT_MAX;
  pack->sentence = UINT_MAX;
  pack->inViewCount = UINT_MAX;

  /* parse */
  fieldCount = nmeaScanf(s, sz, //
      "$" NMEALIB_GPGSV_PREFIX ",%u,%u,%u" //
      ",%u,%d,%u,%u"//
      ",%u,%d,%u,%u"//
      ",%u,%d,%u,%u"//
      ",%u,%d,%u,%u*",//
      &pack->sentenceCount, &pack->sentence, &pack->inViewCount, //
      &pSat0.prn, &pSat0.elevation, &pSat0.azimuth, &pSat0.snr, //
      &pSat1.prn, &pSat1.elevation, &pSat1.azimuth, &pSat1.snr, //
      &pSat2.prn, &pSat2.elevation, &pSat2.azimuth, &pSat2.snr, //
      &pSat3.prn, &pSat3.elevation, &pSat3.azimuth, &pSat3.snr);

  if ((pack->sentenceCount == UINT_MAX) //
      || (pack->sentence == UINT_MAX) //
      || (pack->inViewCount == UINT_MAX)) {
    goto err;
  }

  /* check data */

  if (pack->inViewCount > NMEALIB_MAX_SATELLITES) {
    nmeaContextError(NMEALIB_GPGSV_PREFIX " parse error: can't handle %u satellites (maximum is %u)", pack->inViewCount,
    NMEALIB_MAX_SATELLITES);
    goto err;
  }

  if (!pack->sentenceCount) {
    nmeaContextError(NMEALIB_GPGSV_PREFIX " parse error: sentences count %u is invalid in '%s'", pack->sentenceCount,
        s);
    goto err;
  }

  if (pack->sentenceCount > NMEALIB_GPGSV_MAX_SENTENCES) {
    nmeaContextError(NMEALIB_GPGSV_PREFIX " parse error: can't handle %u sentences (maximum is %u)",
        pack->sentenceCount,
        NMEALIB_GPGSV_MAX_SENTENCES);
    goto err;
  }

  if (pack->sentenceCount != nmeaGPGSVsatellitesToSentencesCount(pack->inViewCount)) {
    nmeaContextError(
        NMEALIB_GPGSV_PREFIX " parse error: sentences count %u does not correspond to satellite count %u in '%s'",
        pack->sentenceCount, pack->inViewCount, s);
    goto err;
  }

  if (!pack->sentence) {
    nmeaContextError(NMEALIB_GPGSV_PREFIX " parse error: sentence index %u is invalid in '%s'", pack->sentence, s);
    goto err;
  }

  if (pack->sentence > pack->sentenceCount) {
    nmeaContextError(
        NMEALIB_GPGSV_PREFIX " parse error: sentence index %u is beyond the number of sentences (%u) in '%s'",
        pack->sentence, pack->sentenceCount, s);
    goto err;
  }

  /* see that there are enough tokens */
  if (pack->sentence != pack->sentenceCount) {
    satellitesInSentence = NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE;
  } else {
    satellitesInSentence = pack->inViewCount - ((pack->sentenceCount - 1) << NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT);
  }

  fieldCountExpected = 3 + (4 * satellitesInSentence); /* 4 fields per satellite */

  if ((fieldCount != fieldCountExpected) //
      && (fieldCount != 19)) {
    nmeaContextError(NMEALIB_GPGSV_PREFIX " parse error: need %lu (or 19) tokens, got %lu in '%s'",
        (long unsigned) fieldCountExpected, (long unsigned) fieldCount, s);
    goto err;
  }

  /* compact satellites */
  qsort(pack->inView, NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE, sizeof(pack->inView[0]), nmeaQsortSatelliteCompact);

  /* validate all satellite settings and count the number of satellites in the sentence */
  satCount = 0;
  for (i = 0; i < NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE; i++) {
    NmeaSatellite *sat = &pack->inView[i];
    if (!nmeaValidateSatellite(sat, NMEALIB_GPGSV_PREFIX, s)) {
      goto err;
    }
    if (sat->prn) {
      satCount++;
    }
  }

  if (!satCount) {
    goto err;
  }

  nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT | NMEALIB_PRESENT_SATINVIEW);

  return true;

err:
  memset(pack, 0, sizeof(*pack));
  return false;

#undef pSat3
#undef pSat2
#undef pSat1
#undef pSat0

}

void nmeaGPGSVToInfo(const NmeaGPGSV *pack, NmeaInfo *info) {
  if (!pack //
      || !info) {
    return;
  }

  if (nmeaInfoIsPresentAny(pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT | NMEALIB_PRESENT_SATINVIEW) //
      && (pack->inViewCount > NMEALIB_MAX_SATELLITES)) {
    nmeaContextError("%s error: can't handle %u satellites (maximum is %u)", __FUNCTION__, pack->inViewCount,
    NMEALIB_MAX_SATELLITES);
    return;
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SATINVIEW)) {
    size_t infoIndex;
    size_t packIndex;

    if (!pack->sentenceCount) {
      nmeaContextError("%s parse error: sentences count %u is invalid", __FUNCTION__, pack->sentenceCount);
      return;
    }

    if (pack->sentenceCount > NMEALIB_GPGSV_MAX_SENTENCES) {
      nmeaContextError("%s error: can't handle %u sentences (maximum is %u)", __FUNCTION__, pack->sentenceCount,
          NMEALIB_GPGSV_MAX_SENTENCES);
      return;
    }

    if (pack->sentenceCount != nmeaGPGSVsatellitesToSentencesCount(pack->inViewCount)) {
      nmeaContextError("%s parse error: sentences count %u does not correspond to satellite count %u", __FUNCTION__,
          pack->sentenceCount, pack->inViewCount);
      return;
    }

    if (!pack->sentence) {
      nmeaContextError("%s error: sentence index %u is invalid", __FUNCTION__, pack->sentence);
      return;
    }

    if (pack->sentence > pack->sentenceCount) {
      nmeaContextError("%s error: sentence %u is beyond the number of sentences (%u)", __FUNCTION__, pack->sentence,
          pack->sentenceCount);
      return;
    }

    if (pack->sentence == 1) {
      /* first sentence; clear info satellites */
      memset(info->satellites.inView, 0, sizeof(info->satellites.inView));
    }

    info->progress.gpgsvInProgress = (pack->sentence != pack->sentenceCount);

    infoIndex = (pack->sentence - 1) << NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT;

    for (packIndex = 0; (packIndex < NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE) && (infoIndex < NMEALIB_MAX_SATELLITES);
        packIndex++, infoIndex++) {
      const NmeaSatellite *src = &pack->inView[packIndex];
      if (!src->prn) {
        memset(&info->satellites.inView[infoIndex], 0, sizeof(info->satellites.inView[infoIndex]));
      } else {
        info->satellites.inView[infoIndex] = *src;
      }
    }

    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEW);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT)) {
    info->satellites.inViewCount = pack->inViewCount;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEWCOUNT);
  }

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SMASK);

  info->smask |= NMEALIB_SENTENCE_GPGSV;
}

void nmeaGPGSVFromInfo(const NmeaInfo *info, NmeaGPGSV *pack, size_t sentence) {
  size_t inViewCount;
  size_t sentences;

  if (!pack) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (!info //
      || !nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_SATINVIEWCOUNT) //
      || !info->satellites.inViewCount) {
    return;
  }

  inViewCount = info->satellites.inViewCount;
  sentences = nmeaGPGSVsatellitesToSentencesCount(inViewCount);

  if (sentence >= sentences) {
    return;
  }

  pack->inViewCount = (unsigned int) inViewCount;
  pack->sentenceCount = (unsigned int) sentences;
  nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT);

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_SATINVIEW)) {
    size_t infoIndex;
    size_t packIndex;

    pack->sentence = (unsigned int) sentence + 1;

    /* now skip the first ((pack->pack_index - 1) * NMEALIB_SATINPACK) in view sats */
    infoIndex = sentence << NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT;

    for (packIndex = 0; (packIndex < NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE) && (infoIndex < NMEALIB_MAX_SATELLITES);
        packIndex++, infoIndex++) {
      if (info->satellites.inView[infoIndex].prn) {
        pack->inView[packIndex] = info->satellites.inView[infoIndex];
      }
    }

    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_SATINVIEW);
  }
}

size_t nmeaGPGSVGenerate(char *s, const size_t sz, const NmeaGPGSV *pack) {

#define dst       (&s[chars])
#define available ((sz <= (size_t) chars) ? 0 : (sz - (size_t) chars))

  int chars = 0;
  size_t satellites = 0;
  size_t sentences = 0;
  size_t sentence = 0;
  size_t satellitesInSentence = 0;
  size_t i = 0;

  if (!s //
      || !pack) {
    return 0;
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT)) {
    satellites = pack->inViewCount;
    sentences = pack->sentenceCount;
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SATINVIEW)) {
    sentence = pack->sentence;
  }

  chars += snprintf(dst, available, //
      "$" NMEALIB_GPGSV_PREFIX ",%lu,%lu,%lu", //
      (long unsigned) sentences, //
      (long unsigned) sentence, //
      (long unsigned) satellites);

  if (pack->sentence != pack->sentenceCount) {
    satellitesInSentence = NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE;
  } else {
    satellitesInSentence = satellites - ((pack->sentenceCount - 1) << NMEALIB_GPGSV_MAX_SATS_PER_SENTENCE_SHIFT);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SATINVIEW)) {
    for (i = 0; i < satellitesInSentence; i++) {
      const NmeaSatellite *sat = &pack->inView[i];
      if (sat->prn) {
        chars += snprintf(dst, available, ",%u,%d,%u,%u", sat->prn, sat->elevation, sat->azimuth, sat->snr);
      } else {
        chars += snprintf(dst, available, ",,,,");
      }
    }
  }

  /* checksum */
  chars += nmeaAppendChecksum(s, sz, (size_t) chars);

  return (size_t) chars;

#undef available
#undef dst

}
