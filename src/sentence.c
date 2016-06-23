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
  const enum NmeaSentence sentenceType;
} SentencePrefixToType;

/**
 * The map from NMEA sentence prefix to sentence type
 */
static const SentencePrefixToType sentencePrefixToType[] = {
    {
        .prefix = "GPGGA", //
        .sentenceType = GPGGA //
    },
    {
        .prefix = "GPGSA", //
        .sentenceType = GPGSA //
    },
    {
        .prefix = "GPGSV", //
        .sentenceType = GPGSV //
    },
    {
        .prefix = "GPRMC", //
        .sentenceType = GPRMC //
    },
    {
        .prefix = "GPVTG", //
        .sentenceType = GPVTG //
    },
    {
        .prefix = NULL, //
        .sentenceType = GPNON //
    }//
};

const char * nmeaSentenceToPrefix(enum NmeaSentence sentence) {
  size_t i = 0;

  while (sentencePrefixToType[i].prefix) {
    if (sentencePrefixToType[i].sentenceType == sentence) {
      return sentencePrefixToType[i].prefix;
    }

    i++;
  }

  return NULL;
}

enum NmeaSentence nmeaPrefixToSentence(const char *s, const size_t sz) {
  size_t i = 0;
  const char * str = s;
  size_t size = sz;

  if (!str) {
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
      return sentencePrefixToType[i].sentenceType;
    }

    i++;
  }

  return GPNON;
}

bool nmeaSentenceToInfo(const char *s, const size_t sz, nmeaINFO * info) {
  enum NmeaSentence sentence = nmeaPrefixToSentence(s, sz);
  switch (sentence) {
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

int nmea_generate(char *s, const int len, const nmeaINFO *info, const int generate_mask) {
  int gen_count = 0;
  int pack_mask = generate_mask;

  if (!s || !len || !info || !generate_mask)
    return 0;

  while (pack_mask) {
    if (pack_mask & GPGGA) {
      nmeaGPGGA gpgga;

      nmeaGPGGAFromInfo(info, &gpgga);
      gen_count += nmea_gen_GPGGA(s + gen_count, len - gen_count, &gpgga);
      pack_mask &= ~GPGGA;
    } else if (pack_mask & GPGSA) {
      nmeaGPGSA gpgsa;

      nmeaGPGSAFromInfo(info, &gpgsa);
      gen_count += nmea_gen_GPGSA(s + gen_count, len - gen_count, &gpgsa);
      pack_mask &= ~GPGSA;
    } else if (pack_mask & GPGSV) {
      nmeaGPGSV gpgsv;
      int gpgsv_it;
      int gpgsv_count = nmeaGPGSVsatellitesToSentencesCount(info->satinfo.inview);

      for (gpgsv_it = 0; gpgsv_it < gpgsv_count && len - gen_count > 0; gpgsv_it++) {
        nmeaGPGSVFromInfo(info, &gpgsv, gpgsv_it);
        gen_count += nmea_gen_GPGSV(s + gen_count, len - gen_count, &gpgsv);
      }
      pack_mask &= ~GPGSV;
    } else if (pack_mask & GPRMC) {
      nmeaGPRMC gprmc;

      nmeaGPRMCFromInfo(info, &gprmc);
      gen_count += nmea_gen_GPRMC(s + gen_count, len - gen_count, &gprmc);
      pack_mask &= ~GPRMC;
    } else if (pack_mask & GPVTG) {
      nmeaGPVTG gpvtg;

      nmeaGPVTGFromInfo(info, &gpvtg);
      gen_count += nmea_gen_GPVTG(s + gen_count, len - gen_count, &gpvtg);
      pack_mask &= ~GPVTG;
    } else {
      /* no more known sentences to process */
      break;
    }

    if (len - gen_count <= 0)
      break;
  }

  return gen_count;
}

