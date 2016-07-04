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

#include <nmealib/gpgga.h>

#include <nmealib/context.h>
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

bool nmeaGPGGAParse(const char *s, const size_t sz, nmeaGPGGA *pack) {
  size_t fieldCount;
  char timeBuf[16];

  if (!s //
      || !sz //
      || !pack) {
    return false;
  }

  nmeaTraceBuffer(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  *timeBuf = '\0';
  memset(pack, 0, sizeof(*pack));
  pack->latitude = NAN;
  pack->longitude = NAN;
  pack->signal = INT_MAX;
  pack->satellitesInView = INT_MAX;
  pack->hdop = NAN;
  pack->elevation = NAN;
  pack->height = NAN;
  pack->dgpsAge = NAN;
  pack->dgpsSid = INT_MAX;

  /* parse */
  fieldCount = nmeaScanf(s, sz, //
      "$" NMEALIB_PREFIX_GPGGA ",%16s,%F,%C,%F,%C,%d,%d,%F,%f,%C,%f,%C,%F,%d*", //
      timeBuf, //
      &pack->latitude, //
      &pack->ns, //
      &pack->longitude, //
      &pack->ew, //
      &pack->signal, //
      &pack->satellitesInView, //
      &pack->hdop, //
      &pack->elevation, //
      &pack->elevationUnit, //
      &pack->height, //
      &pack->heightUnit, //
      &pack->dgpsAge, //
      &pack->dgpsSid);

  /* see that there are enough tokens */
  if (fieldCount != 14) {
    nmeaError(NMEALIB_PREFIX_GPGGA " parse error: need 14 tokens, got %lu in '%s'", (long unsigned) fieldCount, s);
    goto err;
  }

  /* determine which fields are present and validate them */

  if (*timeBuf) {
    if (!nmeaTimeParseTime(timeBuf, &pack->time) //
        || !nmeaValidateTime(&pack->time, NMEALIB_PREFIX_GPGGA, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, UTCTIME);
  } else {
    memset(&pack->time, 0, sizeof(pack->time));
  }

  if (!isnan(pack->latitude)) {
    if (!nmeaValidateNSEW(pack->ns, true, NMEALIB_PREFIX_GPGGA, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, LAT);
  } else {
    pack->latitude = 0.0;
    pack->ns = '\0';
  }

  if (!isnan(pack->longitude)) {
    if (!nmeaValidateNSEW(pack->ew, false, NMEALIB_PREFIX_GPGGA, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, LON);
  } else {
    pack->longitude = 0.0;
    pack->ew = '\0';
  }

  if (pack->signal != INT_MAX) {
    if (!nmeaValidateSignal(pack->signal, NMEALIB_PREFIX_GPGGA, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, SIG);
  } else {
    pack->signal = NMEALIB_SIG_INVALID;
  }

  if (pack->satellitesInView != INT_MAX) {
    pack->satellitesInView = abs(pack->satellitesInView);
    nmeaInfoSetPresent(&pack->present, SATINVIEWCOUNT);
  } else {
    pack->satellitesInView = 0;
  }

  if (!isnan(pack->hdop)) {
    nmeaInfoSetPresent(&pack->present, HDOP);
  } else {
    pack->hdop = 0.0;
  }

  if (!isnan(pack->elevation)) {
    if (pack->elevationUnit != 'M') {
      nmeaError(NMEALIB_PREFIX_GPGGA " parse error: invalid elevation unit '%c' in '%s'", pack->elevationUnit, s);
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, ELV);
  } else {
    pack->elevation = 0.0;
    pack->elevationUnit = '\0';
  }

  if (!isnan(pack->height)) {
    if (pack->heightUnit != 'M') {
      nmeaError(NMEALIB_PREFIX_GPGGA " parse error: invalid height unit '%c' in '%s'", pack->heightUnit, s);
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, HEIGHT);
  } else {
    pack->height = 0.0;
    pack->heightUnit = '\0';
  }

  if (!isnan(pack->dgpsAge)) {
    nmeaInfoSetPresent(&pack->present, DGPSAGE);
  } else {
    pack->dgpsAge = 0.0;
  }

  if (pack->dgpsSid != INT_MAX) {
    pack->dgpsSid = abs(pack->dgpsSid);
    nmeaInfoSetPresent(&pack->present, DGPSSID);
  } else {
    pack->dgpsSid = 0;
  }

  return true;

err:
  memset(pack, 0, sizeof(*pack));
  pack->signal = NMEALIB_SIG_INVALID;
  return false;
}

void nmeaGPGGAToInfo(const nmeaGPGGA *pack, NmeaInfo *info) {
  if (!pack //
      || !info) {
    return;
  }

  nmeaInfoSetPresent(&info->present, SMASK);

  info->smask |= GPGGA;

  if (nmeaInfoIsPresentAll(pack->present, UTCTIME)) {
    info->utc.hour = pack->time.hour;
    info->utc.min = pack->time.min;
    info->utc.sec = pack->time.sec;
    info->utc.hsec = pack->time.hsec;
    nmeaInfoSetPresent(&info->present, UTCTIME);
  }

  if (nmeaInfoIsPresentAll(pack->present, LAT)) {
    info->lat = ((pack->ns == 'S') ?
        -pack->latitude :
        pack->latitude);
    nmeaInfoSetPresent(&info->present, LAT);
  }

  if (nmeaInfoIsPresentAll(pack->present, LON)) {
    info->lon = ((pack->ew == 'W') ?
        -pack->longitude :
        pack->longitude);
    nmeaInfoSetPresent(&info->present, LON);
  }

  if (nmeaInfoIsPresentAll(pack->present, SIG)) {
    info->sig = pack->signal;
    nmeaInfoSetPresent(&info->present, SIG);
  }

  if (nmeaInfoIsPresentAll(pack->present, SATINVIEWCOUNT)) {
    info->satinfo.inViewCount = pack->satellitesInView;
    nmeaInfoSetPresent(&info->present, SATINVIEWCOUNT);
  }

  if (nmeaInfoIsPresentAll(pack->present, HDOP)) {
    info->hdop = pack->hdop;
    nmeaInfoSetPresent(&info->present, HDOP);
  }

  if (nmeaInfoIsPresentAll(pack->present, ELV)) {
    info->elv = pack->elevation;
    nmeaInfoSetPresent(&info->present, ELV);
  }

  if (nmeaInfoIsPresentAll(pack->present, HEIGHT)) {
    info->height = pack->height;
    nmeaInfoSetPresent(&info->present, HEIGHT);
  }

  if (nmeaInfoIsPresentAll(pack->present, DGPSAGE)) {
    info->dgpsAge = pack->dgpsAge;
    nmeaInfoSetPresent(&info->present, DGPSAGE);
  }

  if (nmeaInfoIsPresentAll(pack->present, DGPSSID)) {
    info->dgpsSid = pack->dgpsSid;
    nmeaInfoSetPresent(&info->present, DGPSSID);
  }
}

void nmeaGPGGAFromInfo(const NmeaInfo *info, nmeaGPGGA *pack) {
  if (!pack //
      || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (nmeaInfoIsPresentAll(info->present, UTCTIME)) {
    pack->time.hour = info->utc.hour;
    pack->time.min = info->utc.min;
    pack->time.sec = info->utc.sec;
    pack->time.hsec = info->utc.hsec;
    nmeaInfoSetPresent(&pack->present, UTCTIME);
  }

  if (nmeaInfoIsPresentAll(info->present, LAT)) {
    pack->latitude = fabs(info->lat);
    pack->ns = ((info->lat >= 0.0) ?
        'N' :
        'S');
    nmeaInfoSetPresent(&pack->present, LAT);
  }

  if (nmeaInfoIsPresentAll(info->present, LON)) {
    pack->longitude = fabs(info->lon);
    pack->ew = ((info->lon >= 0.0) ?
        'E' :
        'W');
    nmeaInfoSetPresent(&pack->present, LON);
  }

  if (nmeaInfoIsPresentAll(info->present, SIG)) {
    pack->signal = info->sig;
    nmeaInfoSetPresent(&pack->present, SIG);
  } else {
    pack->signal = NMEALIB_SIG_INVALID;
  }

  if (nmeaInfoIsPresentAll(info->present, SATINVIEWCOUNT)) {
    pack->satellitesInView = info->satinfo.inViewCount;
    nmeaInfoSetPresent(&pack->present, SATINVIEWCOUNT);
  }

  if (nmeaInfoIsPresentAll(info->present, HDOP)) {
    pack->hdop = info->hdop;
    nmeaInfoSetPresent(&pack->present, HDOP);
  }

  if (nmeaInfoIsPresentAll(info->present, ELV)) {
    pack->elevation = info->elv;
    pack->elevationUnit = 'M';
    nmeaInfoSetPresent(&pack->present, ELV);
  }

  if (nmeaInfoIsPresentAll(info->present, HEIGHT)) {
    pack->height = info->height;
    pack->heightUnit = 'M';
    nmeaInfoSetPresent(&pack->present, HEIGHT);
  }

  if (nmeaInfoIsPresentAll(info->present, DGPSAGE)) {
    pack->dgpsAge = info->dgpsAge;
    nmeaInfoSetPresent(&pack->present, DGPSAGE);
  }

  if (nmeaInfoIsPresentAll(info->present, DGPSSID)) {
    pack->dgpsSid = info->dgpsSid;
    nmeaInfoSetPresent(&pack->present, DGPSSID);
  }
}

size_t nmeaGPGGAGenerate(char *s, const size_t sz, const nmeaGPGGA *pack) {

#define dst       (&s[chars])
#define available ((size_t) MAX((long) sz - 1 - chars, 0))

  int chars = 0;

  if (!s //
      || !pack) {
    return 0;
  }

  chars += snprintf(dst, available, "$" NMEALIB_PREFIX_GPGGA);

  if (nmeaInfoIsPresentAll(pack->present, UTCTIME)) {
    chars += snprintf(dst, available, //
        ",%02u%02u%02u.%02u", //
        pack->time.hour, //
        pack->time.min, //
        pack->time.sec, //
        pack->time.hsec);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, LAT)) {
    chars += snprintf(dst, available, ",%09.4f", pack->latitude);
    if (pack->ns) {
      chars += snprintf(dst, available, ",%c", pack->ns);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, LON)) {
    chars += snprintf(dst, available, ",%010.4f", pack->longitude);
    if (pack->ew) {
      chars += snprintf(dst, available, ",%c", pack->ew);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, SIG)) {
    chars += snprintf(dst, available, ",%d", pack->signal);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, SATINVIEWCOUNT)) {
    chars += snprintf(dst, available, ",%02d", pack->satellitesInView);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, HDOP)) {
    chars += snprintf(dst, available, ",%03.1f", pack->hdop);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, ELV)) {
    chars += snprintf(dst, available, ",%03.1f", pack->elevation);
    if (pack->elevationUnit) {
      chars += snprintf(dst, available, ",%c", pack->elevationUnit);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, HEIGHT)) {
    chars += snprintf(dst, available, ",%03.1f", pack->height);
    if (pack->heightUnit) {
      chars += snprintf(dst, available, ",%c", pack->heightUnit);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, DGPSAGE)) {
    chars += snprintf(dst, available, ",%03.1f", pack->dgpsAge);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, DGPSSID)) {
    chars += snprintf(dst, available, ",%d", pack->dgpsSid);
  } else {
    chars += snprintf(dst, available, ",");
  }

  /* checksum */
  chars += nmeaAppendChecksum(s, sz, (size_t) chars);

  return (size_t) chars;
}
