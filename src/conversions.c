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

unsigned int nmea_gsv_npack(unsigned int satellites) {
  unsigned int pack_count = satellites / NMEA_SATINPACK;

  if ((satellites % NMEA_SATINPACK) > 0) {
    pack_count++;
  }

  if (!pack_count) {
    pack_count++;
  }

  return pack_count;
}

void nmea_GPGGA2info(const nmeaGPGGA *pack, nmeaINFO *info) {
  if (!pack || !info) {
    return;
  }

  nmea_INFO_set_present(&info->present, SMASK);

  info->smask |= GPGGA;

  if (nmea_INFO_is_present(pack->present, UTCTIME)) {
    info->utc.hour = pack->time.hour;
    info->utc.min = pack->time.min;
    info->utc.sec = pack->time.sec;
    info->utc.hsec = pack->time.hsec;
    nmea_INFO_set_present(&info->present, UTCTIME);
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

  if (nmea_INFO_is_present(pack->present, SIG)) {
    info->sig = pack->sig;
    nmea_INFO_set_present(&info->present, SIG);
  }

  if (nmea_INFO_is_present(pack->present, SATINUSECOUNT)) {
    info->satinfo.inuse = pack->satinuse;
    nmea_INFO_set_present(&info->present, SATINUSECOUNT);
  }

  if (nmea_INFO_is_present(pack->present, HDOP)) {
    info->HDOP = pack->HDOP;
    nmea_INFO_set_present(&info->present, HDOP);
  }

  if (nmea_INFO_is_present(pack->present, ELV)) {
    info->elv = pack->elv;
    nmea_INFO_set_present(&info->present, ELV);
  }

  /* diff and diff_units not supported yet */

  /* dgps_age not supported yet */

  /* dgps_sid not supported yet */
}

void nmea_info2GPGGA(const nmeaINFO *info, nmeaGPGGA *pack) {
  if (!pack || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (nmea_INFO_is_present(info->present, UTCTIME)) {
    pack->time.hour = info->utc.hour;
    pack->time.min = info->utc.min;
    pack->time.sec = info->utc.sec;
    pack->time.hsec = info->utc.hsec;
    nmea_INFO_set_present(&pack->present, UTCTIME);
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

  if (nmea_INFO_is_present(info->present, SIG)) {
    pack->sig = info->sig;
    nmea_INFO_set_present(&pack->present, SIG);
  } else {
    pack->sig = NMEA_SIG_INVALID;
  }

  if (nmea_INFO_is_present(info->present, SATINUSECOUNT)) {
    pack->satinuse = info->satinfo.inuse;
    nmea_INFO_set_present(&pack->present, SATINUSECOUNT);
  }

  if (nmea_INFO_is_present(info->present, HDOP)) {
    pack->HDOP = info->HDOP;
    nmea_INFO_set_present(&pack->present, HDOP);
  }

  if (nmea_INFO_is_present(info->present, ELV)) {
    pack->elv = info->elv;
    pack->elv_units = 'M';
    nmea_INFO_set_present(&pack->present, ELV);
  }

  /* diff and diff_units not supported yet */

  /* dgps_age not supported yet */

  /* dgps_sid not supported yet */
}

void nmea_GPGSA2info(const nmeaGPGSA *pack, nmeaINFO *info) {
  if (!pack || !info) {
    return;
  }

  nmea_INFO_set_present(&info->present, SMASK);

  info->smask |= GPGSA;

  if (nmea_INFO_is_present(pack->present, SIG)) {
    if (pack->sig == 'M') {
      info->sig = NMEA_SIG_MANUAL;
    } else if (nmea_INFO_is_present(pack->present, FIX)) {
      info->sig = NMEA_SIG_FIX;
    } else {
      info->sig = NMEA_SIG_INVALID;
    }

    nmea_INFO_set_present(&info->present, SIG);
  }

  if (nmea_INFO_is_present(pack->present, FIX)) {
    info->fix = pack->fix;
    nmea_INFO_set_present(&info->present, FIX);
  }

  if (nmea_INFO_is_present(pack->present, SATINUSE)) {
    unsigned int packIndex = 0;
    unsigned int infoIndex = 0;

    info->satinfo.inuse = 0;
    memset(&info->satinfo.in_use, 0, sizeof(info->satinfo.in_use[0]));

    for (packIndex = 0; (infoIndex < NMEA_MAXSAT) && (packIndex < GPGSA_SAT_COUNT); packIndex++) {
      int prn = pack->sat_prn[packIndex];
      if (prn) {
        info->satinfo.in_use[infoIndex++] = prn;
        info->satinfo.inuse++;
      }
    }
    nmea_INFO_set_present(&info->present, (SATINUSECOUNT | SATINUSE));
  }

  if (nmea_INFO_is_present(pack->present, PDOP)) {
    info->PDOP = pack->PDOP;
    nmea_INFO_set_present(&info->present, PDOP);
  }

  if (nmea_INFO_is_present(pack->present, HDOP)) {
    info->HDOP = pack->HDOP;
    nmea_INFO_set_present(&info->present, HDOP);
  }

  if (nmea_INFO_is_present(pack->present, VDOP)) {
    info->VDOP = pack->VDOP;
    nmea_INFO_set_present(&info->present, VDOP);
  }
}

void nmea_info2GPGSA(const nmeaINFO *info, nmeaGPGSA *pack) {
  if (!pack || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));
  pack->fix = NMEA_FIX_BAD;

  if (nmea_INFO_is_present(info->present, SIG)) {
    if (info->sig == NMEA_SIG_MANUAL) {
      pack->sig = 'M';
    } else {
      pack->sig = 'A';
    }

    nmea_INFO_set_present(&pack->present, SIG);
  }

  if (nmea_INFO_is_present(info->present, FIX)) {
    pack->fix = info->fix;
    nmea_INFO_set_present(&pack->present, FIX);
  }

  if (nmea_INFO_is_present(info->present, SATINUSE)) {
    unsigned int infoIndex = 0;
    unsigned int packIndex = 0;

    for (infoIndex = 0; (infoIndex < NMEA_MAXSAT) && (packIndex < GPGSA_SAT_COUNT); infoIndex++) {
      int prn = info->satinfo.in_use[infoIndex];
      if (prn) {
        pack->sat_prn[packIndex++] = prn;
      }
    }

    nmea_INFO_set_present(&pack->present, SATINUSE);
  }

  if (nmea_INFO_is_present(info->present, PDOP)) {
    pack->PDOP = info->PDOP;
    nmea_INFO_set_present(&pack->present, PDOP);
  }

  if (nmea_INFO_is_present(info->present, HDOP)) {
    pack->HDOP = info->HDOP;
    nmea_INFO_set_present(&pack->present, HDOP);
  }

  if (nmea_INFO_is_present(info->present, VDOP)) {
    pack->VDOP = info->VDOP;
    nmea_INFO_set_present(&pack->present, VDOP);
  }
}

void nmea_GPGSV2info(const nmeaGPGSV *pack, nmeaINFO *info) {
  int pack_index;

  assert(pack);
  assert(info);

  pack_index = pack->pack_index;
  if (pack_index < 1)
    pack_index = 1;

  if (pack_index > pack->pack_count)
    pack_index = pack->pack_count;

  if ((pack_index * NMEA_SATINPACK) > NMEA_MAXSAT)
    pack_index = NMEA_NSATPACKS;

  info->present |= pack->present;
  nmea_INFO_set_present(&info->present, SMASK);
  info->smask |= GPGSV;
  if (nmea_INFO_is_present(pack->present, SATINVIEW)) {
    int sat_index;

    /* index of 1st sat in pack */
    int sat_offset = (pack_index - 1) * NMEA_SATINPACK;
    /* the number of sats in this sentence */
    int sat_count = ((sat_offset + NMEA_SATINPACK) > pack->sat_count) ?
        (pack->sat_count - sat_offset) :
        NMEA_SATINPACK;

    for (sat_index = 0; sat_index < sat_count; sat_index++) {
      info->satinfo.sat[sat_offset + sat_index].id = pack->sat_data[sat_index].id;
      info->satinfo.sat[sat_offset + sat_index].elv = pack->sat_data[sat_index].elv;
      info->satinfo.sat[sat_offset + sat_index].azimuth = pack->sat_data[sat_index].azimuth;
      info->satinfo.sat[sat_offset + sat_index].sig = pack->sat_data[sat_index].sig;
    }

    info->satinfo.inview = pack->sat_count;
  }
}

void nmea_info2GPGSV(const nmeaINFO *info, nmeaGPGSV *pack, unsigned int pack_idx) {
  assert(pack);
  assert(info);

  nmea_zero_GPGSV(pack);

  pack->present = info->present;
  nmea_INFO_unset_present(&pack->present, SMASK);
  if (nmea_INFO_is_present(info->present, SATINVIEW)) {
    int sit;
    int pit;
    int toskip;

    pack->sat_count = (info->satinfo.inview < NMEA_MAXSAT) ?
        info->satinfo.inview :
        NMEA_MAXSAT;
    pack->pack_count = nmea_gsv_npack(pack->sat_count);

    if ((int) pack_idx >= pack->pack_count)
      pack->pack_index = pack->pack_count;
    else
      pack->pack_index = pack_idx + 1;

    /* now skip the first ((pack->pack_index - 1) * NMEA_SATINPACK) in view sats */
    toskip = ((pack->pack_index - 1) * NMEA_SATINPACK);
    sit = 0;
    while ((toskip > 0) && (sit < NMEA_MAXSAT)) {
      if (info->satinfo.sat[sit].id) {
        toskip--;
      }
      sit++;
    }

    for (pit = 0; pit < NMEA_SATINPACK; sit++) {
      if (sit < NMEA_MAXSAT) {
        if (info->satinfo.sat[sit].id) {
          pack->sat_data[pit] = info->satinfo.sat[sit];
          pit++;
        }
      } else {
        memset(&pack->sat_data[pit], 0, sizeof(pack->sat_data[pit]));
        pit++;
      }
    }
  }
}

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
