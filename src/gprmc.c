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

#include <nmealib/gprmc.h>

#include <nmealib/context.h>
#include <nmealib/gmath.h>
#include <nmealib/info.h>
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

bool nmeaGPRMCParse(const char *s, const size_t sz, nmeaGPRMC *pack) {
  size_t fieldCount;
  char timeBuf[16];
  char dateBuf[8];
  bool v23Saved;

  if (!s //
      || !sz //
      || !pack) {
    return false;
  }

  nmeaTraceBuffer(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  memset(timeBuf, 0, sizeof(timeBuf));
  memset(dateBuf, 0, sizeof(dateBuf));
  memset(pack, 0, sizeof(*pack));
  pack->latitude = NAN;
  pack->longitude = NAN;
  pack->speedN = NAN;
  pack->track = NAN;
  pack->magvar = NAN;

  /* parse */
  fieldCount = nmeaScanf(s, sz, //
      "$GPRMC,%16s,%C,%F,%C,%F,%C,%f,%f,%8s,%F,%C,%C*", //
      timeBuf, //
      &pack->sigSelection, //
      &pack->latitude, //
      &pack->ns, //
      &pack->longitude, //
      &pack->ew, //
      &pack->speedN, //
      &pack->track, //
      dateBuf, //
      &pack->magvar, //
      &pack->magvar_ew, //
      &pack->sig);

  /* see that there are enough tokens */
  if ((fieldCount != 11) //
      && (fieldCount != 12)) {
    nmeaError(NMEALIB_PREFIX_GPRMC " parse error: need 11 or 12 tokens, got %lu in '%s'", (long unsigned) fieldCount, s);
    goto err;
  }

  pack->v23 = (fieldCount == 12);

  /* determine which fields are present and validate them */

  if (*timeBuf) {
    if (!nmeaTIMEparseTime(timeBuf, &pack->utc) //
        || !nmeaValidateTime(&pack->utc, NMEALIB_PREFIX_GPRMC, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, UTCTIME);
  } else {
    pack->utc.hour = 0;
    pack->utc.min = 0;
    pack->utc.sec = 0;
    pack->utc.hsec = 0;
  }

  if (pack->sigSelection //
      && !((pack->sigSelection == 'A') //
          || (pack->sigSelection == 'V'))) {
    nmeaError(NMEALIB_PREFIX_GPRMC " parse error: invalid status '%c' in '%s'", pack->sigSelection, s);
    goto err;
  }

  if (!pack->v23) {
    /* no mode */
    if (pack->sigSelection) {
      pack->sig = '\0';
      nmeaInfoSetPresent(&pack->present, SIG);
    } else {
      pack->sigSelection = '\0';
      pack->sig = '\0';
    }
  } else {
    /* with mode */
    if (pack->sigSelection //
        && pack->sig) {
      if (!nmeaValidateMode(pack->sig, NMEALIB_PREFIX_GPRMC, s)) {
        goto err;
      }

      nmeaInfoSetPresent(&pack->present, SIG);
    } else {
      pack->sigSelection = '\0';
      pack->sig = '\0';
    }
  }

  if (!isnan(pack->latitude)) {
    if (!nmeaValidateNSEW(pack->ns, true, NMEALIB_PREFIX_GPRMC, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, LAT);
  } else {
    pack->latitude = 0.0;
    pack->ns = '\0';
  }

  if (!isnan(pack->longitude)) {
    if (!nmeaValidateNSEW(pack->ew, false, NMEALIB_PREFIX_GPRMC, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, LON);
  } else {
    pack->longitude = 0.0;
    pack->ew = '\0';
  }

  if (!isnan(pack->speedN)) {
    nmeaInfoSetPresent(&pack->present, SPEED);
  } else {
    pack->speedN = 0.0;
  }

  if (!isnan(pack->track)) {
    nmeaInfoSetPresent(&pack->present, TRACK);
  } else {
    pack->track = 0.0;
  }

  if (*dateBuf) {
    if (!nmeaTIMEparseDate(dateBuf, &pack->utc) //
        || !nmeaValidateDate(&pack->utc, NMEALIB_PREFIX_GPRMC, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, UTCDATE);
  } else {
    pack->utc.year = 0;
    pack->utc.mon = 0;
    pack->utc.day = 0;
  }

  if (!isnan(pack->magvar)) {
    if (!nmeaValidateNSEW(pack->magvar_ew, false, NMEALIB_PREFIX_GPRMC, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, MAGVAR);
  } else {
    pack->magvar = 0.0;
    pack->magvar_ew = '\0';
  }

  return true;

err:
  v23Saved = pack->v23;
  memset(pack, 0, sizeof(*pack));
  pack->v23 = v23Saved;
  return false;
}

void nmeaGPRMCToInfo(const nmeaGPRMC *pack, NmeaInfo *info) {
  if (!pack //
      || !info) {
    return;
  }

  nmeaInfoSetPresent(&info->present, SMASK);

  info->smask |= GPRMC;

  if (nmeaInfoIsPresentAll(pack->present, UTCTIME)) {
    info->utc.hour = pack->utc.hour;
    info->utc.min = pack->utc.min;
    info->utc.sec = pack->utc.sec;
    info->utc.hsec = pack->utc.hsec;
    nmeaInfoSetPresent(&info->present, UTCTIME);
  }

  if (nmeaInfoIsPresentAll(pack->present, SIG)) {
    if (!pack->v23) {
      /* no mode */
      if ((pack->sigSelection == 'A') //
          && (info->sig == NMEALIB_SIG_INVALID)) {
        info->sig = NMEALIB_SIG_FIX;
        nmeaInfoSetPresent(&info->present, SIG);
      }
    } else {
      /* with mode */

      if (pack->sigSelection != 'A') {
        info->sig = NMEALIB_SIG_INVALID;
      } else {
        info->sig = nmeaInfoModeToSig(pack->sig);
      }
      nmeaInfoSetPresent(&info->present, SIG);
    }
  }

  if (nmeaInfoIsPresentAll(pack->present, LAT)) {
    info->lat = ((pack->ns == 'N') ?
        pack->latitude :
        -pack->latitude);
    nmeaInfoSetPresent(&info->present, LAT);
  }

  if (nmeaInfoIsPresentAll(pack->present, LON)) {
    info->lon = ((pack->ew == 'E') ?
        pack->longitude :
        -pack->longitude);
    nmeaInfoSetPresent(&info->present, LON);
  }

  if (nmeaInfoIsPresentAll(pack->present, SPEED)) {
    info->speed = pack->speedN * NMEALIB_TUD_KNOTS;
    nmeaInfoSetPresent(&info->present, SPEED);
  }

  if (nmeaInfoIsPresentAll(pack->present, TRACK)) {
    info->track = pack->track;
    nmeaInfoSetPresent(&info->present, TRACK);
  }

  if (nmeaInfoIsPresentAll(pack->present, UTCDATE)) {
    info->utc.year = pack->utc.year;
    info->utc.mon = pack->utc.mon;
    info->utc.day = pack->utc.day;
    nmeaInfoSetPresent(&info->present, UTCDATE);
  }

  if (nmeaInfoIsPresentAll(pack->present, MAGVAR)) {
    info->magvar = ((pack->magvar_ew == 'E') ?
        pack->magvar :
        -pack->magvar);
    nmeaInfoSetPresent(&info->present, MAGVAR);
  }
}

void nmeaGPRMCFromInfo(const NmeaInfo *info, nmeaGPRMC *pack) {
  if (!pack //
      || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  pack->v23 = true;

  if (nmeaInfoIsPresentAll(info->present, UTCTIME)) {
    pack->utc.hour = info->utc.hour;
    pack->utc.min = info->utc.min;
    pack->utc.sec = info->utc.sec;
    pack->utc.hsec = info->utc.hsec;
    nmeaInfoSetPresent(&pack->present, UTCTIME);
  }

  if (nmeaInfoIsPresentAll(info->present, SIG)) {
    pack->sigSelection = ((info->sig != NMEALIB_SIG_INVALID) ?
        'A' :
        'V');
    pack->sig = nmeaInfoSigToMode(info->sig);
    nmeaInfoSetPresent(&pack->present, SIG);
  }

  if (nmeaInfoIsPresentAll(info->present, LAT)) {
    pack->latitude = fabs(info->lat);
    pack->ns = ((info->lat >= 0.0) ?
        'N' :
        'S');
    nmeaInfoSetPresent(&pack->present, LAT);
  }

  if (nmeaInfoIsPresentAll(info->present, LON)) {
    pack->longitude = fabs(info->lon);
    pack->ew = ((info->lon >= 0.0) ?
        'E' :
        'W');
    nmeaInfoSetPresent(&pack->present, LON);
  }

  if (nmeaInfoIsPresentAll(info->present, SPEED)) {
    pack->speedN = info->speed / NMEALIB_TUD_KNOTS;
    nmeaInfoSetPresent(&pack->present, SPEED);
  }

  if (nmeaInfoIsPresentAll(info->present, TRACK)) {
    pack->track = info->track;
    nmeaInfoSetPresent(&pack->present, TRACK);
  }

  if (nmeaInfoIsPresentAll(info->present, UTCDATE)) {
    pack->utc.year = info->utc.year;
    pack->utc.mon = info->utc.mon;
    pack->utc.day = info->utc.day;
    nmeaInfoSetPresent(&pack->present, UTCDATE);
  }

  if (nmeaInfoIsPresentAll(info->present, MAGVAR)) {
    pack->magvar = fabs(info->magvar);
    pack->magvar_ew = ((info->magvar >= 0.0) ?
        'E' :
        'W');
    nmeaInfoSetPresent(&pack->present, MAGVAR);
  }
}

size_t nmeaGPRMCGenerate(char *s, const size_t sz, const nmeaGPRMC *pack) {

#define dst       (&s[chars])
#define available ((size_t) MAX((long) sz - 1 - chars, 0))

  int chars = 0;

  if (!s //
      || !pack) {
    return 0;
  }

  chars += snprintf(dst, available, "$" NMEALIB_PREFIX_GPRMC);

  if (nmeaInfoIsPresentAll(pack->present, UTCTIME)) {
    chars += snprintf(dst, available, //
        ",%02u%02u%02u.%02u", //
        pack->utc.hour, //
        pack->utc.min, //
        pack->utc.sec, //
        pack->utc.hsec);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, SIG) && pack->sigSelection) {
    chars += snprintf(dst, available, ",%c", pack->sigSelection);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, LAT)) {
    chars += snprintf(dst, available, ",%09.4f", pack->latitude);
    if (pack->ns) {
      chars += snprintf(dst, available, ",%c", pack->ns);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, LON)) {
    chars += snprintf(dst, available, ",%010.4f", pack->longitude);
    if (pack->ew) {
      chars += snprintf(dst, available, ",%c", pack->ew);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, SPEED)) {
    chars += snprintf(dst, available, ",%03.1f", pack->speedN);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, TRACK)) {
    chars += snprintf(dst, available, ",%03.1f", pack->track);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, UTCDATE)) {
    chars += snprintf(dst, available, //
        ",%02u%02u%02u", //
        pack->utc.day, //
        pack->utc.mon, //
        pack->utc.year % 100);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, MAGVAR)) {
    chars += snprintf(dst, available, ",%03.1f", pack->magvar);
    if (pack->magvar_ew) {
      chars += snprintf(dst, available, ",%c", pack->magvar_ew);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (pack->v23) {
    if (nmeaInfoIsPresentAll(pack->present, SIG) && pack->sig) {
      chars += snprintf(dst, available, ",%c", pack->sig);
    } else {
      chars += snprintf(dst, available, ",");
    }
  }

  /* checksum */
  chars += nmeaAppendChecksum(s, sz, (size_t) chars);

  return (size_t) chars;
}
