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
#include <string.h>

/**
 * The type definition for an entry mapping a NMEA sentence prefix to a sentence type
 */
typedef struct {
  const char * prefix;
  const enum NmeaSentence sentence;
} SentencePrefixToType;

/**
 * The map from NMEA sentence prefix to sentence type
 */
static const SentencePrefixToType sentencePrefixToType[] = {
    {
        .prefix = NMEA_PREFIX_GPGGA, //
        .sentence = GPGGA //
    },
    {
        .prefix = NMEA_PREFIX_GPGSA, //
        .sentence = GPGSA //
    },
    {
        .prefix = NMEA_PREFIX_GPGSV, //
        .sentence = GPGSV //
    },
    {
        .prefix = NMEA_PREFIX_GPRMC, //
        .sentence = GPRMC //
    },
    {
        .prefix = NMEA_PREFIX_GPVTG, //
        .sentence = GPVTG //
    },
    {
        .prefix = NULL, //
        .sentence = GPNON //
    }//
};

const char * nmeaSentenceToPrefix(enum NmeaSentence sentence) {
  size_t i = 0;

  while (sentencePrefixToType[i].prefix) {
    if (sentencePrefixToType[i].sentence == sentence) {
      return sentencePrefixToType[i].prefix;
    }

    i++;
  }

  return NULL;
}

enum NmeaSentence nmeaPrefixToSentence(const char *s, const size_t sz) {
  const char * str = s;
  size_t size = sz;
  size_t i = 0;

  if (!str || !size) {
    return GPNON;
  }

  if (*str == '$') {
    str++;
    size--;
  }

  if (size < NMEA_PREFIX_LENGTH) {
    return GPNON;
  }

  while (sentencePrefixToType[i].prefix) {
    if (!strncmp(str, sentencePrefixToType[i].prefix, NMEA_PREFIX_LENGTH)) {
      return sentencePrefixToType[i].sentence;
    }

    i++;
  }

  return GPNON;
}

bool nmeaSentenceToInfo(const char *s, const size_t sz, nmeaINFO *info) {
  switch (nmeaPrefixToSentence(s, sz)) {
    case GPGGA: {
      nmeaGPGGA gpgga;
      if (nmeaGPGGAparse(s, sz, &gpgga)) {
        nmeaGPGGAToInfo(&gpgga, info);
        return true;
      }

      return false;
    }

    case GPGSA: {
      nmeaGPGSA gpgsa;
      if (nmeaGPGSAparse(s, sz, &gpgsa)) {
        nmeaGPGSAToInfo(&gpgsa, info);
        return true;
      }

      return false;
    }

    case GPGSV: {
      nmeaGPGSV gpgsv;
      if (nmeaGPGSVparse(s, sz, &gpgsv)) {
        nmeaGPGSVToInfo(&gpgsv, info);
        return true;
      }

      return false;
    }

    case GPRMC: {
      nmeaGPRMC gprmc;
      if (nmeaGPRMCparse(s, sz, &gprmc)) {
        nmeaGPRMCToInfo(&gprmc, info);
        return true;
      }

      return false;
    }

    case GPVTG: {
      nmeaGPVTG gpvtg;
      if (nmeaGPVTGparse(s, sz, &gpvtg)) {
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

int nmeaSentenceGenerate(char *s, const size_t sz, const nmeaINFO *info, const enum NmeaSentence mask) {
  int chars = 0;
  int msk = mask;

  if (!s || !sz || !info || !mask) {
    return 0;
  }

  while (msk) {
    if (msk & GPGGA) {
      nmeaGPGGA pack;
      nmeaGPGGAFromInfo(info, &pack);
      chars += nmeaGPGGAgenerate(&s[chars], sz - chars, &pack);
      msk &= ~GPGGA;
    } else if (msk & GPGSA) {
      nmeaGPGSA pack;
      nmeaGPGSAFromInfo(info, &pack);
      chars += nmeaGPGSAgenerate(&s[chars], sz - chars, &pack);
      msk &= ~GPGSA;
    } else if (msk & GPGSV) {
      nmeaGPGSV pack;
      int sentence;
      int sentences = nmeaGPGSVsatellitesToSentencesCount(info->satinfo.inview);

      for (sentence = 0; (sentence < sentences) && ((sz - chars) > 0); sentence++) {
        nmeaGPGSVFromInfo(info, &pack, sentence);
        chars += nmeaGPGSVgenerate(&s[chars], sz - chars, &pack);
      }
      msk &= ~GPGSV;
    } else if (msk & GPRMC) {
      nmeaGPRMC pack;
      nmeaGPRMCFromInfo(info, &pack);
      chars += nmeaGPRMCgenerate(&s[chars], sz - chars, &pack);
      msk &= ~GPRMC;
    } else if (msk & GPVTG) {
      nmeaGPVTG pack;
      nmeaGPVTGFromInfo(info, &pack);
      chars += nmeaGPVTGgenerate(&s[chars], sz - chars, &pack);
      msk &= ~GPVTG;
    } else {
      /* no more known sentences to process */
      break;
    }

    if ((sz - chars) <= 0) {
      break;
    }
  }

  s[sz - 1] = '\0';

  return chars;
}
