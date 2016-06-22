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

#include <nmea/gpgsv.h>

#include <nmea/context.h>
#include <nmea/info.h>
#include <nmea/parse.h>
#include <nmea/tok.h>
#include <nmea/validate.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

unsigned int nmeaGPGSVsatellitesToSentencesCount(unsigned int satellites) {
  unsigned int pack_count = satellites / NMEA_SATINPACK;

  if ((satellites % NMEA_SATINPACK) > 0) {
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

  nmea_trace_buff(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  pack->present = 0;
  pack->sentences = INT_MAX;
  pack->sentence = INT_MAX;
  pack->satellites = INT_MAX;
  for (i = 0; i < NMEA_SATINPACK; i++) {
    pack->satellite[i].id = INT_MAX;
    pack->satellite[i].elv = INT_MAX;
    pack->satellite[i].azimuth = INT_MAX;
    pack->satellite[i].sig = INT_MAX;
  }

  /* parse */
  fieldCount = nmea_scanf(s, sz, //
      "$GPGSV,%d,%d,%d," //
          "%d,%d,%d,%d,"//
          "%d,%d,%d,%d,"//
          "%d,%d,%d,%d,"//
          "%d,%d,%d,%d",//
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
    nmea_error("GPGSV parse error: sentence index %d is invalid", pack->sentence);
    goto err;
  }
  if (pack->sentence > pack->sentences) {
    nmea_error("GPGSV parse error: sentence index %d is larger than the number of sentences %d", pack->sentence,
        pack->sentences);
    goto err;
  }
  if (nmeaGPGSVsatellitesToSentencesCount((pack->sentences * NMEA_SATINPACK)) != nmeaGPGSVsatellitesToSentencesCount(pack->satellites)) {
    nmea_error("GPGSV parse error: sentence count %d does not correspond to satellite count %d", pack->sentences,
        pack->satellites);
    goto err;
  }

  /* see that there are enough tokens */
  if (pack->sentence != pack->sentences) {
    fieldCountMinimum = 19;
  } else {
    fieldCountMinimum = (4 * (NMEA_SATINPACK - ((pack->sentences * NMEA_SATINPACK) - pack->satellites))) + 3;
  }
  if ((fieldCount != fieldCountMinimum) && (fieldCount != 19)) {
    nmea_error("GPGSV parse error: need %d (or 19) tokens, got %d in '%s'", fieldCountMinimum, fieldCount, s);
    goto err;
  }

  /* validate all satellite settings and count the number of satellites in the sentence */
  for (i = 0; i < NMEA_SATINPACK; i++) {
    nmeaSATELLITE *sat = &pack->satellite[i];
    if ((sat->id == INT_MAX) //
        || (sat->elv == INT_MAX) //
        || (sat->azimuth == INT_MAX) //
        || (sat->sig == INT_MAX)) {
      /* incomplete satellite data */
      memset(sat, 0, sizeof(*sat));
    } else {
      /* complete satellite data */

      if ((sat->id <= 0)) {
        nmea_error("GPGSV parse error: invalid satellite PRN %d in '%s'", sat->id, s);
        memset(sat, 0, sizeof(*sat));
      } else if ((sat->elv < -180) || (sat->elv > 180)) {
        nmea_error("GPGSV parse error: invalid satellite elevation %d in '%s'", sat->elv, s);
        memset(sat, 0, sizeof(*sat));
      } else if ((sat->azimuth < 0) || (sat->azimuth >= 360)) {
        nmea_error("GPGSV parse error: invalid satellite azimuth %d in '%s'", sat->azimuth, s);
        memset(sat, 0, sizeof(*sat));
      } else if ((sat->sig < 0) || (sat->sig > 99)) {
        nmea_error("GPGSV parse error: invalid satellite signal %d in '%s'", sat->sig, s);
        memset(sat, 0, sizeof(*sat));
      }
    }
  }

  nmea_INFO_set_present(&pack->present, SATINVIEW);

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

  if (pack->sentence > NMEA_NSATPACKS) {
    return;
  }

  if (nmeaGPGSVsatellitesToSentencesCount((pack->sentences * NMEA_SATINPACK)) != nmeaGPGSVsatellitesToSentencesCount(pack->satellites)) {
    return;
  }

  nmea_INFO_set_present(&info->present, SMASK);

  info->smask |= GPGSV;

  if (nmea_INFO_is_present(pack->present, SATINVIEW)) {
    int i;
    int offset;
    int max;

    if (pack->sentence == 1) {
      /* first sentence; clear info satellites */
      memset(info->satinfo.sat, 0, sizeof(info->satinfo.sat));
    }

    /* index of 1st sat in pack */
    offset = (pack->sentence - 1) * NMEA_SATINPACK;

    if (pack->sentence != pack->sentences) {
      max = NMEA_SATINPACK;
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

    info->satinfo.inview = pack->satellites;

    nmea_INFO_set_present(&info->present, SATINVIEW);
  }
}

void nmeaGPGSVFromInfo(const nmeaINFO *info, nmeaGPGSV *pack, unsigned int pack_idx) {
  if (!pack || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (nmea_INFO_is_present(info->present, SATINVIEW)) {
    int offset;
    int i;
    int skipCount;

    pack->satellites = (info->satinfo.inview < NMEA_MAXSAT) ?
        info->satinfo.inview :
        NMEA_MAXSAT;

    pack->sentences = nmeaGPGSVsatellitesToSentencesCount(pack->satellites);

    if ((int) pack_idx >= pack->sentences) {
      pack->sentence = pack->sentences;
    } else {
      pack->sentence = pack_idx + 1;
    }

    /* now skip the first ((pack->pack_index - 1) * NMEA_SATINPACK) in view sats */
    skipCount = ((pack->sentence - 1) * NMEA_SATINPACK);

    offset = 0;
    while ((skipCount > 0) && (offset < NMEA_MAXSAT)) {
      if (info->satinfo.sat[offset].id) {
        skipCount--;
      }
      offset++;
    }

    for (i = 0; (i < NMEA_SATINPACK) && (offset < NMEA_MAXSAT); offset++) {
      if (info->satinfo.sat[offset].id) {
        pack->satellite[i] = info->satinfo.sat[offset];
        i++;
      }
    }

    nmea_INFO_set_present(&pack->present, SATINVIEW);
  }
}
