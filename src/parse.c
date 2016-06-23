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
#include <math.h>

bool nmeaTIMEparseTime(const double time, nmeaTIME *t) {
  double timeInt;
  long tm;

  if (!t) {
    return false;
  }

  modf(fabs(time) * 1E3, &timeInt);
  tm = lrint(timeInt) + 5;

  t->hour = (tm / 10000000) % 100;
  t->min = (tm / 100000) % 100;
  t->sec = (tm / 1000) % 100;
  t->hsec = (tm / 10) % 100;

  return true;
}

bool nmeaTIMEparseDate(const int date, nmeaTIME *t) {
  if (!t) {
    return false;
  }

  t->day = (date / 10000) % 100;
  t->mon = (date / 100) % 100;
  t->year = date % 100;

  if (t->mon > 0) {
    t->mon--;
  }

  if (t->year < 90) {
    t->year += 100;
  }

  return true;
}
