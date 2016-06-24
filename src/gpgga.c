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
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool nmeaGPGGAparse(const char *s, const size_t sz, nmeaGPGGA *pack) {
  int fieldCount = 0;
  double time;

  if (!s || !sz || !pack) {
    return false;
  }

  nmeaTraceBuffer(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  time = NAN;
  memset(pack, 0, sizeof(*pack));
  pack->latitude = NAN;
  pack->longitude = NAN;
  pack->signal = INT_MAX;
  pack->satellites = INT_MAX;
  pack->hdop = NAN;
  pack->elv = NAN;
  pack->diff = NAN;
  pack->dgpsAge = NAN;
  pack->dgpsSid = INT_MAX;

  /* parse */
  fieldCount = nmea_scanf(s, sz, //
      "$" NMEA_PREFIX_GPGGA ",%f,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f,%c,%f,%d*", //
      &time, //
      &pack->latitude, //
      &pack->ns, //
      &pack->longitude, //
      &pack->ew, //
      &pack->signal, //
      &pack->satellites, //
      &pack->hdop, //
      &pack->elv, //
      &pack->elvUnit, //
      &pack->diff, //
      &pack->diffUnit, //
      &pack->dgpsAge, //
      &pack->dgpsSid);

  /* see that there are enough tokens */
  if (fieldCount != 14) {
    nmeaError(NMEA_PREFIX_GPGGA " parse error: need 14 tokens, got %d '%s'", fieldCount, s);
    goto err;
  }

  /* determine which fields are present and validate them */

  if (!isnan(time)) {
    if (!nmeaTIMEparseTime(time, &pack->time) //
        || !nmeaValidateTime(&pack->time, NMEA_PREFIX_GPGGA, s)) {
      goto err;
    }

    nmea_INFO_set_present(&pack->present, UTCTIME);
  } else {
    memset(&pack->time, 0, sizeof(pack->time));
  }

  if (!isnan(pack->latitude) && (pack->ns)) {
    if (!nmeaValidateNSEW(&pack->ns, true, NMEA_PREFIX_GPGGA, s)) {
      goto err;
    }

    pack->latitude = fabs(pack->latitude);
    nmea_INFO_set_present(&pack->present, LAT);
  } else {
    pack->latitude = 0.0;
    pack->ns = '\0';
  }

  if (!isnan(pack->longitude) && (pack->ew)) {
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

  if (pack->satellites != INT_MAX) {
    pack->satellites = abs(pack->satellites);
    nmea_INFO_set_present(&pack->present, SATINVIEWCOUNT);
  } else {
    pack->satellites = 0;
  }

  if (!isnan(pack->hdop)) {
    pack->hdop = fabs(pack->hdop);
    nmea_INFO_set_present(&pack->present, HDOP);
  } else {
    pack->hdop = 0.0;
  }

  if (!isnan(pack->elv) && (pack->elvUnit)) {
    if (pack->elvUnit != 'M') {
      nmeaError(NMEA_PREFIX_GPGGA " parse error: invalid elevation unit '%c' in '%s'", pack->elvUnit, s);
      goto err;
    }

    nmea_INFO_set_present(&pack->present, ELV);
  } else {
    pack->elv = 0.0;
    pack->elvUnit = '\0';
  }

  if (!isnan(pack->diff) && (pack->diffUnit)) {
    if (pack->diffUnit != 'M') {
      nmeaError(NMEA_PREFIX_GPGGA " parse error: invalid height unit '%c' in '%s'", pack->diffUnit, s);
      goto err;
    }

    /* not supported yet */
  } else {
    pack->diff = 0.0;
    pack->diffUnit = '\0';
  }

  if (!isnan(pack->dgpsAge)) {
    pack->dgpsAge = fabs(pack->dgpsAge);
    /* not supported yet */
  } else {
    pack->dgpsAge = 0.0;
  }

  if (pack->dgpsSid != INT_MAX) {
    pack->dgpsSid = abs(pack->dgpsSid);
    /* not supported yet */
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
  if (!pack || !info) {
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
    info->lat = ((pack->ns == 'N') ?
        fabs(pack->latitude) :
        -fabs(pack->latitude));
    nmea_INFO_set_present(&info->present, LAT);
  }

  if (nmea_INFO_is_present(pack->present, LON)) {
    info->lon = ((pack->ew == 'E') ?
        fabs(pack->longitude) :
        -fabs(pack->longitude));
    nmea_INFO_set_present(&info->present, LON);
  }

  if (nmea_INFO_is_present(pack->present, SIG)) {
    info->sig = pack->signal;
    nmea_INFO_set_present(&info->present, SIG);
  }

  if (nmea_INFO_is_present(pack->present, SATINVIEWCOUNT)) {
    info->satinfo.inview = pack->satellites;
    nmea_INFO_set_present(&info->present, SATINVIEWCOUNT);
  }

  if (nmea_INFO_is_present(pack->present, HDOP)) {
    info->HDOP = pack->hdop;
    nmea_INFO_set_present(&info->present, HDOP);
  }

  if (nmea_INFO_is_present(pack->present, ELV)) {
    info->elv = pack->elv;
    nmea_INFO_set_present(&info->present, ELV);
  }

  /* diff and diff_units not supported yet */

  /* dgps_age not supported yet */

  /* dgps_sid not supported yet */
}

void nmeaGPGGAFromInfo(const nmeaINFO *info, nmeaGPGGA *pack) {
  if (!pack || !info) {
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
    pack->satellites = info->satinfo.inview;
    nmea_INFO_set_present(&pack->present, SATINVIEWCOUNT);
  }

  if (nmea_INFO_is_present(info->present, HDOP)) {
    pack->hdop = info->HDOP;
    nmea_INFO_set_present(&pack->present, HDOP);
  }

  if (nmea_INFO_is_present(info->present, ELV)) {
    pack->elv = info->elv;
    pack->elvUnit = 'M';
    nmea_INFO_set_present(&pack->present, ELV);
  }

  /* diff and diff_units not supported yet */

  /* dgps_age not supported yet */

  /* dgps_sid not supported yet */
}

int nmeaGPGGAgenerate(char *s, const size_t sz, const nmeaGPGGA *pack) {
  char sTime[16];
  char sLat[16];
  char sNs[2];
  char sLon[16];
  char sEw[2];
  char sSig[4];
  char sSatInView[4];
  char sHdop[16];
  char sElv[16];
  char sElvUnit[2];

  sTime[0] = 0;
  sLat[0] = 0;
  sNs[0] = sNs[1] = 0;
  sLon[0] = 0;
  sEw[0] = sEw[1] = 0;
  sSig[0] = 0;
  sSatInView[0] = 0;
  sHdop[0] = 0;
  sElv[0] = 0;
  sElvUnit[0] = sElvUnit[1] = 0;

  if (nmea_INFO_is_present(pack->present, UTCTIME)) {
    snprintf(&sTime[0], sizeof(sTime), "%02d%02d%02d.%02d", pack->time.hour, pack->time.min, pack->time.sec,
        pack->time.hsec);
  }
  if (nmea_INFO_is_present(pack->present, LAT)) {
    snprintf(&sLat[0], sizeof(sLat), "%09.4f", pack->latitude);
    sNs[0] = pack->ns;
  }
  if (nmea_INFO_is_present(pack->present, LON)) {
    snprintf(&sLon[0], sizeof(sLon), "%010.4f", pack->longitude);
    sEw[0] = pack->ew;
  }
  if (nmea_INFO_is_present(pack->present, SIG)) {
    snprintf(&sSig[0], sizeof(sSig), "%1d", pack->signal);
  }
  if (nmea_INFO_is_present(pack->present, SATINVIEWCOUNT)) {
    snprintf(&sSatInView[0], sizeof(sSatInView), "%02d", pack->satellites);
  }
  if (nmea_INFO_is_present(pack->present, HDOP)) {
    snprintf(&sHdop[0], sizeof(sHdop), "%03.1f", pack->hdop);
  }
  if (nmea_INFO_is_present(pack->present, ELV)) {
    snprintf(&sElv[0], sizeof(sElv), "%03.1f", pack->elv);
    sElvUnit[0] = pack->elvUnit;
  }

  return nmea_printf(s, sz, //
      "$" NMEA_PREFIX_GPGGA ",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,,,,", //
      &sTime[0], //
      &sLat[0], //
      &sNs[0], //
      &sLon[0], //
      &sEw[0], //
      &sSig[0], //
      &sSatInView[0], //
      &sHdop[0], //
      &sElv[0], //
      &sElvUnit[0]);
}
