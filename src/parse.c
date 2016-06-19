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
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

/** The size of the buffer to put a time string (that is to be parsed) into */
#define NMEA_TIMEPARSE_BUF  4096

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

/**
 * The type definition for a map from NMEA sentence prefix to sentence type
 */
typedef struct {
  const char * prefix;
  const enum nmeaPACKTYPE sentenceType;
} SentencePrefixToType;

/**
 * The map from NMEA sentence prefix to sentence type
 */
static const SentencePrefixToType sentencePrefixToType[] = {
    {
      .prefix = "GPGGA",
      .sentenceType = GPGGA
    },
    {
      .prefix = "GPGSA",
      .sentenceType = GPGSA
    },
    {
      .prefix = "GPGSV",
      .sentenceType = GPGSV
    },
    {
      .prefix = "GPRMC",
      .sentenceType = GPRMC
    },
    {
      .prefix = "GPVTG",
      .sentenceType = GPVTG
    },
    {
      .prefix = NULL,
      .sentenceType = GPNON
    }
};

/**
 * Parse nmeaTIME (time only, no date) from a string.
 *
 * The format that is used (hhmmss, hhmmss.s, hhmmss.ss or hhmmss.sss) is
 * determined by the length of the string.
 *
 * @param s The string
 * @param t The structure in which to store the parsed time
 * @return True on success, false otherwise
 */
static bool _nmea_parse_time(const char *s, nmeaTIME *t) {
  size_t sz;

  if (!s || !t) {
    return false;
  }

  sz = strlen(s);

  if (sz == 6) { // hhmmss
    t->hsec = 0;
    return (3 == sscanf(s, "%02d%02d%02d", &t->hour, &t->min, &t->sec));
  }

  if (sz == 8) { // hhmmss.s
    if (4 == sscanf(s, "%02d%02d%02d.%1d", &t->hour, &t->min, &t->sec, &t->hsec)) {
      t->hsec *= 10;
      return true;
    }
    return false;
  }

  if (sz == 9) { // hhmmss.ss
    return (4 == sscanf(s, "%02d%02d%02d.%02d", &t->hour, &t->min, &t->sec, &t->hsec));
  }

  if (sz == 10) { // hhmmss.sss
    if ((4 == sscanf(s, "%02d%02d%02d.%03d", &t->hour, &t->min, &t->sec, &t->hsec))) {
      t->hsec = (t->hsec + 5) / 10;
      return true;
    }
    return false;
  }

  nmea_error("Parse error: invalid time format in '%s'", s);

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
 * @return True on success, false otherwise
 */
static bool _nmea_parse_date(const int date, nmeaTIME *t) {
  if (!t) {
    return false;
  }

  if ((date < 0) || (date > 999999)) {
    nmea_error("Parse error: invalid time format in '%d'", date);
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
 * @return True when valid, false otherwise
 */
static bool validateTime(const nmeaTIME * t) {
  if (!t) {
    return false;
  }

  if (!( //
      (t->hour >= 0) && (t->hour <= 23) //
      && (t->min >= 0) && (t->min <= 59) //
      && (t->sec >= 0) && (t->sec <= 60) //
      && (t->hsec >= 0) && (t->hsec <= 99))) {
    nmea_error("Parse error: invalid time '%02d:%02d:%02d.%03d' (hh:mm:ss.mmm)", t->hour, t->min, t->sec, t->hsec * 10);
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
 * @return true when valid, false otherwise
 */
static bool validateDate(const nmeaTIME * t) {
  if (!t) {
    return false;
  }

  if (!( //
      (t->year >= 90) && (t->year <= 189) //
      && (t->mon >= 0) && (t->mon <= 11) //
      && (t->day >= 1) && (t->day <= 31))) {
    nmea_error("Parse error: invalid date '%02d-%02d-%02d' (dd-mm-yyyy)", t->day, t->mon, t->year + 1900);
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
 * @return True when valid, false otherwise
 */
static bool validateNSEW(char * c, const bool ns) {
  if (!c) {
    return false;
  }

  *c = toupper(*c);

  if (ns) {
    if (!((*c == 'N') || (*c == 'S'))) {
      nmea_error("Parse error: invalid North/South '%c'", *c);
      return false;
    }
  } else {
    if (!((*c == 'E') || (*c == 'W'))) {
      nmea_error("Parse error: invalid East/West '%c'", *c);
      return false;
    }
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
 * @return True when valid, false otherwise
 */
static bool validateSignal(int * sig) {
  if ((*sig < NMEA_SIG_FIRST) || (*sig > NMEA_SIG_LAST)) {
    nmea_error("Parse error: invalid signal %d, expected [%d, %d]", *sig, NMEA_SIG_FIRST, NMEA_SIG_LAST);
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
 * @return True when valid, false otherwise
 */
static bool validateMode(char * c) {
  if (!c) {
    return false;
  }

  *c = toupper(*c);

  if (!( //
      (*c == 'A') //
      || (*c == 'D') //
      || (*c == 'E') //
      || (*c == 'F') //
      || (*c == 'M') //
      || (*c == 'N') //
      || (*c == 'P') //
      || (*c == 'R') //
      || (*c == 'S'))) {
    nmea_error("Parse error: invalid mode '%c'", *c);
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

enum nmeaPACKTYPE nmea_parse_get_sentence_type(const char *s, const size_t sz) {
  size_t i = 0;

  if (!s || (sz < NMEA_PREFIX_LENGTH)) {
    return GPNON;
  }

  while (sentencePrefixToType[i].prefix) {
    if (!strncmp(s, sentencePrefixToType[i].prefix, NMEA_PREFIX_LENGTH)) {
      return sentencePrefixToType[i].sentenceType;
    }

    i++;
  }

  return GPNON;
}

bool nmea_parse_GPGGA(const char *s, const size_t sz, bool hasChecksum, nmeaGPGGA *pack) {
  bool r = false;
  const char * fmt = NULL;
  char * buf = NULL;
  int fields = 0;

  if (!pack) {
    goto out;
  }

  /* Clear before parsing, to be able to detect absent fields */
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

  if (!s) {
    goto out;
  }

  nmea_trace_buff(s, sz);

  if (hasChecksum) {
    fmt = "$GPGGA,%s,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f,%c,%f,%d*";
  } else {
    fmt = "$GPGGA,%s,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f,%c,%f,%d";
  }

  buf = malloc(NMEA_TIMEPARSE_BUF);
  *buf = '\0';

  /* parse */
  fields = nmea_scanf(s, sz, fmt, //
      buf, //
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
  if (fields != 14) {
    nmea_error("GPGGA parse error: need 14 tokens, got %d (%s)", fields, s);
    memset(&pack, 0, sizeof(pack));
    pack->sig = NMEA_SIG_INVALID;
    goto out;
  }

  /* determine which fields are present and validate them */

  buf[NMEA_TIMEPARSE_BUF - 1] = '\0';
  if (*buf) {
    if (!_nmea_parse_time(buf, &pack->time) || !validateTime(&pack->time)) {
      memset(&pack->time, 0, sizeof(pack->time));
      goto out;
    }

    nmea_INFO_set_present(&pack->present, UTCTIME);
  } else {
    memset(&pack->time, 0, sizeof(pack->time));
  }

  if (!isnan(pack->lat) && (pack->ns)) {
    if (!validateNSEW(&pack->ns, true)) {
      pack->lat = 0.0;
      pack->ns = '\0';
      goto out;
    }

    pack->lat = fabs(pack->lat);
    nmea_INFO_set_present(&pack->present, LAT);
  } else {
    pack->lat = 0.0;
    pack->ns = '\0';
  }

  if (!isnan(pack->lon) && (pack->ew)) {
    if (!validateNSEW(&pack->ew, false)) {
      pack->lon = 0.0;
      pack->ew = '\0';
      goto out;
    }

    pack->lon = fabs(pack->lon);
    nmea_INFO_set_present(&pack->present, LON);
  } else {
    pack->lon = 0.0;
    pack->ew = '\0';
  }

  if (pack->sig != INT_MAX) {
    if (!validateSignal(&pack->sig)) {
      pack->sig = NMEA_SIG_INVALID;
      goto out;
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
      nmea_error("GPGGA parse error: invalid elevation unit '%c'", pack->elv_units);
      pack->elv = 0.0;
      pack->elv_units = '\0';
      goto out;
    }

    nmea_INFO_set_present(&pack->present, ELV);
  } else {
    pack->elv = 0.0;
    pack->elv_units = '\0';
  }

  if (!isnan(pack->diff) && (pack->diff_units)) {
    if (pack->diff_units != 'M') {
      nmea_error("GPGGA parse error: invalid height unit '%c'", pack->diff_units);
      pack->diff = 0.0;
      pack->diff_units = '\0';
      goto out;
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

  r = true;

  out: free(buf);

  return r;
}

bool nmea_parse_GPGSA(const char *s, const size_t sz, bool hasChecksum, nmeaGPGSA *pack) {
  int token_count;
  int i;

  if (!hasChecksum) {
    return 0;
  }

  assert(s);
  assert(pack);

  nmea_trace_buff(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  pack->present = 0;
  pack->fix_mode = 0;
  pack->fix_type = -1;
  for (i = 0; i < NMEA_MAXSAT; i++) {
    pack->sat_prn[i] = 0;
  }
  pack->PDOP = NAN;
  pack->HDOP = NAN;
  pack->VDOP = NAN;

  /* parse */
  token_count = nmea_scanf(s, sz, "$GPGSA,%c,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f*", &pack->fix_mode,
      &pack->fix_type, &pack->sat_prn[0], &pack->sat_prn[1], &pack->sat_prn[2], &pack->sat_prn[3], &pack->sat_prn[4],
      &pack->sat_prn[5], &pack->sat_prn[6], &pack->sat_prn[7], &pack->sat_prn[8], &pack->sat_prn[9], &pack->sat_prn[10],
      &pack->sat_prn[11], &pack->PDOP, &pack->HDOP, &pack->VDOP);

  /* see that there are enough tokens */
  if (token_count != 17) {
    nmea_error("GPGSA parse error: need 17 tokens, got %d in %s", token_count, s);
    return 0;
  }

  /* determine which fields are present and validate them */

  pack->fix_mode = toupper(pack->fix_mode);
  if (!((pack->fix_mode == 'A') || (pack->fix_mode == 'M'))) {
    nmea_error("GPGSA parse error: invalid fix mode (%c)", pack->fix_mode);
    return 0;
  }
  if (pack->fix_type != -1) {
    if (!((pack->fix_type >= NMEA_FIX_FIRST) && (pack->fix_type <= NMEA_FIX_LAST))) {
      nmea_error("GPGSA parse error: invalid fix type %d, expected [%d, %d]", pack->fix_type, NMEA_FIX_FIRST,
      NMEA_FIX_LAST);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, FIX);
  }
  for (i = 0; i < NMEA_MAXSAT; i++) {
    if (pack->sat_prn[i] != 0) {
      nmea_INFO_set_present(&pack->present, SATINUSE);
      break;
    }
  }
  if (!isnan(pack->PDOP)) {
    nmea_INFO_set_present(&pack->present, PDOP);
  }
  if (!isnan(pack->HDOP)) {
    nmea_INFO_set_present(&pack->present, HDOP);
  }
  if (!isnan(pack->VDOP)) {
    nmea_INFO_set_present(&pack->present, VDOP);
  }

  return 1;
}

bool nmea_parse_GPGSV(const char *s, const size_t sz, bool hasChecksum, nmeaGPGSV *pack) {
  int token_count;
  int token_count_expected;
  int sat_count;
  int sat_counted = 0;

  if (!hasChecksum) {
    return 0;
  }

  assert(s);
  assert(pack);

  nmea_trace_buff(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  memset(pack, 0, sizeof(nmeaGPGSV));

  /* parse */
  token_count = nmea_scanf(s, sz, "$GPGSV,%d,%d,%d,"
      "%d,%d,%d,%d,"
      "%d,%d,%d,%d,"
      "%d,%d,%d,%d,"
      "%d,%d,%d,%d*", &pack->pack_count, &pack->pack_index, &pack->sat_count, &pack->sat_data[0].id,
      &pack->sat_data[0].elv, &pack->sat_data[0].azimuth, &pack->sat_data[0].sig, &pack->sat_data[1].id,
      &pack->sat_data[1].elv, &pack->sat_data[1].azimuth, &pack->sat_data[1].sig, &pack->sat_data[2].id,
      &pack->sat_data[2].elv, &pack->sat_data[2].azimuth, &pack->sat_data[2].sig, &pack->sat_data[3].id,
      &pack->sat_data[3].elv, &pack->sat_data[3].azimuth, &pack->sat_data[3].sig);

  /* return if we have no sentences or satellites */
  if ((pack->pack_count < 1) || (pack->pack_count > NMEA_NSATPACKS) || (pack->pack_index < 1)
      || (pack->pack_index > pack->pack_count) || (pack->sat_count < 0) || (pack->sat_count > NMEA_MAXSAT)) {
    nmea_error("GPGSV parse error: inconsistent pack (count/index/satcount = %d/%d/%d)", pack->pack_count,
        pack->pack_index, pack->sat_count);
    return 0;
  }

  /* validate all satellite settings and count the number of satellites in the sentence */
  for (sat_count = 0; sat_count < NMEA_SATINPACK; sat_count++) {
    if (pack->sat_data[sat_count].id != 0) {
      if ((pack->sat_data[sat_count].id < 0)) {
        nmea_error("GPGSV parse error: invalid sat %d id (%d)", sat_count + 1, pack->sat_data[sat_count].id);
        return 0;
      }
      if ((pack->sat_data[sat_count].elv < -180) || (pack->sat_data[sat_count].elv > 180)) {
        nmea_error("GPGSV parse error: invalid sat %d elevation (%d)", sat_count + 1, pack->sat_data[sat_count].elv);
        return 0;
      }
      if ((pack->sat_data[sat_count].azimuth < 0) || (pack->sat_data[sat_count].azimuth >= 360)) {
        nmea_error("GPGSV parse error: invalid sat %d azimuth (%d)", sat_count + 1, pack->sat_data[sat_count].azimuth);
        return 0;
      }
      if ((pack->sat_data[sat_count].sig < 0) || (pack->sat_data[sat_count].sig > 99)) {
        nmea_error("GPGSV parse error: invalid sat %d signal (%d)", sat_count + 1, pack->sat_data[sat_count].sig);
        return 0;
      }
      sat_counted++;
    }
  }

  /* see that there are enough tokens */
  token_count_expected = (sat_counted * 4) + 3;
  if ((token_count < token_count_expected) || (token_count > (NMEA_SATINPACK * 4 + 3))) {
    nmea_error("GPGSV parse error: need %d tokens, got %d", token_count_expected, token_count);
    return 0;
  }

  /* determine which fields are present and validate them */

  if (pack->sat_count > 0) {
    nmea_INFO_set_present(&pack->present, SATINVIEW);
  }

  return 1;
}

bool nmea_parse_GPRMC(const char *s, const size_t sz, bool hasChecksum, nmeaGPRMC *pack) {
  int token_count;
  char time_buff[NMEA_TIMEPARSE_BUF];
  int date;

  if (!hasChecksum) {
    return 0;
  }

  assert(s);
  assert(pack);

  nmea_trace_buff(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  time_buff[0] = '\0';
  date = -1;
  pack->present = 0;
  pack->utc.year = -1;
  pack->utc.mon = -1;
  pack->utc.day = -1;
  pack->utc.hour = -1;
  pack->utc.min = -1;
  pack->utc.sec = -1;
  pack->utc.hsec = -1;
  pack->status = 0;
  pack->lat = NAN;
  pack->ns = 0;
  pack->lon = NAN;
  pack->ew = 0;
  pack->speed = NAN;
  pack->track = NAN;
  pack->magvar = NAN;
  pack->magvar_ew = 0;
  pack->mode = 0;

  /* parse */
  token_count = nmea_scanf(s, sz, "$GPRMC,%s,%c,%f,%c,%f,%c,%f,%f,%d,%f,%c,%c*", &time_buff[0], &pack->status,
      &pack->lat, &pack->ns, &pack->lon, &pack->ew, &pack->speed, &pack->track, &date, &pack->magvar, &pack->magvar_ew,
      &pack->mode);

  /* see that there are enough tokens */
  if ((token_count != 11) && (token_count != 12)) {
    nmea_error("GPRMC parse error: need 11 or 12 tokens, got %d in %s", token_count, s);
    return 0;
  }

  /* determine which fields are present and validate them */

  time_buff[NMEA_TIMEPARSE_BUF - 1] = '\0';
  if (*time_buff) {
    if (!_nmea_parse_time(&time_buff[0], &pack->utc)) {
      return 0;
    }

    if (!validateTime(&pack->utc)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, UTCTIME);
  }

  if (!pack->status) {
    pack->status = 'V';
  } else {
    pack->status = toupper(pack->status);
    if (!((pack->status == 'A') || (pack->status == 'V'))) {
      nmea_error("GPRMC parse error: invalid status (%c)", pack->status);
      return 0;
    }
  }
  if (!isnan(pack->lat) && (pack->ns)) {
    if (!validateNSEW(&pack->ns, true)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, LAT);
  }
  if (!isnan(pack->lon) && (pack->ew)) {
    if (!validateNSEW(&pack->ew, false)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, LON);
  }
  if (!isnan(pack->speed)) {
    nmea_INFO_set_present(&pack->present, SPEED);
  }
  if (!isnan(pack->track)) {
    nmea_INFO_set_present(&pack->present, TRACK);
  }

  if (date != -1) {
    if (!_nmea_parse_date(date, &pack->utc)) {
      return 0;
    }

    if (!validateDate(&pack->utc)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, UTCDATE);
  }

  if (!isnan(pack->magvar) && (pack->magvar_ew)) {
    if (!validateNSEW(&pack->magvar_ew, false)) {
      return 0;
    }

    nmea_INFO_set_present(&pack->present, MAGVAR);
  }
  if (token_count == 11) {
    pack->mode = 'A';
  } else {
    if (!pack->mode) {
      pack->mode = 'N';
    } else {
      if (!validateMode(&pack->mode)) {
        return 0;
      }
    }
  }

  return 1;
}

bool nmea_parse_GPVTG(const char *s, const size_t sz, bool hasChecksum, nmeaGPVTG *pack) {
  int token_count;

  if (!hasChecksum) {
    return 0;
  }

  assert(s);
  assert(pack);

  nmea_trace_buff(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  pack->present = 0;
  pack->track = NAN;
  pack->track_t = 0;
  pack->mtrack = NAN;
  pack->mtrack_m = 0;
  pack->spn = NAN;
  pack->spn_n = 0;
  pack->spk = NAN;
  pack->spk_k = 0;

  /* parse */
  token_count = nmea_scanf(s, sz, "$GPVTG,%f,%c,%f,%c,%f,%c,%f,%c*", &pack->track, &pack->track_t, &pack->mtrack,
      &pack->mtrack_m, &pack->spn, &pack->spn_n, &pack->spk, &pack->spk_k);

  /* see that there are enough tokens */
  if (token_count != 8) {
    nmea_error("GPVTG parse error: need 8 tokens, got %d in %s", token_count, s);
    return 0;
  }

  /* determine which fields are present and validate them */

  if (!isnan(pack->track) && (pack->track_t)) {
    pack->track_t = toupper(pack->track_t);
    if (pack->track_t != 'T') {
      nmea_error("GPVTG parse error: invalid track unit, got %c, expected T", pack->track_t);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, TRACK);
  }
  if (!isnan(pack->mtrack) && (pack->mtrack_m)) {
    pack->mtrack_m = toupper(pack->mtrack_m);
    if (pack->mtrack_m != 'M') {
      nmea_error("GPVTG parse error: invalid mtrack unit, got %c, expected M", pack->mtrack_m);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, MTRACK);
  }
  if (!isnan(pack->spn) && (pack->spn_n)) {
    pack->spn_n = toupper(pack->spn_n);
    if (pack->spn_n != 'N') {
      nmea_error("GPVTG parse error: invalid knots speed unit, got %c, expected N", pack->spn_n);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, SPEED);

    if (isnan(pack->spk)) {
      pack->spk = pack->spn * NMEA_TUD_KNOTS;
      pack->spk_k = 'K';
    }
  }
  if (!isnan(pack->spk) && (pack->spk_k)) {
    pack->spk_k = toupper(pack->spk_k);
    if (pack->spk_k != 'K') {
      nmea_error("GPVTG parse error: invalid kph speed unit, got %c, expected K", pack->spk_k);
      return 0;
    }

    nmea_INFO_set_present(&pack->present, SPEED);

    if (isnan(pack->spn)) {
      pack->spn = pack->spk / NMEA_TUD_KNOTS;
      pack->spn_n = 'N';
    }
  }

  return 1;
}
