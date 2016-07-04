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
#include <nmealib/tok.h>
#include <ctype.h>
#include <string.h>

bool nmeaTimeParseTime(const char *s, NmeaTime *time) {
  const char *tm;
  size_t sz;

  if (!s //
      || !time) {
    return false;
  }

  tm = s;
  sz = nmeaStringTrim(&tm);

  if (nmeaStringContainsWhitespace(tm, sz)) {
    return false;
  }

  if (sz == 6) { // HHMMSS
    time->hsec = 0;
    return (3 == nmeaScanf(tm, sz, "%2u%2u%2u", &time->hour, &time->min, &time->sec));
  }

  if (sz == 8) { // HHMMSS.t
    if (4 == nmeaScanf(tm, sz, "%2u%2u%2u.%u", &time->hour, &time->min, &time->sec, &time->hsec)) {
      time->hsec *= 10;
      return true;
    }
    return false;
  }

  if (sz == 9) { // HHMMSS.hh
    return (4 == nmeaScanf(tm, sz, "%2u%2u%2u.%u", &time->hour, &time->min, &time->sec, &time->hsec));
  }

  if (sz == 10) { // HHMMSS.mmm
    if ((4 == nmeaScanf(tm, sz, "%2u%2u%2u.%u", &time->hour, &time->min, &time->sec, &time->hsec))) {
      time->hsec = (time->hsec + 5) / 10;
      return true;
    }
    return false;
  }

  return false;
}

bool nmeaTimeParseDate(const char *s, NmeaTime *date) {
  size_t sz;
  const char * d;

  if (!s //
      || !date) {
    return false;
  }

  d = s;
  sz = nmeaStringTrim(&d);

  if (nmeaStringContainsWhitespace(d, sz)) {
    return false;
  }

  if (sz != 6) {
    return false;
  }

  if (3 != nmeaScanf(d, sz, "%2u%2u%2u", &date->day, &date->mon, &date->year)) {
    return false;
  }

  if (date->year > 90) {
    date->year += 1900;
  } else {
    date->year += 2000;
  }

  return true;
}
