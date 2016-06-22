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

#include <nmea/conversions.h>
#include <nmea/gmath.h>
#include <string.h>
#include <math.h>
#include <assert.h>

void nmea_GPRMC2info(const nmeaGPRMC *pack, nmeaINFO *info) {
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

void nmea_info2GPRMC(const nmeaINFO *info, nmeaGPRMC *pack) {
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

void nmea_GPVTG2info(const nmeaGPVTG *pack, nmeaINFO *info) {
  if (!pack || !info) {
    return;
  }

  nmea_INFO_set_present(&info->present, SMASK);

  info->smask |= GPVTG;

  if (nmea_INFO_is_present(pack->present, TRACK)) {
    info->track = pack->track;
    nmea_INFO_set_present(&info->present, TRACK);
  }

  if (nmea_INFO_is_present(pack->present, MTRACK)) {
    info->mtrack = pack->mtrack;
    nmea_INFO_set_present(&info->present, MTRACK);
  }

  if (nmea_INFO_is_present(pack->present, SPEED)) {
    double speed;
    if (pack->spk_k) {
      speed = pack->spk;
    } else {
      speed = pack->spn * NMEA_TUD_KNOTS;
    }
    info->speed = speed;
    nmea_INFO_set_present(&info->present, SPEED);
  }
}

void nmea_info2GPVTG(const nmeaINFO *info, nmeaGPVTG *pack) {
  if (!pack || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (nmea_INFO_is_present(info->present, TRACK)) {
    pack->track = info->track;
    pack->track_t = 'T';
    nmea_INFO_set_present(&pack->present, TRACK);
  }

  if (nmea_INFO_is_present(info->present, MTRACK)) {
    pack->mtrack = info->mtrack;
    pack->mtrack_m = 'M';
    nmea_INFO_set_present(&pack->present, MTRACK);
  }

  if (nmea_INFO_is_present(info->present, SPEED)) {
    pack->spn = info->speed / NMEA_TUD_KNOTS;
    pack->spn_n = 'N';
    pack->spk = info->speed;
    pack->spk_k = 'K';
    nmea_INFO_set_present(&pack->present, SPEED);
  }
}
