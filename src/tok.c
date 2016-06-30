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

#include <nmealib/tok.h>

#include <nmealib/context.h>
#include <nmealib/util.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** The maximum size of a string-to-number conversion buffer*/
#define NMEA_CONVSTR_BUF    64

unsigned int nmeaCalculateCRC(const char *s, const size_t sz) {
  size_t i = 0;
  int crc = 0;

  if (s[i] == '$') {
    i++;
  }

  for (; i < sz; i++) {
    crc ^= (int) s[i];
  }

  return ((unsigned int) crc & 0xff);
}

int nmeaStringToInteger(const char *s, size_t sz, int radix) {
  long r = nmeaStringToLong(s, sz, radix);

  if (r < INT_MIN) {
    r = INT_MIN;
  }

  if (r > INT_MAX) {
    r = INT_MAX;
  }

  return (int) r;
}

unsigned int nmeaStringToUnsignedInteger(const char *s, size_t sz, int radix) {
  unsigned long r = nmeaStringToUnsignedLong(s, sz, radix);

  if (r > UINT_MAX) {
    r = UINT_MAX;
  }

  return (unsigned int) r;
}

long nmeaStringToLong(const char *s, size_t sz, int radix) {
  char buf[NMEA_CONVSTR_BUF];
  char * endPtr = NULL;
  long value;

  if (!s || !sz || (sz >= NMEA_CONVSTR_BUF) || (radix < 1)) {
    return 0;
  }

  memcpy(buf, s, sz);
  buf[sz] = '\0';

  errno = 0;
  value = strtol(buf, &endPtr, radix);

  if (!((endPtr != buf) && (*buf != '\0')) || (errno == ERANGE)) {
    /* invalid conversion */
    nmeaError("Could not convert '%s' to a long integer", buf);
    return 0;
  }

  return value;
}

unsigned long nmeaStringToUnsignedLong(const char *s, size_t sz, int radix) {
  char buf[NMEA_CONVSTR_BUF];
  char * endPtr = NULL;
  unsigned long value;

  if (!s || !sz || (sz >= NMEA_CONVSTR_BUF) || (radix < 1)) {
    return 0;
  }

  memcpy(buf, s, sz);
  buf[sz] = '\0';

  errno = 0;
  value = strtoul(buf, &endPtr, radix);

  if (!((endPtr != buf) && (*buf != '\0')) || (errno == ERANGE)) {
    /* invalid conversion */
    nmeaError("Could not convert '%s' to an unsigned long integer", buf);
    return 0;
  }

  return value;
}

double nmeaStringToDouble(const char *s, const size_t sz) {
  char buf[NMEA_CONVSTR_BUF];
  char * endPtr = NULL;
  double value;

  if (!s || !sz || (sz >= NMEA_CONVSTR_BUF)) {
    return 0.0;
  }

  memcpy(buf, s, sz);
  buf[sz] = '\0';

  errno = 0;
  value = strtod(buf, &endPtr);

  if (!((endPtr != buf) && (*buf != '\0')) || (errno == ERANGE)) {
    /* invalid conversion */
    nmeaError("Could not convert '%s' to a double", buf);
    return 0.0;
  }

  return value;
}

int nmeaAppendChecksum(char *s, size_t sz, size_t len) {
  size_t l = 0;
  if (sz > len) {
    l = sz - len;
  }
  return snprintf(&s[len], l, "*%02X\r\n", nmeaCalculateCRC(s, len));
}

int nmeaPrintf(char *s, size_t sz, const char *format, ...) {
  va_list args;
  int chars;
  int addedChars;

  if (!s || !sz || !format) {
    return 0;
  }

  va_start(args, format);

  chars = vsnprintf(s, sz, format, args);

  if ((chars < 0) //
      || (size_t) chars >= sz) {
    goto out;
  }

  addedChars = nmeaAppendChecksum(s, sz, chars);
  if (addedChars < 0) {
    chars = addedChars;
    goto out;
  }

  chars += addedChars;

out:
  va_end(args);
  return chars;
}

size_t nmeaScanf(const char *s, size_t sz, const char *format, ...) {

#define NMEA_SCANF_COMPARE   1u
#define NMEA_SCANF_TOKEN     2u

  size_t tokens = 0;
  unsigned char state = NMEA_SCANF_COMPARE;

  const char *sCharacter = s;
  const char *sEnd = &s[sz];

  const char *formatCharacter = format;
  const char *formatStart = format;

  size_t width = 0;
  size_t widthMax = sz;
  size_t widthCount = 0;
  const char *sTokenStart = sCharacter;

  void *arg = NULL;
  va_list args;

  va_start(args, format);

  for (formatCharacter = format; *formatCharacter && (sCharacter < sEnd); formatCharacter++) {
    switch (state) {
      case NMEA_SCANF_COMPARE:
        if (*formatCharacter == '%') {
          /* start of format */
          formatStart = &formatCharacter[1];
          widthCount = 0;
          state = NMEA_SCANF_TOKEN;
        } else if (*sCharacter++ != *formatCharacter) {
          /* compare regular character between s and format */
          goto out;
        }
        break;

      case NMEA_SCANF_TOKEN:
        if (isdigit(*formatCharacter)) {
          widthCount++;
          break;
        }

        /* determine width */
        if (!widthCount) {
          width = 0;
          widthMax = (size_t)(sEnd - sCharacter);
        } else {
          width = nmeaStringToUnsignedInteger(formatStart, widthCount, 10);
          widthMax = width;
        }

        sTokenStart = sCharacter;

        if (!width && ('C' == toupper(*formatCharacter)) && (*sCharacter != formatCharacter[1])) {
          width = 1;
        }

        if (!width) {
          if (!formatCharacter[1] || (0 == (sCharacter = (char *) memchr(sCharacter, formatCharacter[1], sEnd - sCharacter)))) {
            sCharacter = sEnd;
          }
        } else if ('s' == *formatCharacter) {
          if (!formatCharacter[1] || (0 == (sCharacter = (char *) memchr(sCharacter, formatCharacter[1], sEnd - sCharacter)))) {
            sCharacter = sEnd;
          }
        } else {
          if ((sCharacter + width) <= sEnd) {
            sCharacter += width;
          } else {
            goto out;
          }
        }

        if (sCharacter > sEnd) {
          goto out;
        }

        state = NMEA_SCANF_COMPARE;
        tokens++;

        arg = NULL;
        if (*sTokenStart == '*') {
          width = 0;
        } else {
          width = (size_t) (sCharacter - sTokenStart);
        }
        if (width > widthMax) {
          width = widthMax;
        }

        switch (*formatCharacter) {
          case 'c':
            arg = (void *) va_arg(args, char *);
            if (width && arg) {
              *((char *) arg) = *sTokenStart;
            }
            break;

          case 'C':
            arg = (void *) va_arg(args, char *);
            if (width && arg) {
              *((char *) arg) = toupper(*sTokenStart);
            }
            break;

          case 's':
            arg = (void *) va_arg(args, char *);
            if (width && arg) {
              memcpy(arg, sTokenStart, width);
              if (width < widthMax) {
                ((char *) arg)[width] = '\0';
              } else {
                ((char *) arg)[widthMax - 1] = '\0';
              }
            }
            break;

          case 'f':
            arg = (void *) va_arg(args, double *);
            if (width && arg) {
              *((double *) arg) = nmeaStringToDouble(sTokenStart, width);
            }
            break;

          case 'F':
            arg = (void *) va_arg(args, double *);
            if (width && arg) {
              *((double *) arg) = fabs(nmeaStringToDouble(sTokenStart, width));
            }
            break;

          case 'd':
            arg = (void *) va_arg(args, int *);
            if (width && arg) {
              *((int *) arg) = nmeaStringToInteger(sTokenStart, width, 10);
            }
            break;

          case 'u':
            arg = (void *) va_arg(args, unsigned int *);
            if (width && arg) {
              *((unsigned int *) arg) = nmeaStringToUnsignedInteger(sTokenStart, width, 10);
            }
            break;

          case 'l':
            arg = (void *) va_arg(args, long *);
            if (width && arg) {
              *((long *) arg) = nmeaStringToLong(sTokenStart, width, 10);
            }
            break;

          default:
            goto out;
        }

        break;

      default:
        break;
    };
  } /* for */

out:
  va_end(args);
  return tokens;
}
