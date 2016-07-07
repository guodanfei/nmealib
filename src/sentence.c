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

#include <nmealib/sentence.h>
#include <stdlib.h>
#include <string.h>

/**
 * The type definition for an entry mapping a NMEA sentence prefix to a sentence type
 */
typedef struct _NmeaSentencePrefixToType {
  const char * prefix;
  const enum NmeaSentence sentence;
} NmeaSentencePrefixToType;

/**
 * The map from NMEA sentence prefix to sentence type
 */
static const NmeaSentencePrefixToType nmealibSentencePrefixToType[] = {
    {
        .prefix = NMEALIB_PREFIX_GPGGA, //
        .sentence = GPGGA //
    },
    {
        .prefix = NMEALIB_PREFIX_GPGSA, //
        .sentence = GPGSA //
    },
    {
        .prefix = NMEALIB_PREFIX_GPGSV, //
        .sentence = GPGSV //
    },
    {
        .prefix = NMEALIB_PREFIX_GPRMC, //
        .sentence = GPRMC //
    },
    {
        .prefix = NMEALIB_PREFIX_GPVTG, //
        .sentence = GPVTG //
    },
    {
        .prefix = NULL, //
        .sentence = GPNON //
    }//
};

const char * nmeaSentenceToPrefix(enum NmeaSentence sentence) {
  size_t i = 0;

  while (nmealibSentencePrefixToType[i].prefix) {
    if (nmealibSentencePrefixToType[i].sentence == sentence) {
      return nmealibSentencePrefixToType[i].prefix;
    }

    i++;
  }

  return NULL;
}

enum NmeaSentence nmeaPrefixToSentence(const char *s, const size_t sz) {
  const char * str = s;
  size_t size = sz;
  size_t i = 0;

  if (!str //
      || !size) {
    return GPNON;
  }

  if (*str == '$') {
    str++;
    size--;
  }

  if (size < NMEALIB_PREFIX_LENGTH) {
    return GPNON;
  }

  while (nmealibSentencePrefixToType[i].prefix) {
    if (!strncmp(str, nmealibSentencePrefixToType[i].prefix, NMEALIB_PREFIX_LENGTH)) {
      return nmealibSentencePrefixToType[i].sentence;
    }

    i++;
  }

  return GPNON;
}

bool nmeaSentenceToInfo(const char *s, const size_t sz, NmeaInfo *info) {
  switch (nmeaPrefixToSentence(s, sz)) {
    case GPGGA: {
      NmeaGPGGA gpgga;
      if (nmeaGPGGAParse(s, sz, &gpgga)) {
        nmeaGPGGAToInfo(&gpgga, info);
        return true;
      }

      return false;
    }

    case GPGSA: {
      NmeaGPGSA gpgsa;
      if (nmeaGPGSAParse(s, sz, &gpgsa)) {
        nmeaGPGSAToInfo(&gpgsa, info);
        return true;
      }

      return false;
    }

    case GPGSV: {
      NmeaGPGSV gpgsv;
      if (nmeaGPGSVParse(s, sz, &gpgsv)) {
        nmeaGPGSVToInfo(&gpgsv, info);
        return true;
      }

      return false;
    }

    case GPRMC: {
      NmeaGPRMC gprmc;
      if (nmeaGPRMCParse(s, sz, &gprmc)) {
        nmeaGPRMCToInfo(&gprmc, info);
        return true;
      }

      return false;
    }

    case GPVTG: {
      NmeaGPVTG gpvtg;
      if (nmeaGPVTGParse(s, sz, &gpvtg)) {
        nmeaGPVTGToInfo(&gpvtg, info);
        return true;
      }

      return false;
    }

    case GPNON:
    default:
      return false;
  }
}

size_t nmeaSentenceFromInfo(char **buf, const NmeaInfo *info, const enum NmeaSentence mask) {

#define dst       (&s[chars])
#define available ((sz <= (size_t) chars) ? 0 : (sz - (size_t) chars))

#define generateSentence(expression) { \
  size_t addedChars = expression; \
  while (addedChars >= available) { \
    sz += NMEALIB_BUFFER_CHUNK_SIZE; \
    s = realloc(s, sz); \
    if (!s) { \
      return 0; \
    } \
    addedChars = expression; \
  } \
  chars += addedChars; \
}

  char *s;
  size_t sz;
  size_t chars;
  enum NmeaSentence msk;

  if (!buf) {
    return 0;
  }

  *buf = NULL;

  if (!info //
      || !mask) {
    return 0;
  }

  sz = NMEALIB_BUFFER_CHUNK_SIZE;
  s = malloc(sz);
  if (!s) {
    /* can't be covered in a test */
    return 0;
  }

  chars = 0;
  msk = mask;

  while (msk) {
    if (msk & GPGGA) {
      NmeaGPGGA pack;
      nmeaGPGGAFromInfo(info, &pack);
      generateSentence(nmeaGPGGAGenerate(dst, available, &pack));
      msk &= (enum NmeaSentence) ~GPGGA;
    } else if (msk & GPGSA) {
      NmeaGPGSA pack;
      nmeaGPGSAFromInfo(info, &pack);
      generateSentence(nmeaGPGSAGenerate(dst, available, &pack));
      msk &= (enum NmeaSentence) ~GPGSA;
    } else if (msk & GPGSV) {
      size_t satCount = nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_SATINVIEWCOUNT) ? (size_t) info->satinfo.inViewCount : 0;
      NmeaGPGSV pack;
      size_t sentence;
      size_t sentences = nmeaGPGSVsatellitesToSentencesCount(satCount);

      for (sentence = 0; sentence < sentences; sentence++) {
        nmeaGPGSVFromInfo(info, &pack, sentence);
        generateSentence(nmeaGPGSVGenerate(dst, available, &pack));
      }
      msk &= (enum NmeaSentence) ~GPGSV;
    } else if (msk & GPRMC) {
      NmeaGPRMC pack;
      nmeaGPRMCFromInfo(info, &pack);
      generateSentence(nmeaGPRMCGenerate(dst, available, &pack));
      msk &= (enum NmeaSentence) ~GPRMC;
    } else if (msk & GPVTG) {
      NmeaGPVTG pack;
      nmeaGPVTGFromInfo(info, &pack);
      generateSentence(nmeaGPVTGGenerate(dst, available, &pack));
      msk &= (enum NmeaSentence) ~GPVTG;
    } else {
      /* no more known sentences to process */
      break;
    }
  }

  if (!chars) {
    free(s);
    s = NULL;
  } else {
    s[chars] = '\0';
  }

  *buf = s;

  return chars;

#undef generateSentence
#undef available
#undef dst

}
