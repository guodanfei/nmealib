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

bool nmeaGPGGAParse(const char *s, const size_t sz, NmeaGPGGA *pack) {
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

    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_UTCTIME);
  } else {
    memset(&pack->time, 0, sizeof(pack->time));
  }

  if (!isnan(pack->latitude)) {
    if (!nmeaValidateNSEW(pack->ns, true, NMEALIB_PREFIX_GPGGA, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_LAT);
  } else {
    pack->latitude = 0.0;
    pack->ns = '\0';
  }

  if (!isnan(pack->longitude)) {
    if (!nmeaValidateNSEW(pack->ew, false, NMEALIB_PREFIX_GPGGA, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_LON);
  } else {
    pack->longitude = 0.0;
    pack->ew = '\0';
  }

  if (pack->signal != INT_MAX) {
    if (!nmeaValidateSignal(pack->signal, NMEALIB_PREFIX_GPGGA, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_SIG);
  } else {
    pack->signal = NMEALIB_SIG_INVALID;
  }

  if (pack->satellitesInView != INT_MAX) {
    pack->satellitesInView = abs(pack->satellitesInView);
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT);
  } else {
    pack->satellitesInView = 0;
  }

  if (!isnan(pack->hdop)) {
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_HDOP);
  } else {
    pack->hdop = 0.0;
  }

  if (!isnan(pack->elevation)) {
    if (pack->elevationUnit != 'M') {
      nmeaError(NMEALIB_PREFIX_GPGGA " parse error: invalid elevation unit '%c' in '%s'", pack->elevationUnit, s);
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_ELV);
  } else {
    pack->elevation = 0.0;
    pack->elevationUnit = '\0';
  }

  if (!isnan(pack->height)) {
    if (pack->heightUnit != 'M') {
      nmeaError(NMEALIB_PREFIX_GPGGA " parse error: invalid height unit '%c' in '%s'", pack->heightUnit, s);
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_HEIGHT);
  } else {
    pack->height = 0.0;
    pack->heightUnit = '\0';
  }

  if (!isnan(pack->dgpsAge)) {
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_DGPSAGE);
  } else {
    pack->dgpsAge = 0.0;
  }

  if (pack->dgpsSid != INT_MAX) {
    pack->dgpsSid = abs(pack->dgpsSid);
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_DGPSSID);
  } else {
    pack->dgpsSid = 0;
  }

  return true;

err:
  memset(pack, 0, sizeof(*pack));
  pack->signal = NMEALIB_SIG_INVALID;
  return false;
}

void nmeaGPGGAToInfo(const NmeaGPGGA *pack, NmeaInfo *info) {
  if (!pack //
      || !info) {
    return;
  }

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SMASK);

  info->smask |= NMEALIB_SENTENCE_GPGGA;

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_UTCTIME)) {
    info->utc.hour = pack->time.hour;
    info->utc.min = pack->time.min;
    info->utc.sec = pack->time.sec;
    info->utc.hsec = pack->time.hsec;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_UTCTIME);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_LAT)) {
    info->lat = ((pack->ns == 'S') ?
        -pack->latitude :
        pack->latitude);
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LAT);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_LON)) {
    info->lon = ((pack->ew == 'W') ?
        -pack->longitude :
        pack->longitude);
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LON);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SIG)) {
    info->sig = pack->signal;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SIG);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT)) {
    info->satinfo.inViewCount = pack->satellitesInView;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEWCOUNT);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_HDOP)) {
    info->hdop = pack->hdop;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_HDOP);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_ELV)) {
    info->elv = pack->elevation;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_ELV);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_HEIGHT)) {
    info->height = pack->height;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_HEIGHT);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_DGPSAGE)) {
    info->dgpsAge = pack->dgpsAge;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_DGPSAGE);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_DGPSSID)) {
    info->dgpsSid = pack->dgpsSid;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_DGPSSID);
  }
}

void nmeaGPGGAFromInfo(const NmeaInfo *info, NmeaGPGGA *pack) {
  if (!pack //
      || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_UTCTIME)) {
    pack->time.hour = info->utc.hour;
    pack->time.min = info->utc.min;
    pack->time.sec = info->utc.sec;
    pack->time.hsec = info->utc.hsec;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_UTCTIME);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_LAT)) {
    pack->latitude = fabs(info->lat);
    pack->ns = ((info->lat >= 0.0) ?
        'N' :
        'S');
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_LAT);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_LON)) {
    pack->longitude = fabs(info->lon);
    pack->ew = ((info->lon >= 0.0) ?
        'E' :
        'W');
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_LON);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_SIG)) {
    pack->signal = info->sig;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_SIG);
  } else {
    pack->signal = NMEALIB_SIG_INVALID;
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_SATINVIEWCOUNT)) {
    pack->satellitesInView = info->satinfo.inViewCount;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_HDOP)) {
    pack->hdop = info->hdop;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_HDOP);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_ELV)) {
    pack->elevation = info->elv;
    pack->elevationUnit = 'M';
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_ELV);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_HEIGHT)) {
    pack->height = info->height;
    pack->heightUnit = 'M';
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_HEIGHT);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_DGPSAGE)) {
    pack->dgpsAge = info->dgpsAge;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_DGPSAGE);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_DGPSSID)) {
    pack->dgpsSid = info->dgpsSid;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_DGPSSID);
  }
}

size_t nmeaGPGGAGenerate(char *s, const size_t sz, const NmeaGPGGA *pack) {

#define dst       (&s[chars])
#define available ((sz <= (size_t) chars) ? 0 : (sz - (size_t) chars))

  int chars = 0;

  if (!s //
      || !pack) {
    return 0;
  }

  chars += snprintf(dst, available, "$" NMEALIB_PREFIX_GPGGA);

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_UTCTIME)) {
    chars += snprintf(dst, available, //
        ",%02u%02u%02u.%02u", //
        pack->time.hour, //
        pack->time.min, //
        pack->time.sec, //
        pack->time.hsec);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_LAT)) {
    chars += snprintf(dst, available, ",%09.4f", pack->latitude);
    if (pack->ns) {
      chars += snprintf(dst, available, ",%c", pack->ns);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_LON)) {
    chars += snprintf(dst, available, ",%010.4f", pack->longitude);
    if (pack->ew) {
      chars += snprintf(dst, available, ",%c", pack->ew);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SIG)) {
    chars += snprintf(dst, available, ",%d", pack->signal);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT)) {
    chars += snprintf(dst, available, ",%02d", pack->satellitesInView);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_HDOP)) {
    chars += snprintf(dst, available, ",%03.1f", pack->hdop);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_ELV)) {
    chars += snprintf(dst, available, ",%03.1f", pack->elevation);
    if (pack->elevationUnit) {
      chars += snprintf(dst, available, ",%c", pack->elevationUnit);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_HEIGHT)) {
    chars += snprintf(dst, available, ",%03.1f", pack->height);
    if (pack->heightUnit) {
      chars += snprintf(dst, available, ",%c", pack->heightUnit);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_DGPSAGE)) {
    chars += snprintf(dst, available, ",%03.1f", pack->dgpsAge);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_DGPSSID)) {
    chars += snprintf(dst, available, ",%d", pack->dgpsSid);
  } else {
    chars += snprintf(dst, available, ",");
  }

  /* checksum */
  chars += nmeaAppendChecksum(s, sz, (size_t) chars);

  return (size_t) chars;

#undef available
#undef dst

}
