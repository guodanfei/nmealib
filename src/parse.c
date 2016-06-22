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

#include <nmea/parse.h>

#include <nmea/context.h>
#include <nmea/conversions.h>
#include <nmea/gmath.h>
#include <nmea/tok.h>
#include <nmea/validate.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <limits.h>

bool _nmea_parse_time(const char *s, nmeaTIME *t, const char * prefix) {
  size_t sz;

  if (!s || !t) {
    return false;
  }

  sz = strlen(s);

  if (sz == 6) { // hhmmss
    t->hsec = 0;
    return (3 == nmea_scanf(s, sz, "%2d%2d%2d", &t->hour, &t->min, &t->sec));
  }

  if (sz == 8) { // hhmmss.s
    if (4 == nmea_scanf(s, sz, "%2d%2d%2d.%d", &t->hour, &t->min, &t->sec, &t->hsec)) {
      t->hsec *= 10;
      return true;
    }
    return false;
  }

  if (sz == 9) { // hhmmss.ss
    return (4 == nmea_scanf(s, sz, "%2d%2d%2d.%d", &t->hour, &t->min, &t->sec, &t->hsec));
  }

  if (sz == 10) { // hhmmss.sss
    if ((4 == nmea_scanf(s, sz, "%2d%2d%2d.%d", &t->hour, &t->min, &t->sec, &t->hsec))) {
      t->hsec = (t->hsec + 5) / 10;
      return true;
    }
    return false;
  }

  nmea_error("%s parse error: invalid time format in '%s'", prefix, s);

  return false;
}

/**
 * Parse nmeaTIME (date only, no time) from a string.
 *
 * The month is adjusted (decremented by 1) to comply with the nmeaTIME month
 * range of [0, 11]. The year is adjusted (incremented by 100) for years
 * before 90 to comply with the nmeaTIME year range of [90, 189].
 *
 * @param date The date (DDMMYY)
 * @param t The structure in which to store the parsed date
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True on success, false otherwise
 */
static bool _nmea_parse_date(const int date, nmeaTIME *t, const char * prefix, const char * s) {
  if (!t) {
    return false;
  }

  if ((date < 0) || (date > 999999)) {
    nmea_error("%s parse error: invalid date '%d' in '%s'", prefix, date, s);
    return false;
  }

  t->day = (date / 10000) % 100;
  t->mon = (date / 100) % 100;
  t->year = date % 100;

  t->mon--;

  if (t->year < 90) {
    t->year += 100;
  }

  return true;
}

bool nmea_parse_GPGSV(const char *s, const size_t sz, nmeaGPGSV *pack) {
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
  if (nmea_gsv_npack((pack->sentences * NMEA_SATINPACK)) != nmea_gsv_npack(pack->satellites)) {
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

bool nmea_parse_GPRMC(const char *s, const size_t sz, nmeaGPRMC *pack) {
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

bool nmea_parse_GPVTG(const char *s, const size_t sz, nmeaGPVTG *pack) {
  bool speedK = false;
  bool speedN = false;
  int fieldCount;

  if (!pack) {
    return false;
  }

  if (!s) {
    goto err;
  }

  nmea_trace_buff(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  memset(pack, 0, sizeof(*pack));
  pack->track = NAN;
  pack->mtrack = NAN;
  pack->spn = NAN;
  pack->spk = NAN;

  /* parse */
  fieldCount = nmea_scanf(s, sz, //
      "$GPVTG,%f,%c,%f,%c,%f,%c,%f,%c", //
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
    nmea_error("GPVTG parse error: need 8 tokens, got %d in '%s'", fieldCount, s);
    return false;
  }

  /* determine which fields are present and validate them */

  if (!isnan(pack->track) && (pack->track_t)) {
    pack->track_t = toupper(pack->track_t);
    if (pack->track_t != 'T') {
      nmea_error("GPVTG parse error: invalid track unit, got '%c', expected 'T'", pack->track_t);
      return false;
    }

    nmea_INFO_set_present(&pack->present, TRACK);
  } else {
    pack->track = 0.0;
    pack->track_t = '\0';
  }

  if (!isnan(pack->mtrack) && (pack->mtrack_m)) {
    pack->mtrack_m = toupper(pack->mtrack_m);
    if (pack->mtrack_m != 'M') {
      nmea_error("GPVTG parse error: invalid mtrack unit, got '%c', expected 'M'", pack->mtrack_m);
      return false;
    }

    nmea_INFO_set_present(&pack->present, MTRACK);
  } else {
    pack->mtrack = 0.0;
    pack->mtrack_m = '\0';
  }

  if (!isnan(pack->spn) && (pack->spn_n)) {
    pack->spn_n = toupper(pack->spn_n);
    if (pack->spn_n != 'N') {
      nmea_error("GPVTG parse error: invalid knots speed unit, got '%c', expected 'N'", pack->spn_n);
      return false;
    }

    speedN = true;
    nmea_INFO_set_present(&pack->present, SPEED);
  } else {
    pack->spn = 0.0;
    pack->spn_n = '\0';
  }

  if (!isnan(pack->spk) && (pack->spk_k)) {
    pack->spk_k = toupper(pack->spk_k);
    if (pack->spk_k != 'K') {
      nmea_error("GPVTG parse error: invalid kph speed unit, got '%c', expected 'K'", pack->spk_k);
      return false;
    }

    speedK = true;
    nmea_INFO_set_present(&pack->present, SPEED);

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
