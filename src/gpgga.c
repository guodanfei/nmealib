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
#include <nmealib/tok.h>
#include <nmealib/validate.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

bool nmeaGPGGAparse(const char *s, const size_t sz, nmeaGPGGA *pack) {
  int fieldCount = 0;
  char timeBuf[256];

  if (!pack) {
    return false;
  }

  if (!s) {
    goto err;
  }

  nmea_trace_buff(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  memset(timeBuf, 0, sizeof(timeBuf));
  memset(pack, 0, sizeof(*pack));
  pack->lat = NAN;
  pack->lon = NAN;
  pack->sig = INT_MAX;
  pack->satinuse = INT_MAX;
  pack->HDOP = NAN;
  pack->elv = NAN;
  pack->diff = NAN;
  pack->dgps_age = NAN;
  pack->dgps_sid = INT_MAX;

  /* parse */
  fieldCount = nmea_scanf(s, sz, //
      "$GPGGA,%s,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f,%c,%f,%d", //
      timeBuf, //
      &pack->lat, //
      &pack->ns, //
      &pack->lon, //
      &pack->ew, //
      &pack->sig, //
      &pack->satinuse, //
      &pack->HDOP, //
      &pack->elv, //
      &pack->elv_units, //
      &pack->diff, //
      &pack->diff_units, //
      &pack->dgps_age, //
      &pack->dgps_sid);

  /* see that there are enough tokens */
  if (fieldCount != 14) {
    nmea_error("GPGGA parse error: need 14 tokens, got %d '%s'", fieldCount, s);
    goto err;
  }

  /* determine which fields are present and validate them */

  timeBuf[sizeof(timeBuf) - 1] = '\0';
  if (*timeBuf) {
    if (!_nmea_parse_time(timeBuf, &pack->time, "GPGGA") //
        || !validateTime(&pack->time, "GPGGA", s)) {
      goto err;
    }

    nmea_INFO_set_present(&pack->present, UTCTIME);
  } else {
    memset(&pack->time, 0, sizeof(pack->time));
  }

  if (!isnan(pack->lat) && (pack->ns)) {
    if (!validateNSEW(&pack->ns, true, "GPGGA", s)) {
      goto err;
    }

    pack->lat = fabs(pack->lat);
    nmea_INFO_set_present(&pack->present, LAT);
  } else {
    pack->lat = 0.0;
    pack->ns = '\0';
  }

  if (!isnan(pack->lon) && (pack->ew)) {
    if (!validateNSEW(&pack->ew, false, "GPGGA", s)) {
      goto err;
    }

    pack->lon = fabs(pack->lon);
    nmea_INFO_set_present(&pack->present, LON);
  } else {
    pack->lon = 0.0;
    pack->ew = '\0';
  }

  if (pack->sig != INT_MAX) {
    if (!validateSignal(&pack->sig, "GPGGA", s)) {
      goto err;
    }

    nmea_INFO_set_present(&pack->present, SIG);
  } else {
    pack->sig = NMEA_SIG_INVALID;
  }

  if (pack->satinuse != INT_MAX) {
    pack->satinuse = abs(pack->satinuse);
    nmea_INFO_set_present(&pack->present, SATINUSECOUNT);
  } else {
    pack->satinuse = 0;
  }

  if (!isnan(pack->HDOP)) {
    pack->HDOP = fabs(pack->HDOP);
    nmea_INFO_set_present(&pack->present, HDOP);
  } else {
    pack->HDOP = 0.0;
  }

  if (!isnan(pack->elv) && (pack->elv_units)) {
    if (pack->elv_units != 'M') {
      nmea_error("GPGGA parse error: invalid elevation unit '%c' in '%s'", pack->elv_units, s);
      goto err;
    }

    nmea_INFO_set_present(&pack->present, ELV);
  } else {
    pack->elv = 0.0;
    pack->elv_units = '\0';
  }

  if (!isnan(pack->diff) && (pack->diff_units)) {
    if (pack->diff_units != 'M') {
      nmea_error("GPGGA parse error: invalid height unit '%c' in '%s'", pack->diff_units, s);
      goto err;
    }

    /* not supported yet */
  } else {
    pack->diff = 0.0;
    pack->diff_units = '\0';
  }

  if (!isnan(pack->dgps_age)) {
    pack->dgps_age = fabs(pack->dgps_age);
    /* not supported yet */
  } else {
    pack->dgps_age = 0.0;
  }

  if (pack->dgps_sid != INT_MAX) {
    pack->dgps_sid = abs(pack->dgps_sid);
    /* not supported yet */
  } else {
    pack->dgps_sid = 0;
  }

  return true;

err:
  memset(pack, 0, sizeof(*pack));
  pack->sig = NMEA_SIG_INVALID;
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
        fabs(pack->lat) :
        -fabs(pack->lat));
    nmea_INFO_set_present(&info->present, LAT);
  }

  if (nmea_INFO_is_present(pack->present, LON)) {
    info->lon = ((pack->ew == 'E') ?
        fabs(pack->lon) :
        -fabs(pack->lon));
    nmea_INFO_set_present(&info->present, LON);
  }

  if (nmea_INFO_is_present(pack->present, SIG)) {
    info->sig = pack->sig;
    nmea_INFO_set_present(&info->present, SIG);
  }

  if (nmea_INFO_is_present(pack->present, SATINUSECOUNT)) {
    info->satinfo.inuse = pack->satinuse;
    nmea_INFO_set_present(&info->present, SATINUSECOUNT);
  }

  if (nmea_INFO_is_present(pack->present, HDOP)) {
    info->HDOP = pack->HDOP;
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
    pack->lat = fabs(info->lat);
    pack->ns = ((info->lat >= 0.0) ?
        'N' :
        'S');
    nmea_INFO_set_present(&pack->present, LAT);
  }

  if (nmea_INFO_is_present(info->present, LON)) {
    pack->lon = fabs(info->lon);
    pack->ew = ((info->lon >= 0.0) ?
        'E' :
        'W');
    nmea_INFO_set_present(&pack->present, LON);
  }

  if (nmea_INFO_is_present(info->present, SIG)) {
    pack->sig = info->sig;
    nmea_INFO_set_present(&pack->present, SIG);
  } else {
    pack->sig = NMEA_SIG_INVALID;
  }

  if (nmea_INFO_is_present(info->present, SATINUSECOUNT)) {
    pack->satinuse = info->satinfo.inuse;
    nmea_INFO_set_present(&pack->present, SATINUSECOUNT);
  }

  if (nmea_INFO_is_present(info->present, HDOP)) {
    pack->HDOP = info->HDOP;
    nmea_INFO_set_present(&pack->present, HDOP);
  }

  if (nmea_INFO_is_present(info->present, ELV)) {
    pack->elv = info->elv;
    pack->elv_units = 'M';
    nmea_INFO_set_present(&pack->present, ELV);
  }

  /* diff and diff_units not supported yet */

  /* dgps_age not supported yet */

  /* dgps_sid not supported yet */
}
