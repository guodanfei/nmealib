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
#include <nmea/gmath.h>
#include <nmea/tok.h>
#include <nmea/context.h>
#include <nmea/conversions.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <limits.h>

/** Invalid NMEA character: non-ASCII */
static const InvalidNMEACharacter invalidNonAsciiCharsName = {
  .character = '*',
  .description = "non-ascii character"
};

/** Invalid NMEA characters */
static const InvalidNMEACharacter invalidCharacters[] = {
    {
      .character = '$',
      .description = "sentence delimiter" },
    {
      .character = '*',
      .description = "checksum field delimiter" },
    {
      .character = '!',
      .description = "exclamation mark" },
    {
      .character = '\\',
      .description = "backslash" },
    {
      .character = '^',
      .description = "power" },
    {
      .character = '~',
      .description = "tilde" },
    {
      .character = '\0',
      .description = NULL }
};

static int cmp_int(const void *p1, const void *p2) {
  int prn1 = *((const int *) p1);
  int prn2 = *((const int *) p2);

  if (!prn1) {
    prn1 += 1000;
  }
  if (!prn2) {
    prn2 += 1000;
  }

  return (prn1 - prn2);
}

/**
 * Parse nmeaTIME (time only, no date) from a string.
 *
 * The format that is used (hhmmss, hhmmss.s, hhmmss.ss or hhmmss.sss) is
 * determined by the length of the string.
 *
 * @param s The string
 * @param t The structure in which to store the parsed time
 * @param prefix The NMEA prefix
 * @return True on success, false otherwise
 */
static bool _nmea_parse_time(const char *s, nmeaTIME *t, const char * prefix) {
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

/**
 * Validate the time fields in an nmeaTIME structure.
 *
 * Expects:
 * <pre>
 *   hour [0, 23]
 *   min  [0, 59]
 *   sec  [0, 60] (1 leap second)
 *   hsec [0, 99]
 * </pre>
 *
 * @param t The structure
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True when valid, false otherwise
 */
static bool validateTime(const nmeaTIME * t, const char * prefix, const char * s) {
  if (!t) {
    return false;
  }

  if (!( //
      (t->hour >= 0) && (t->hour <= 23) //
      && (t->min >= 0) && (t->min <= 59) //
      && (t->sec >= 0) && (t->sec <= 60) //
      && (t->hsec >= 0) && (t->hsec <= 99))) {
    nmea_error("%s parse error: invalid time '%02d:%02d:%02d.%03d' (hh:mm:ss.mmm) in '%s'", prefix, t->hour, t->min,
        t->sec, t->hsec * 10, s);
    return false;
  }

  return true;
}

/**
 * Validate the date fields in an nmeaTIME structure.
 *
 * Expects:
 * <pre>
 *   year  [90, 189]
 *   month [ 0,  11]
 *   day   [ 1,  31]
 * </pre>
 *
 * @param t a pointer to the structure
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return true when valid, false otherwise
 */
static bool validateDate(const nmeaTIME * t, const char * prefix, const char * s) {
  if (!t) {
    return false;
  }

  if (!( //
      (t->year >= 90) && (t->year <= 189) //
      && (t->mon >= 0) && (t->mon <= 11) //
      && (t->day >= 1) && (t->day <= 31))) {
    nmea_error("%s parse error: invalid date '%02d-%02d-%04d' (dd-mm-yyyy) in '%s'", prefix, t->day, t->mon,
        t->year + 1900, s);
    return false;
  }

  return true;
}

/**
 * Validate north/south or east/west and upper-case it.
 *
 * Expects:
 * <pre>
 *   c in { n, N, s, S } (for north/south)
 *   c in { e, E, w, W } (for east/west)
 * </pre>
 *
 * @param c The character, will also be converted to upper-case.
 * @param ns Evaluate north/south when true, evaluate east/west otherwise
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True when valid, false otherwise
 */
static bool validateNSEW(char * c, const bool ns, const char * prefix, const char * s) {
  if (!c) {
    return false;
  }

  *c = toupper(*c);

  if (ns) {
    if (!((*c == 'N') || (*c == 'S'))) {
      nmea_error("%s parse error: invalid North/South '%c' in '%s'", prefix, *c, s);
      return false;
    }
  } else {
    if (!((*c == 'E') || (*c == 'W'))) {
      nmea_error("%s parse error: invalid East/West '%c' in '%s'", prefix, *c, s);
      return false;
    }
  }

  return true;
}

/**
 * Validate a fix.
 *
 * Expects:
 * <pre>
 *   fix in [NMEA_FIX_FIRST, NMEA_FIX_LAST]
 * </pre>
 *
 * @param fix The fix
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True when valid, false otherwise
 */
static bool validateFix(int * fix, const char * prefix, const char * s) {
  if ((*fix < NMEA_FIX_FIRST) || (*fix > NMEA_FIX_LAST)) {
    nmea_error("%s parse error: invalid fix %d, expected [%d, %d] in '%s'", prefix, *fix, NMEA_FIX_FIRST, NMEA_FIX_LAST,
        s);
    return false;
  }

  return true;
}

/**
 * Validate a signal.
 *
 * Expects:
 * <pre>
 *   sig in [NMEA_SIG_FIRST, NMEA_SIG_LAST]
 * </pre>
 *
 * @param sig The signal
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True when valid, false otherwise
 */
static bool validateSignal(int * sig, const char * prefix, const char * s) {
  if ((*sig < NMEA_SIG_FIRST) || (*sig > NMEA_SIG_LAST)) {
    nmea_error("%s parse error: invalid signal %d, expected [%d, %d] in '%s'", prefix, *sig, NMEA_SIG_FIRST,
        NMEA_SIG_LAST, s);
    return false;
  }

  return true;
}

/**
 * Validate and upper-case the mode.
 *
 * Expects:
 * <pre>
 *   c in { A, D, E, F, M, N, P, R, S }
 *
 *   A = Autonomous
 *   D = Differential
 *   E = Estimated (dead reckoning)
 *   F = Float RTK (using floating integers)
 *   M = Manual input
 *   N = No fix
 *   P = Precise
 *   R = Real Time Kinematic (using fixed integers)
 *   S = Simulation mode
 * </pre>
 *
 * @param c The character, will also be converted to upper-case.
 * @param prefix The NMEA prefix
 * @param s The NMEA sentence
 * @return True when valid, false otherwise
 */
static bool validateMode(char * c, const char * prefix, const char * s) {
  if (!c) {
    return false;
  }

  *c = toupper(*c);

  if (!( //
         (*c == 'N') //
      || (*c == 'A') //
      || (*c == 'D') //
      || (*c == 'P') //
      || (*c == 'R') //
      || (*c == 'F') //
      || (*c == 'E') //
      || (*c == 'M') //
      || (*c == 'S'))) {
    nmea_error("%s parse error: invalid mode '%c' in '%s'", prefix, *c, s);
    return false;
  }

  return true;
}

const InvalidNMEACharacter * isInvalidNMEACharacter(const char * c) {
  size_t i = 0;
  char ch;

  if (!c) {
    return NULL;
  }

  ch = *c;

  if ((ch < 32) || (ch > 126)) {
    return &invalidNonAsciiCharsName;
  }

  while (invalidCharacters[i].description) {
    if (ch == invalidCharacters[i].character) {
      return &invalidCharacters[i];
    }

    i++;
  }

  return NULL;
}

const InvalidNMEACharacter * nmea_parse_sentence_has_invalid_chars(const char * s, const size_t sz) {
  size_t i = 0;

  if (!s || !sz) {
    return NULL;
  }

  for (i = 0; i < sz; i++) {
    const InvalidNMEACharacter * invalid = isInvalidNMEACharacter(&s[i]);
    if (invalid) {
      return invalid;
    }
  }

  return NULL;
}

bool nmea_parse_GPGGA(const char *s, const size_t sz, nmeaGPGGA *pack) {
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

bool nmea_parse_GPGSA(const char *s, const size_t sz, nmeaGPGSA *pack) {
  int fieldCount;
  int i;

  if (!pack) {
    return false;
  }

  if (!s) {
    goto err;
  }

  nmea_trace_buff(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  memset(pack, 0, sizeof(*pack));
  pack->fix = INT_MAX;
  pack->PDOP = NAN;
  pack->HDOP = NAN;
  pack->VDOP = NAN;

  /* parse */
  fieldCount = nmea_scanf(s, sz, //
      "$GPGSA,%c,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f*", //
      &pack->sig, //
      &pack->fix, //
      &pack->sat_prn[0], //
      &pack->sat_prn[1], //
      &pack->sat_prn[2], //
      &pack->sat_prn[3], //
      &pack->sat_prn[4], //
      &pack->sat_prn[5], //
      &pack->sat_prn[6], //
      &pack->sat_prn[7], //
      &pack->sat_prn[8], //
      &pack->sat_prn[9], //
      &pack->sat_prn[10], //
      &pack->sat_prn[11], //
      &pack->PDOP, //
      &pack->HDOP, //
      &pack->VDOP);

  /* see that there are enough tokens */
  if (fieldCount != 17) {
    nmea_error("GPGSA parse error: need 17 tokens, got %d '%s'", fieldCount, s);
    goto err;
  }

  /* determine which fields are present and validate them */

  if (pack->sig) {
    pack->sig = toupper(pack->sig);
    if (!((pack->sig == 'A') || (pack->sig == 'M'))) {
      nmea_error("GPGSA parse error: invalid selection mode '%c' in '%s'", pack->sig, s);
      goto err;
    }

    nmea_INFO_set_present(&pack->present, SIG);
  } else {
    pack->sig = '\0';
  }

  if (pack->fix != INT_MAX) {
    if (!validateFix(&pack->fix, "GPGSA", s)) {
      goto err;
    }

    nmea_INFO_set_present(&pack->present, FIX);
  } else {
    pack->fix = NMEA_FIX_BAD;
  }

  for (i = 0; i < GPGSA_SAT_COUNT; i++) {
    if (pack->sat_prn[i] != 0) {
      qsort(pack->sat_prn, GPGSA_SAT_COUNT, sizeof(int), cmp_int);

      nmea_INFO_set_present(&pack->present, SATINUSE);
      break;
    }
  }
  if (!nmea_INFO_is_present(pack->present, SATINUSE)) {
    memset(pack->sat_prn, 0, sizeof(pack->sat_prn));
  }

  if (!isnan(pack->PDOP)) {
    pack->PDOP = fabs(pack->PDOP);
    nmea_INFO_set_present(&pack->present, PDOP);
  } else {
    pack->PDOP = 0.0;
  }

  if (!isnan(pack->HDOP)) {
    pack->HDOP = fabs(pack->HDOP);
    nmea_INFO_set_present(&pack->present, HDOP);
  } else {
    pack->HDOP = 0.0;
  }

  if (!isnan(pack->VDOP)) {
    pack->VDOP = fabs(pack->VDOP);
    nmea_INFO_set_present(&pack->present, VDOP);
  } else {
    pack->VDOP = 0.0;
  }

  return true;

  err: memset(pack, 0, sizeof(*pack));
  pack->fix = NMEA_FIX_BAD;
  return false;
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
