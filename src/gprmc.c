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

#include <nmealib/parse.h>

#include <nmealib/context.h>
#include <nmealib/gmath.h>
#include <nmealib/info.h>
#include <nmealib/parse.h>
#include <nmealib/tok.h>
#include <nmealib/validate.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool nmeaGPRMCparse(const char *s, const size_t sz, nmeaGPRMC *pack) {
  int fieldCount;
  char timeBuf[256];
  int date;

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
  pack->speed = NAN;
  pack->track = NAN;
  date = INT_MAX;
  pack->magvar = NAN;

  /* parse */
  fieldCount = nmea_scanf(s, sz, //
      "$GPRMC,%s,%c,%f,%c,%f,%c,%f,%f,%d,%f,%c,%c", //
      timeBuf, //
      &pack->sig, //
      &pack->lat, //
      &pack->ns, //
      &pack->lon, //
      &pack->ew, //
      &pack->speed, //
      &pack->track, //
      &date, //
      &pack->magvar, //
      &pack->magvar_ew, //
      &pack->sigMode);

  /* see that there are enough tokens */
  if ((fieldCount != 11) && (fieldCount != 12)) {
    nmea_error("GPRMC parse error: need 11 or 12 tokens, got %d in '%s'", fieldCount, s);
    goto err;
  }

  /* determine which fields are present and validate them */

  timeBuf[sizeof(timeBuf) - 1] = '\0';
  if (*timeBuf) {
    if (!_nmea_parse_time(timeBuf, &pack->utc, "GPRMC") //
        || !validateTime(&pack->utc, "GPRMC", s)) {
      goto err;
    }

    nmea_INFO_set_present(&pack->present, UTCTIME);
  } else {
    pack->utc.hour = 0;
    pack->utc.min = 0;
    pack->utc.sec = 0;
    pack->utc.hsec = 0;
  }

  if (fieldCount == 11) {
    /* no mode */
    if (pack->sig) {
      pack->sig = toupper(pack->sig);
      if (!((pack->sig == 'A') || (pack->sig == 'V'))) {
        nmea_error("GPRMC parse error: invalid status '%c' in '%s'", pack->sig, s);
        goto err;
      }

      pack->sigMode = '\0';

      nmea_INFO_set_present(&pack->present, SIG);
    } else {
      pack->sig = '\0';
      pack->sigMode = '\0';
    }
  } else {
    /* with mode */
    if (pack->sig && pack->sigMode) {
      pack->sig = toupper(pack->sig);
      if (!((pack->sig == 'A') || (pack->sig == 'V'))) {
        nmea_error("GPRMC parse error: invalid status '%c' in '%s'", pack->sig, s);
        goto err;
      }

      if (!validateMode(&pack->sigMode, "GPRMC", s)) {
        goto err;
      }

      nmea_INFO_set_present(&pack->present, SIG);
    } else {
      pack->sig = '\0';
      pack->sigMode = '\0';
    }
  }

  if (!isnan(pack->lat) && (pack->ns)) {
    if (!validateNSEW(&pack->ns, true, "GPRMC", s)) {
      goto err;
    }

    pack->lat = fabs(pack->lat);
    nmea_INFO_set_present(&pack->present, LAT);
  } else {
    pack->lat = 0.0;
    pack->ns = '\0';
  }

  if (!isnan(pack->lon) && (pack->ew)) {
    if (!validateNSEW(&pack->ew, false, "GPRMC", s)) {
      goto err;
    }

    pack->lon = fabs(pack->lon);
    nmea_INFO_set_present(&pack->present, LON);
  } else {
    pack->lon = 0.0;
    pack->ew = '\0';
  }

  if (!isnan(pack->speed)) {
    pack->speed = fabs(pack->speed);
    nmea_INFO_set_present(&pack->present, SPEED);
  } else {
    pack->speed = 0.0;
  }

  if (!isnan(pack->track)) {
    pack->track = fabs(pack->track);
    nmea_INFO_set_present(&pack->present, TRACK);
  } else {
    pack->track = 0.0;
  }

  if (date != INT_MAX) {
    if (!_nmea_parse_date(date, &pack->utc, "GPRMC", s) //
        || !validateDate(&pack->utc, "GPRMC", s)) {
      goto err;
    }

    nmea_INFO_set_present(&pack->present, UTCDATE);
  } else {
    pack->utc.year = 0;
    pack->utc.mon = 0;
    pack->utc.day = 0;
  }

  if (!isnan(pack->magvar) && (pack->magvar_ew)) {
    if (!validateNSEW(&pack->magvar_ew, false, "GPRMC", s)) {
      goto err;
    }

    pack->magvar = fabs(pack->magvar);
    nmea_INFO_set_present(&pack->present, MAGVAR);
  } else {
    pack->magvar = 0.0;
    pack->magvar_ew = '\0';
  }

  return true;

  err: memset(pack, 0, sizeof(*pack));
  return false;
}

void nmeaGPRMCToInfo(const nmeaGPRMC *pack, nmeaINFO *info) {
  if (!pack || !info) {
    return;
  }

  nmea_INFO_set_present(&info->present, SMASK);

  info->smask |= GPRMC;

  if (nmea_INFO_is_present(pack->present, UTCTIME)) {
    info->utc.hour = pack->utc.hour;
    info->utc.min = pack->utc.min;
    info->utc.sec = pack->utc.sec;
    info->utc.hsec = pack->utc.hsec;
    nmea_INFO_set_present(&info->present, UTCTIME);
  }

  if (nmea_INFO_is_present(pack->present, SIG)) {
    if (pack->sig != 'A') {
      info->sig = NMEA_SIG_INVALID;
    } else if (pack->sigMode != '\0') {
      /* with mode */
      info->sig = nmea_INFO_mode_to_sig(pack->sigMode);
    } else {
      /* without mode */
      info->sig = NMEA_SIG_FIX;
    }

    nmea_INFO_set_present(&info->present, SIG);
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

  if (nmea_INFO_is_present(pack->present, SPEED)) {
    info->speed = pack->speed * NMEA_TUD_KNOTS;
    nmea_INFO_set_present(&info->present, SPEED);
  }

  if (nmea_INFO_is_present(pack->present, TRACK)) {
    info->track = pack->track;
    nmea_INFO_set_present(&info->present, TRACK);
  }

  if (nmea_INFO_is_present(pack->present, UTCDATE)) {
    info->utc.year = pack->utc.year;
    info->utc.mon = pack->utc.mon;
    info->utc.day = pack->utc.day;
    nmea_INFO_set_present(&info->present, UTCDATE);
  }

  if (nmea_INFO_is_present(pack->present, MAGVAR)) {
    info->magvar = ((pack->magvar_ew == 'E') ?
        fabs(pack->magvar) :
        -fabs(pack->magvar));
    nmea_INFO_set_present(&info->present, MAGVAR);
  }
}

void nmeaGPRMCFromInfo(const nmeaINFO *info, nmeaGPRMC *pack) {
  if (!pack || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (nmea_INFO_is_present(info->present, UTCTIME)) {
    pack->utc.hour = info->utc.hour;
    pack->utc.min = info->utc.min;
    pack->utc.sec = info->utc.sec;
    pack->utc.hsec = info->utc.hsec;
    nmea_INFO_set_present(&pack->present, UTCTIME);
  }

  if (nmea_INFO_is_present(info->present, SIG)) {
    pack->sig = ((info->sig != NMEA_SIG_INVALID) ?
        'A' :
        'V');
    pack->sigMode = nmea_INFO_sig_to_mode(info->sig);
    nmea_INFO_set_present(&pack->present, SIG);
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

  if (nmea_INFO_is_present(info->present, SPEED)) {
    pack->speed = info->speed / NMEA_TUD_KNOTS;
    nmea_INFO_set_present(&pack->present, SPEED);
  }

  if (nmea_INFO_is_present(info->present, TRACK)) {
    pack->track = info->track;
    nmea_INFO_set_present(&pack->present, TRACK);
  }

  if (nmea_INFO_is_present(info->present, UTCDATE)) {
    pack->utc.year = info->utc.year;
    pack->utc.mon = info->utc.mon;
    pack->utc.day = info->utc.day;
    nmea_INFO_set_present(&pack->present, UTCDATE);
  }

  if (nmea_INFO_is_present(info->present, MAGVAR)) {
    pack->magvar = fabs(info->magvar);
    pack->ew = ((info->magvar >= 0.0) ?
        'E' :
        'W');
    nmea_INFO_set_present(&pack->present, MAGVAR);
  }
}

int nmeaGPRMCgenerate(char *s, const int sz, const nmeaGPRMC *pack) {
  char sTime[16];
  char sDate[16];
  char sLat[16];
  char sNs[2];
  char sLon[16];
  char sEw[2];
  char sSpeed[16];
  char sTrack[16];
  char sMagvar[16];
  char sMagvar_ew[2];

  sTime[0] = 0;
  sDate[0] = 0;
  sLat[0] = 0;
  sNs[0] = sNs[1] = 0;
  sLon[0] = 0;
  sEw[0] = sEw[1] = 0;
  sSpeed[0] = 0;
  sTrack[0] = 0;
  sMagvar[0] = 0;
  sMagvar_ew[0] = sMagvar_ew[1] = 0;

  if (nmea_INFO_is_present(pack->present, UTCDATE)) {
    snprintf(&sDate[0], sizeof(sDate), "%02d%02d%02d", pack->utc.day, pack->utc.mon + 1, pack->utc.year - 100);
  }
  if (nmea_INFO_is_present(pack->present, UTCTIME)) {
    snprintf(&sTime[0], sizeof(sTime), "%02d%02d%02d.%02d", pack->utc.hour, pack->utc.min, pack->utc.sec,
        pack->utc.hsec);
  }
  if (nmea_INFO_is_present(pack->present, LAT)) {
    snprintf(&sLat[0], sizeof(sLat), "%09.4f", pack->lat);
    sNs[0] = pack->ns;
  }
  if (nmea_INFO_is_present(pack->present, LON)) {
    snprintf(&sLon[0], sizeof(sLon), "%010.4f", pack->lon);
    sEw[0] = pack->ew;
  }
  if (nmea_INFO_is_present(pack->present, SPEED)) {
    snprintf(&sSpeed[0], sizeof(sSpeed), "%03.1f", pack->speed);
  }
  if (nmea_INFO_is_present(pack->present, TRACK)) {
    snprintf(&sTrack[0], sizeof(sTrack), "%03.1f", pack->track);
  }
  if (nmea_INFO_is_present(pack->present, MAGVAR)) {
    snprintf(&sMagvar[0], sizeof(sMagvar), "%03.1f", pack->magvar);
    sMagvar_ew[0] = pack->magvar_ew;
  }

  return nmea_printf(s, sz, "$GPRMC,%s,%c,%s,%s,%s,%s,%s,%s,%s,%s,%s,%c", &sTime[0], pack->sig, &sLat[0], &sNs[0],
      &sLon[0], &sEw[0], &sSpeed[0], &sTrack[0], &sDate[0], &sMagvar[0], &sMagvar_ew[0], pack->sigMode);
}
