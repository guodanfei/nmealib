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

bool _nmea_parse_date(const int date, nmeaTIME *t, const char * prefix, const char * s) {
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
