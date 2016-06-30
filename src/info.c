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

#include <nmealib/info.h>

#include <nmealib/sentence.h>
#include <nmealib/gmath.h>

#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>

const char * nmeaInfoSigToString(int sig) {
  switch (sig) {
    case NMEA_SIG_INVALID:
      return "INVALID";

    case NMEA_SIG_FIX:
      return "FIX";

    case NMEA_SIG_DIFFERENTIAL:
      return "DIFFERENTIAL";

    case NMEA_SIG_SENSITIVE:
      return "SENSITIVE";

    case NMEA_SIG_RTKIN:
      return "REAL TIME KINEMATIC";

    case NMEA_SIG_FLOAT_RTK:
      return "FLOAT RTK";

    case NMEA_SIG_ESTIMATED:
      return "ESTIMATED (DEAD RECKONING)";

    case NMEA_SIG_MANUAL:
      return "MANUAL INPUT";

    case NMEA_SIG_SIMULATION:
      return "SIMULATION";

    default:
      return NULL;
  }
}

int nmeaInfoModeToSig(char mode) {
  switch (mode) {
    case 'N':
      return NMEA_SIG_INVALID;

    case 'A':
      return NMEA_SIG_FIX;

    case 'D':
      return NMEA_SIG_DIFFERENTIAL;

    case 'P':
      return NMEA_SIG_SENSITIVE;

    case 'R':
      return NMEA_SIG_RTKIN;

    case 'F':
      return NMEA_SIG_FLOAT_RTK;

    case 'E':
      return NMEA_SIG_ESTIMATED;

    case 'M':
      return NMEA_SIG_MANUAL;

    case 'S':
      return NMEA_SIG_SIMULATION;

    default:
      return NMEA_SIG_INVALID;
  }
}

char nmeaInfoSigToMode(int sig) {
  switch (sig) {
    case NMEA_SIG_INVALID:
      return 'N';

    case NMEA_SIG_FIX:
      return 'A';

    case NMEA_SIG_DIFFERENTIAL:
      return 'D';

    case NMEA_SIG_SENSITIVE:
      return 'P';

    case NMEA_SIG_RTKIN:
      return 'R';

    case NMEA_SIG_FLOAT_RTK:
      return 'F';

    case NMEA_SIG_ESTIMATED:
      return 'E';

    case NMEA_SIG_MANUAL:
      return 'M';

    case NMEA_SIG_SIMULATION:
      return 'S';

    default:
      return 'N';
  }
}

const char * nmeaInfoFixToString(int fix) {
  switch (fix) {
    case NMEA_FIX_BAD:
      return "BAD";

    case NMEA_FIX_2D:
      return "2D";

    case NMEA_FIX_3D:
      return "3D";

    default:
      return NULL;
  }
}

const char * nmeaInfoFieldToString(NmeaPresence field) {
  switch (field) {
    case SMASK:
      return "SMASK";

    case UTCDATE:
      return "UTCDATE";

    case UTCTIME:
      return "UTCTIME";

    case SIG:
      return "SIG";

    case FIX:
      return "FIX";

    case PDOP:
      return "PDOP";

    case HDOP:
      return "HDOP";

    case VDOP:
      return "VDOP";

    case LAT:
      return "LAT";

    case LON:
      return "LON";

    case ELV:
      return "ELV";

    case HEIGHT:
      return "HEIGHT";

    case SPEED:
      return "SPEED";

    case TRACK:
      return "TRACK";

    case MTRACK:
      return "MTRACK";

    case MAGVAR:
      return "MAGVAR";

    case SATINUSECOUNT:
      return "SATINUSECOUNT";

    case SATINUSE:
      return "SATINUSE";

    case SATINVIEWCOUNT:
      return "SATINVIEWCOUNT";

    case SATINVIEW:
      return "SATINVIEW";

    case DGPSAGE:
      return "DGPSAGE";

    case DGPSSID:
      return "DGPSSID";

    default:
      return NULL;
  }
}

void nmeaInfoTimeSetNow(NmeaTime *utc, uint32_t * present) {
  struct timeval tp;
  struct tm tt;

  assert(utc);

  gettimeofday(&tp, NULL);
  gmtime_r(&tp.tv_sec, &tt);

  utc->year = (unsigned int) tt.tm_year + 1900;
  utc->mon = (unsigned int) tt.tm_mon + 1;
  utc->day = (unsigned int) tt.tm_mday;
  utc->hour = (unsigned int) tt.tm_hour;
  utc->min = (unsigned int) tt.tm_min;
  utc->sec = (unsigned int) tt.tm_sec;
  utc->hsec = (unsigned int) (tp.tv_usec / 10000);
  if (present) {
    nmeaInfoSetPresent(present, UTCDATE | UTCTIME);
  }
}

void nmeaInfoClear(NmeaInfo *info) {
  if (!info) {
    return;
  }

  memset(info, 0, sizeof(NmeaInfo));

  nmeaInfoTimeSetNow(&info->utc, &info->present);

  info->sig = NMEA_SIG_INVALID;
  nmeaInfoSetPresent(&info->present, SIG);

  info->fix = NMEA_FIX_BAD;
  nmeaInfoSetPresent(&info->present, FIX);
}

void nmeaInfoSanitise(NmeaInfo *nmeaInfo) {
  double lat = 0;
  double lon = 0;
  double speed = 0;
  double track = 0;
  double mtrack = 0;
  double magvar = 0;
  bool latAdjusted = false;
  bool lonAdjusted = false;
  bool speedAdjusted = false;
  bool trackAdjusted = false;
  bool mtrackAdjusted = false;
  bool magvarAdjusted = false;
  NmeaTime utc;
  size_t inuseIndex;
  size_t inviewIndex;

  if (!nmeaInfo) {
    return;
  }

  nmeaInfo->present = nmeaInfo->present & NMEA_INFO_PRESENT_MASK;

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, SMASK)) {
    nmeaInfo->smask = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, UTCDATE) || !nmeaInfoIsPresentAll(nmeaInfo->present, UTCTIME)) {
    nmeaInfoTimeSetNow(&utc, NULL);
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, UTCDATE)) {
    nmeaInfo->utc.year = utc.year;
    nmeaInfo->utc.mon = utc.mon;
    nmeaInfo->utc.day = utc.day;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, UTCTIME)) {
    nmeaInfo->utc.hour = utc.hour;
    nmeaInfo->utc.min = utc.min;
    nmeaInfo->utc.sec = utc.sec;
    nmeaInfo->utc.hsec = utc.hsec;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, SIG)) {
    nmeaInfo->sig = NMEA_SIG_INVALID;
  } else {
    if ((nmeaInfo->sig < NMEA_SIG_FIRST) || (nmeaInfo->sig > NMEA_SIG_LAST)) {
      nmeaInfo->sig = NMEA_SIG_INVALID;
    }
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, FIX)) {
    nmeaInfo->fix = NMEA_FIX_BAD;
  } else {
    if ((nmeaInfo->fix < NMEA_FIX_FIRST) || (nmeaInfo->fix > NMEA_FIX_LAST)) {
      nmeaInfo->fix = NMEA_FIX_BAD;
    }
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, PDOP)) {
    nmeaInfo->pdop = 0;
  } else {
    nmeaInfo->pdop = fabs(nmeaInfo->pdop);
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, HDOP)) {
    nmeaInfo->hdop = 0;
  } else {
    nmeaInfo->hdop = fabs(nmeaInfo->hdop);
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, VDOP)) {
    nmeaInfo->vdop = 0;
  } else {
    nmeaInfo->vdop = fabs(nmeaInfo->vdop);
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, LAT)) {
    nmeaInfo->lat = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, LON)) {
    nmeaInfo->lon = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, ELV)) {
    nmeaInfo->elv = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, SPEED)) {
    nmeaInfo->speed = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, TRACK)) {
    nmeaInfo->track = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, MTRACK)) {
    nmeaInfo->mtrack = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, MAGVAR)) {
    nmeaInfo->magvar = 0;
  } else {
    nmeaInfo->magvar = fabs(nmeaInfo->magvar);
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, SATINUSECOUNT)) {
    nmeaInfo->satinfo.inUseCount = 0;
  }
  if (!nmeaInfoIsPresentAll(nmeaInfo->present, SATINUSE)) {
    memset(&nmeaInfo->satinfo.inUse, 0, sizeof(nmeaInfo->satinfo.inUse));
  }
  if (!nmeaInfoIsPresentAll(nmeaInfo->present, SATINVIEWCOUNT)) {
    nmeaInfo->satinfo.inViewCount = 0;
  }
  if (!nmeaInfoIsPresentAll(nmeaInfo->present, SATINVIEW)) {
    memset(&nmeaInfo->satinfo.inView, 0, sizeof(nmeaInfo->satinfo.inView));
  }

  /*
   * lat
   */

  lat = nmeaInfo->lat;
  lon = nmeaInfo->lon;

  /* force lat in [-18000, 18000] */
  while (lat < -18000.0) {
    lat += 36000.0;
    latAdjusted = true;
  }
  while (lat > 18000.0) {
    lat -= 36000.0;
    latAdjusted = true;
  }

  /* lat is now in [-18000, 18000] */

  /* force lat from <9000, 18000] in [9000, 0] */
  if (lat > 9000.0) {
    lat = 18000.0 - lat;
    lon += 18000.0;
    latAdjusted = true;
    lonAdjusted = true;
  }

  /* force lat from [-18000, -9000> in [0, -9000] */
  if (lat < -9000.0) {
    lat = -18000.0 - lat;
    lon += 18000.0;
    latAdjusted = true;
    lonAdjusted = true;
  }

  /* lat is now in [-9000, 9000] */

  if (latAdjusted) {
    nmeaInfo->lat = lat;
  }

  /*
   * lon
   */

  /* force lon in [-18000, 18000] */
  while (lon < -18000.0) {
    lon += 36000.0;
    lonAdjusted = true;
  }
  while (lon > 18000.0) {
    lon -= 36000.0;
    lonAdjusted = true;
  }

  /* lon is now in [-18000, 18000] */

  if (lonAdjusted) {
    nmeaInfo->lon = lon;
  }

  /*
   * speed
   */

  speed = nmeaInfo->speed;
  track = nmeaInfo->track;
  mtrack = nmeaInfo->mtrack;

  if (speed < 0.0) {
    speed = -speed;
    track += 180.0;
    mtrack += 180.0;
    speedAdjusted = true;
    trackAdjusted = true;
    mtrackAdjusted = true;
  }

  /* speed is now in [0, max> */

  if (speedAdjusted) {
    nmeaInfo->speed = speed;
  }

  /*
   * track
   */

  /* force track in [0, 360> */
  while (track < 0.0) {
    track += 360.0;
    trackAdjusted = true;
  }
  while (track >= 360.0) {
    track -= 360.0;
    trackAdjusted = true;
  }

  /* track is now in [0, 360> */

  if (trackAdjusted) {
    nmeaInfo->track = track;
  }

  /*
   * mtrack
   */

  /* force mtrack in [0, 360> */
  while (mtrack < 0.0) {
    mtrack += 360.0;
    mtrackAdjusted = true;
  }
  while (mtrack >= 360.0) {
    mtrack -= 360.0;
    mtrackAdjusted = true;
  }

  /* mtrack is now in [0, 360> */

  if (mtrackAdjusted) {
    nmeaInfo->mtrack = mtrack;
  }

  /*
   * magvar
   */

  magvar = nmeaInfo->magvar;

  /* force magvar in [0, 360> */
  while (magvar < 0.0) {
    magvar += 360.0;
    magvarAdjusted = true;
  }
  while (magvar >= 360.0) {
    magvar -= 360.0;
    magvarAdjusted = true;
  }

  /* magvar is now in [0, 360> */

  if (magvarAdjusted) {
    nmeaInfo->magvar = magvar;
  }

  /*
   * satinfo
   */

  nmeaInfo->satinfo.inUseCount = 0;
  for (inuseIndex = 0; inuseIndex < NMEALIB_MAX_SATELLITES; inuseIndex++) {
    if (nmeaInfo->satinfo.inUse[inuseIndex])
      nmeaInfo->satinfo.inUseCount++;
  }

  nmeaInfo->satinfo.inViewCount = 0;
  for (inviewIndex = 0; inviewIndex < NMEALIB_MAX_SATELLITES; inviewIndex++) {
    if (nmeaInfo->satinfo.inView[inviewIndex].prn) {
      nmeaInfo->satinfo.inViewCount++;

      /* force elv in [-180, 180] */
      while (nmeaInfo->satinfo.inView[inviewIndex].elevation < -180) {
        nmeaInfo->satinfo.inView[inviewIndex].elevation += 360;
      }
      while (nmeaInfo->satinfo.inView[inviewIndex].elevation > 180) {
        nmeaInfo->satinfo.inView[inviewIndex].elevation -= 360;
      }

      /* elv is now in [-180, 180] */

      /* force elv from <90, 180] in [90, 0] */
      if (nmeaInfo->satinfo.inView[inviewIndex].elevation > 90) {
        nmeaInfo->satinfo.inView[inviewIndex].elevation = 180 - nmeaInfo->satinfo.inView[inviewIndex].elevation;
      }

      /* force elv from [-180, -90> in [0, -90] */
      if (nmeaInfo->satinfo.inView[inviewIndex].elevation < -90) {
        nmeaInfo->satinfo.inView[inviewIndex].elevation = -180 - nmeaInfo->satinfo.inView[inviewIndex].elevation;
      }

      /* elv is now in [-90, 90] */

      if (nmeaInfo->satinfo.inView[inviewIndex].elevation < 0) {
        nmeaInfo->satinfo.inView[inviewIndex].elevation = -nmeaInfo->satinfo.inView[inviewIndex].elevation;
      }

      /* elv is now in [0, 90] */

      /* force azimuth in [0, 360> */
      while (nmeaInfo->satinfo.inView[inviewIndex].azimuth < 0) {
        nmeaInfo->satinfo.inView[inviewIndex].azimuth += 360;
      }
      while (nmeaInfo->satinfo.inView[inviewIndex].azimuth >= 360) {
        nmeaInfo->satinfo.inView[inviewIndex].azimuth -= 360;
      }
      /* azimuth is now in [0, 360> */

      /* force sig in [0, 99] */
      if (nmeaInfo->satinfo.inView[inviewIndex].snr < 0)
        nmeaInfo->satinfo.inView[inviewIndex].snr = 0;
      if (nmeaInfo->satinfo.inView[inviewIndex].snr > 99)
        nmeaInfo->satinfo.inView[inviewIndex].snr = 99;
    }
  }

  /* make sure the in_use IDs map to sat IDs */
  for (inuseIndex = 0; inuseIndex < NMEALIB_MAX_SATELLITES; inuseIndex++) {
    int inuseID = nmeaInfo->satinfo.inUse[inuseIndex];
    if (inuseID) {
      bool found = false;
      for (inviewIndex = 0; inviewIndex < NMEALIB_MAX_SATELLITES; inviewIndex++) {
        int inviewID = nmeaInfo->satinfo.inView[inviewIndex].prn;
        if (inuseID == inviewID) {
          found = true;
          break;
        }
      }
      if (!found) {
        /* clear the id, did not find it */
        nmeaInfo->satinfo.inUse[inuseIndex] = 0;
        if (nmeaInfo->satinfo.inUseCount)
          nmeaInfo->satinfo.inUseCount--;
      }
    }
  }
}

/**
 * Converts the position fields to degrees and DOP fields to meters so that
 * all fields use normal metric units.
 *
 * @param nmeaInfo
 * the nmeaINFO
 */
void nmeaInfoUnitConversion(NmeaInfo * nmeaInfo) {
  if (!nmeaInfo) {
    return;
  }

  /* smask (already in correct format) */

  /* utc (already in correct format) */

  /* sig (already in correct format) */
  /* fix (already in correct format) */

  if (nmeaInfoIsPresentAll(nmeaInfo->present, PDOP)) {
    nmeaInfo->pdop = nmea_dop2meters(nmeaInfo->pdop);
  }

  if (nmeaInfoIsPresentAll(nmeaInfo->present, HDOP)) {
    nmeaInfo->hdop = nmea_dop2meters(nmeaInfo->hdop);
  }

  if (nmeaInfoIsPresentAll(nmeaInfo->present, VDOP)) {
    nmeaInfo->vdop = nmea_dop2meters(nmeaInfo->vdop);
  }

  if (nmeaInfoIsPresentAll(nmeaInfo->present, LAT)) {
    nmeaInfo->lat = nmea_ndeg2degree(nmeaInfo->lat);
  }

  if (nmeaInfoIsPresentAll(nmeaInfo->present, LON)) {
    nmeaInfo->lon = nmea_ndeg2degree(nmeaInfo->lon);
  }

  /* elv (already in correct format) */
  /* speed (already in correct format) */
  /* track (already in correct format) */
  /* mtrack (already in correct format) */
  /* magvar (already in correct format) */

  /* satinfo (already in correct format) */
}

int qsortComparePRN(const void *p1, const void *p2) {
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

int qsortCompactPRN(const void *p1, const void *p2) {
  int prn1 = *((const int *) p1);
  int prn2 = *((const int *) p2);

  if (prn1 && prn2) {
    return 0;
  }

  return qsortComparePRN(p1, p2);
}

int qsortCompareSatellite(const void *s1, const void *s2) {
  const NmeaSatellite *sat1 = (const NmeaSatellite *) s1;
  const NmeaSatellite *sat2 = (const NmeaSatellite *) s2;

  return qsortComparePRN(&sat1->prn, &sat2->prn);
}

int qsortCompactSatellite(const void *s1, const void *s2) {
  const NmeaSatellite *sat1 = (const NmeaSatellite *) s1;
  const NmeaSatellite *sat2 = (const NmeaSatellite *) s2;

  return qsortCompactPRN(&sat1->prn, &sat2->prn);
}
