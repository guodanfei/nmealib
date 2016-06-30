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
      "$" NMEA_PREFIX_GPGGA ",%16s,%f,%C,%f,%C,%d,%d,%f,%f,%C,%f,%C,%f,%d*", //
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
    nmeaError(NMEA_PREFIX_GPGGA " parse error: need 14 tokens, got %lu in '%s'", (long unsigned) fieldCount, s);
    goto err;
  }

  /* determine which fields are present and validate them */

  if (*timeBuf) {
    if (!nmeaTIMEparseTime(timeBuf, &pack->time) //
        || !nmeaValidateTime(&pack->time, NMEA_PREFIX_GPGGA, s)) {
      goto err;
    }

    nmea_INFO_set_present(&pack->present, UTCTIME);
  } else {
    memset(&pack->time, 0, sizeof(pack->time));
  }

  if (!isnan(pack->latitude)) {
    if (!nmeaValidateNSEW(&pack->ns, true, NMEA_PREFIX_GPGGA, s)) {
      goto err;
    }

    pack->latitude = fabs(pack->latitude);
    nmea_INFO_set_present(&pack->present, LAT);
  } else {
    pack->latitude = 0.0;
    pack->ns = '\0';
  }

  if (!isnan(pack->longitude)) {
    if (!nmeaValidateNSEW(&pack->ew, false, NMEA_PREFIX_GPGGA, s)) {
      goto err;
    }

    pack->longitude = fabs(pack->longitude);
    nmea_INFO_set_present(&pack->present, LON);
  } else {
    pack->longitude = 0.0;
    pack->ew = '\0';
  }

  if (pack->signal != INT_MAX) {
    if (!nmeaValidateSignal(&pack->signal, NMEA_PREFIX_GPGGA, s)) {
      goto err;
    }

    nmea_INFO_set_present(&pack->present, SIG);
  } else {
    pack->signal = NMEA_SIG_INVALID;
  }

  if (pack->satellitesInView != INT_MAX) {
    pack->satellitesInView = abs(pack->satellitesInView);
    nmea_INFO_set_present(&pack->present, SATINVIEWCOUNT);
  } else {
    pack->satellitesInView = 0;
  }

  if (!isnan(pack->hdop)) {
    pack->hdop = fabs(pack->hdop);
    nmea_INFO_set_present(&pack->present, HDOP);
  } else {
    pack->hdop = 0.0;
  }

  if (!isnan(pack->elevation)) {
    if (pack->elevationUnit != 'M') {
      nmeaError(NMEA_PREFIX_GPGGA " parse error: invalid elevation unit '%c' in '%s'", pack->elevationUnit, s);
      goto err;
    }

    nmea_INFO_set_present(&pack->present, ELV);
  } else {
    pack->elevation = 0.0;
    pack->elevationUnit = '\0';
  }

  if (!isnan(pack->height)) {
    if (pack->heightUnit != 'M') {
      nmeaError(NMEA_PREFIX_GPGGA " parse error: invalid height unit '%c' in '%s'", pack->heightUnit, s);
      goto err;
    }

    nmea_INFO_set_present(&pack->present, HEIGHT);
  } else {
    pack->height = 0.0;
    pack->heightUnit = '\0';
  }

  if (!isnan(pack->dgpsAge)) {
    pack->dgpsAge = fabs(pack->dgpsAge);
    nmea_INFO_set_present(&pack->present, DGPSAGE);
  } else {
    pack->dgpsAge = 0.0;
  }

  if (pack->dgpsSid != INT_MAX) {
    pack->dgpsSid = abs(pack->dgpsSid);
    nmea_INFO_set_present(&pack->present, DGPSSID);
  } else {
    pack->dgpsSid = 0;
  }

  return true;

err:
  memset(pack, 0, sizeof(*pack));
  pack->signal = NMEA_SIG_INVALID;
  return false;
}

void nmeaGPGGAToInfo(const nmeaGPGGA *pack, nmeaINFO *info) {
  if (!pack //
      || !info) {
    return;
  }

  nmea_INFO_set_present(&info->present, SMASK);

  info->smask |= GPGGA;

  if (nmea_INFO_is_present(pack->present, UTCTIME)) {
    info->utc.hour = pack->time.hour;
    info->utc.min = pack->time.min;
    info->utc.sec = pack->time.sec;
    info->utc.hsec = pack->time.hsec;
    nmea_INFO_set_present(&info->present, UTCTIME);
  }

  if (nmea_INFO_is_present(pack->present, LAT)) {
    info->lat = ((pack->ns == 'S') ?
        -fabs(pack->latitude) :
        fabs(pack->latitude));
    nmea_INFO_set_present(&info->present, LAT);
  }

  if (nmea_INFO_is_present(pack->present, LON)) {
    info->lon = ((pack->ew == 'W') ?
        -fabs(pack->longitude) :
        fabs(pack->longitude));
    nmea_INFO_set_present(&info->present, LON);
  }

  if (nmea_INFO_is_present(pack->present, SIG)) {
    info->sig = pack->signal;
    nmea_INFO_set_present(&info->present, SIG);
  }

  if (nmea_INFO_is_present(pack->present, SATINVIEWCOUNT)) {
    info->satinfo.inview = pack->satellitesInView;
    nmea_INFO_set_present(&info->present, SATINVIEWCOUNT);
  }

  if (nmea_INFO_is_present(pack->present, HDOP)) {
    info->HDOP = fabs(pack->hdop);
    nmea_INFO_set_present(&info->present, HDOP);
  }

  if (nmea_INFO_is_present(pack->present, ELV)) {
    info->elv = pack->elevation;
    nmea_INFO_set_present(&info->present, ELV);
  }

  if (nmea_INFO_is_present(pack->present, HEIGHT)) {
    info->height = pack->height;
    nmea_INFO_set_present(&info->present, HEIGHT);
  }

  if (nmea_INFO_is_present(pack->present, DGPSAGE)) {
    info->dgpsAge = pack->dgpsAge;
    nmea_INFO_set_present(&info->present, DGPSAGE);
  }

  if (nmea_INFO_is_present(pack->present, DGPSSID)) {
    info->dgpsSid = pack->dgpsSid;
    nmea_INFO_set_present(&info->present, DGPSSID);
  }
}

void nmeaGPGGAFromInfo(const nmeaINFO *info, nmeaGPGGA *pack) {
  if (!pack //
      || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (nmea_INFO_is_present(info->present, UTCTIME)) {
    pack->time.hour = info->utc.hour;
    pack->time.min = info->utc.min;
    pack->time.sec = info->utc.sec;
    pack->time.hsec = info->utc.hsec;
    nmea_INFO_set_present(&pack->present, UTCTIME);
  }

  if (nmea_INFO_is_present(info->present, LAT)) {
    pack->latitude = fabs(info->lat);
    pack->ns = ((info->lat >= 0.0) ?
        'N' :
        'S');
    nmea_INFO_set_present(&pack->present, LAT);
  }

  if (nmea_INFO_is_present(info->present, LON)) {
    pack->longitude = fabs(info->lon);
    pack->ew = ((info->lon >= 0.0) ?
        'E' :
        'W');
    nmea_INFO_set_present(&pack->present, LON);
  }

  if (nmea_INFO_is_present(info->present, SIG)) {
    pack->signal = info->sig;
    nmea_INFO_set_present(&pack->present, SIG);
  } else {
    pack->signal = NMEA_SIG_INVALID;
  }

  if (nmea_INFO_is_present(info->present, SATINVIEWCOUNT)) {
    pack->satellitesInView = info->satinfo.inview;
    nmea_INFO_set_present(&pack->present, SATINVIEWCOUNT);
  }

  if (nmea_INFO_is_present(info->present, HDOP)) {
    pack->hdop = info->HDOP;
    nmea_INFO_set_present(&pack->present, HDOP);
  }

  if (nmea_INFO_is_present(info->present, ELV)) {
    pack->elevation = info->elv;
    pack->elevationUnit = 'M';
    nmea_INFO_set_present(&pack->present, ELV);
  }

  if (nmea_INFO_is_present(info->present, HEIGHT)) {
    pack->height = info->height;
    pack->heightUnit = 'M';
    nmea_INFO_set_present(&pack->present, HEIGHT);
  }

  if (nmea_INFO_is_present(info->present, DGPSAGE)) {
    pack->dgpsAge = info->dgpsAge;
    nmea_INFO_set_present(&pack->present, DGPSAGE);
  }

  if (nmea_INFO_is_present(info->present, DGPSSID)) {
    pack->dgpsSid = info->dgpsSid;
    nmea_INFO_set_present(&pack->present, DGPSSID);
  }
}

int nmeaGPGGAGenerate(char *s, const size_t sz, const nmeaGPGGA *pack) {

#define dst       (&s[chars])
#define available ((size_t) MAX((long) sz - 1 - chars, 0))

  int chars = 0;

  if (!s //
      || !pack) {
    return 0;
  }

  chars += snprintf(dst, available, "$" NMEA_PREFIX_GPGGA);

  if (nmea_INFO_is_present(pack->present, UTCTIME)) {
    chars += snprintf(dst, available, //
        ",%02d%02d%02d.%02d", //
        pack->time.hour, //
        pack->time.min, //
        pack->time.sec, //
        pack->time.hsec);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmea_INFO_is_present(pack->present, LAT)) {
    chars += snprintf(dst, available, ",%09.4f", pack->latitude);
    if (pack->ns) {
      chars += snprintf(dst, available, ",%c", pack->ns);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmea_INFO_is_present(pack->present, LON)) {
    chars += snprintf(dst, available, ",%010.4f", pack->longitude);
    if (pack->ew) {
      chars += snprintf(dst, available, ",%c", pack->ew);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmea_INFO_is_present(pack->present, SIG)) {
    chars += snprintf(dst, available, ",%d", pack->signal);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmea_INFO_is_present(pack->present, SATINVIEWCOUNT)) {
    chars += snprintf(dst, available, ",%02d", pack->satellitesInView);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmea_INFO_is_present(pack->present, HDOP)) {
    chars += snprintf(dst, available, ",%03.1f", pack->hdop);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmea_INFO_is_present(pack->present, ELV)) {
    chars += snprintf(dst, available, ",%03.1f", pack->elevation);
    if (pack->elevationUnit) {
      chars += snprintf(dst, available, ",%c", pack->elevationUnit);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmea_INFO_is_present(pack->present, HEIGHT)) {
    chars += snprintf(dst, available, ",%03.1f", pack->height);
    if (pack->heightUnit) {
      chars += snprintf(dst, available, ",%c", pack->heightUnit);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmea_INFO_is_present(pack->present, DGPSAGE)) {
    chars += snprintf(dst, available, ",%03.1f", pack->dgpsAge);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmea_INFO_is_present(pack->present, DGPSSID)) {
    chars += snprintf(dst, available, ",%d", pack->dgpsSid);
  } else {
    chars += snprintf(dst, available, ",");
  }

  /* checksum */
  chars += nmeaAppendChecksum(s, sz, chars);

  return chars;
}
