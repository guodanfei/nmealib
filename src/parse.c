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
#include <string.h>

bool nmeaTIMEparseTime(const char *time, nmeaTIME *t) {
  size_t sz;

  if (!time || !t) {
    return false;
  }

  sz = strlen(time);

  if (sz == 6) { // HHMMSS
    t->hsec = 0;
    return (3 == nmeaScanf(time, sz, "%2d%2d%2d", &t->hour, &t->min, &t->sec));
  }

  if (sz == 8) { // HHMMSS.t
    if (4 == nmeaScanf(time, sz, "%2d%2d%2d.%d", &t->hour, &t->min, &t->sec, &t->hsec)) {
      t->hsec *= 10;
      return true;
    }
    return false;
  }

  if (sz == 9) { // HHMMSS.hh
    return (4 == nmeaScanf(time, sz, "%2d%2d%2d.%d", &t->hour, &t->min, &t->sec, &t->hsec));
  }

  if (sz == 10) { // HHMMSS.mmm
    if ((4 == nmeaScanf(time, sz, "%2d%2d%2d.%d", &t->hour, &t->min, &t->sec, &t->hsec))) {
      t->hsec = (t->hsec + 5) / 10;
      return true;
    }
    return false;
  }

  return false;
}

bool nmeaTIMEparseDate(const char *date, nmeaTIME *t) {
  size_t sz;

  if (!date || !t) {
    return false;
  }

  sz = strlen(date);

  if (sz != 6) {
    return false;
  }

  if (3 != nmeaScanf(date, sz, "%2d%2d%2d", &t->day, &t->mon, &t->year)) {
    return false;
  }

  if (t->year > 90) {
    t->year += 1900;
  } else {
    t->year += 2000;
  }

  return true;
}
