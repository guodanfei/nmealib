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

#include <nmealib/gpvtg.h>

#include <nmealib/context.h>
#include <nmealib/gmath.h>
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

bool nmeaGPVTGParse(const char *s, const size_t sz, nmeaGPVTG *pack) {
  size_t fieldCount;
  bool speedK = false;
  bool speedN = false;

  if (!s //
      || !sz //
      || !pack) {
    return false;
  }

  nmeaTraceBuffer(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  memset(pack, 0, sizeof(*pack));
  pack->track = NAN;
  pack->mtrack = NAN;
  pack->spn = NAN;
  pack->spk = NAN;

  /* parse */
  fieldCount = nmeaScanf(s, sz, //
      "$" NMEA_PREFIX_GPVTG ",%f,%C,%f,%C,%f,%C,%f,%C*", //
      &pack->track, //
      &pack->track_t, //
      &pack->mtrack, //
      &pack->mtrack_m, //
      &pack->spn, //
      &pack->spn_n, //
      &pack->spk, //
      &pack->spk_k);

  /* see that there are enough tokens */
  if (fieldCount != 8) {
    nmeaError(NMEA_PREFIX_GPVTG " parse error: need 8 tokens, got %lu in '%s'", (long unsigned) fieldCount, s);
    goto err;
  }

  /* determine which fields are present and validate them */

  if (!isnan(pack->track)) {
    if (pack->track_t != 'T') {
      nmeaError(NMEA_PREFIX_GPVTG " parse error: invalid track unit, got '%c', expected 'T'", pack->track_t);
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, TRACK);
  } else {
    pack->track = 0.0;
    pack->track_t = '\0';
  }

  if (!isnan(pack->mtrack)) {
    if (pack->mtrack_m != 'M') {
      nmeaError(NMEA_PREFIX_GPVTG " parse error: invalid mtrack unit, got '%c', expected 'M'", pack->mtrack_m);
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, MTRACK);
  } else {
    pack->mtrack = 0.0;
    pack->mtrack_m = '\0';
  }

  if (!isnan(pack->spn)) {
    if (pack->spn_n != 'N') {
      nmeaError(NMEA_PREFIX_GPVTG " parse error: invalid knots speed unit, got '%c', expected 'N'", pack->spn_n);
      goto err;
    }

    speedN = true;
    nmeaInfoSetPresent(&pack->present, SPEED);
  } else {
    pack->spn = 0.0;
    pack->spn_n = '\0';
  }

  if (!isnan(pack->spk)) {
    if (pack->spk_k != 'K') {
      nmeaError(NMEA_PREFIX_GPVTG " parse error: invalid kph speed unit, got '%c', expected 'K'", pack->spk_k);
      goto err;
    }

    speedK = true;
    nmeaInfoSetPresent(&pack->present, SPEED);
  } else {
    pack->spk = 0.0;
    pack->spk_k = '\0';
  }

  if (!speedK && speedN) {
    pack->spk = pack->spn * NMEA_TUD_KNOTS;
    pack->spk_k = 'K';
  } else if (speedK && !speedN) {
    pack->spn = pack->spk / NMEA_TUD_KNOTS;
    pack->spn_n = 'N';
  }

  return true;

err:
  memset(pack, 0, sizeof(*pack));
  return false;
}

void nmeaGPVTGToInfo(const nmeaGPVTG *pack, NmeaInfo *info) {
  if (!pack //
      || !info) {
    return;
  }

  nmeaInfoSetPresent(&info->present, SMASK);

  info->smask |= GPVTG;

  if (nmeaInfoIsPresentAll(pack->present, TRACK)) {
    info->track = pack->track;
    nmeaInfoSetPresent(&info->present, TRACK);
  }

  if (nmeaInfoIsPresentAll(pack->present, MTRACK)) {
    info->mtrack = pack->mtrack;
    nmeaInfoSetPresent(&info->present, MTRACK);
  }

  if (nmeaInfoIsPresentAll(pack->present, SPEED)) {
    double speed;
    if (pack->spk_k) {
      speed = pack->spk;
    } else {
      speed = pack->spn * NMEA_TUD_KNOTS;
    }
    info->speed = speed;
    nmeaInfoSetPresent(&info->present, SPEED);
  }
}

void nmeaGPVTGFromInfo(const NmeaInfo *info, nmeaGPVTG *pack) {
  if (!pack //
      || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (nmeaInfoIsPresentAll(info->present, TRACK)) {
    pack->track = info->track;
    pack->track_t = 'T';
    nmeaInfoSetPresent(&pack->present, TRACK);
  }

  if (nmeaInfoIsPresentAll(info->present, MTRACK)) {
    pack->mtrack = info->mtrack;
    pack->mtrack_m = 'M';
    nmeaInfoSetPresent(&pack->present, MTRACK);
  }

  if (nmeaInfoIsPresentAll(info->present, SPEED)) {
    pack->spn = info->speed / NMEA_TUD_KNOTS;
    pack->spn_n = 'N';
    pack->spk = info->speed;
    pack->spk_k = 'K';
    nmeaInfoSetPresent(&pack->present, SPEED);
  }
}

int nmeaGPVTGGenerate(char *s, const size_t sz, const nmeaGPVTG *pack) {

#define dst       (&s[chars])
#define available ((size_t) MAX((long) sz - 1 - chars, 0))

  int chars = 0;

  if (!s //
      || !pack) {
    return 0;
  }

  chars += snprintf(dst, available, "$" NMEA_PREFIX_GPVTG);

  if (nmeaInfoIsPresentAll(pack->present, TRACK)) {
    chars += snprintf(dst, available, ",%03.1f,%c", pack->track, pack->track_t);
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, MTRACK)) {
    chars += snprintf(dst, available, ",%03.1f,%c", pack->mtrack, pack->mtrack_m);
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, SPEED)) {
    if (pack->spn_n) {
      chars += snprintf(dst, available, ",%03.1f,N", pack->spn);
    } else {
      chars += snprintf(dst, available, ",,");
    }
    if (pack->spk_k) {
      chars += snprintf(dst, available, ",%03.1f,K", pack->spk);
    } else {
      chars += snprintf(dst, available, ",,");
    }
  } else {
    chars += snprintf(dst, available, ",,,,");
  }

  /* checksum */
  chars += nmeaAppendChecksum(s, sz, chars);

  return chars;
}
