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

const char * nmeaInfoSigToString(NmeaSignal sig) {
  switch (sig) {
    case NMEALIB_SIG_INVALID:
      return "INVALID";

    case NMEALIB_SIG_FIX:
      return "FIX";

    case NMEALIB_SIG_DIFFERENTIAL:
      return "DIFFERENTIAL";

    case NMEALIB_SIG_SENSITIVE:
      return "SENSITIVE";

    case NMEALIB_SIG_RTKIN:
      return "REAL TIME KINEMATIC";

    case NMEALIB_SIG_FLOAT_RTK:
      return "FLOAT REAL TIME KINEMATIC";

    case NMEALIB_SIG_ESTIMATED:
      return "ESTIMATED (DEAD RECKONING)";

    case NMEALIB_SIG_MANUAL:
      return "MANUAL";

    case NMEALIB_SIG_SIMULATION:
      return "SIMULATION";

    default:
      return NULL;
  }
}

NmeaSignal nmeaInfoModeToSig(char mode) {
  switch (mode) {
    case 'N':
      return NMEALIB_SIG_INVALID;

    case 'A':
      return NMEALIB_SIG_FIX;

    case 'D':
      return NMEALIB_SIG_DIFFERENTIAL;

    case 'P':
      return NMEALIB_SIG_SENSITIVE;

    case 'R':
      return NMEALIB_SIG_RTKIN;

    case 'F':
      return NMEALIB_SIG_FLOAT_RTK;

    case 'E':
      return NMEALIB_SIG_ESTIMATED;

    case 'M':
      return NMEALIB_SIG_MANUAL;

    case 'S':
      return NMEALIB_SIG_SIMULATION;

    default:
      return NMEALIB_SIG_INVALID;
  }
}

char nmeaInfoSigToMode(NmeaSignal sig) {
  switch (sig) {
    case NMEALIB_SIG_INVALID:
      return 'N';

    case NMEALIB_SIG_FIX:
      return 'A';

    case NMEALIB_SIG_DIFFERENTIAL:
      return 'D';

    case NMEALIB_SIG_SENSITIVE:
      return 'P';

    case NMEALIB_SIG_RTKIN:
      return 'R';

    case NMEALIB_SIG_FLOAT_RTK:
      return 'F';

    case NMEALIB_SIG_ESTIMATED:
      return 'E';

    case NMEALIB_SIG_MANUAL:
      return 'M';

    case NMEALIB_SIG_SIMULATION:
      return 'S';

    default:
      return 'N';
  }
}

const char * nmeaInfoFixToString(NmeaFix fix) {
  switch (fix) {
    case NMEALIB_FIX_BAD:
      return "BAD";

    case NMEALIB_FIX_2D:
      return "2D";

    case NMEALIB_FIX_3D:
      return "3D";

    default:
      return NULL;
  }
}

const char * nmeaInfoFieldToString(NmeaPresence field) {
  switch (field) {
    case NMEALIB_PRESENT_SMASK:
      return "SMASK";

    case NMEALIB_PRESENT_UTCDATE:
      return "UTCDATE";

    case NMEALIB_PRESENT_UTCTIME:
      return "UTCTIME";

    case NMEALIB_PRESENT_SIG:
      return "SIG";

    case NMEALIB_PRESENT_FIX:
      return "FIX";

    case NMEALIB_PRESENT_PDOP:
      return "PDOP";

    case NMEALIB_PRESENT_HDOP:
      return "HDOP";

    case NMEALIB_PRESENT_VDOP:
      return "VDOP";

    case NMEALIB_PRESENT_LAT:
      return "LAT";

    case NMEALIB_PRESENT_LON:
      return "LON";

    case NMEALIB_PRESENT_ELV:
      return "ELV";

    case NMEALIB_PRESENT_HEIGHT:
      return "HEIGHT";

    case NMEALIB_PRESENT_SPEED:
      return "SPEED";

    case NMEALIB_PRESENT_TRACK:
      return "TRACK";

    case NMEALIB_PRESENT_MTRACK:
      return "MTRACK";

    case NMEALIB_PRESENT_MAGVAR:
      return "MAGVAR";

    case NMEALIB_PRESENT_SATINUSECOUNT:
      return "SATINUSECOUNT";

    case NMEALIB_PRESENT_SATINUSE:
      return "SATINUSE";

    case NMEALIB_PRESENT_SATINVIEWCOUNT:
      return "SATINVIEWCOUNT";

    case NMEALIB_PRESENT_SATINVIEW:
      return "SATINVIEW";

    case NMEALIB_PRESENT_DGPSAGE:
      return "DGPSAGE";

    case NMEALIB_PRESENT_DGPSSID:
      return "DGPSSID";

    default:
      return NULL;
  }
}

void nmeaTimeSet(NmeaTime *utc, uint32_t * present, struct timeval *timeval) {
  struct timeval tp;
  struct tm tt;

  assert(utc);

  if (timeval) {
    gettimeofday(timeval, NULL);
  } else {
    gettimeofday(&tp, NULL);
  }

  gmtime_r(&tp.tv_sec, &tt);

  utc->year = (unsigned int) tt.tm_year + 1900;
  utc->mon = (unsigned int) tt.tm_mon + 1;
  utc->day = (unsigned int) tt.tm_mday;
  utc->hour = (unsigned int) tt.tm_hour;
  utc->min = (unsigned int) tt.tm_min;
  utc->sec = (unsigned int) tt.tm_sec;
  utc->hsec = (unsigned int) (tp.tv_usec / 10000);
  if (present) {
    nmeaInfoSetPresent(present, NMEALIB_PRESENT_UTCDATE | NMEALIB_PRESENT_UTCTIME);
  }
}

void nmeaInfoClear(NmeaInfo *info) {
  if (!info) {
    return;
  }

  memset(info, 0, sizeof(NmeaInfo));

  info->sig = NMEALIB_SIG_INVALID;
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SIG);

  info->fix = NMEALIB_FIX_BAD;
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_FIX);
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

  nmeaInfo->present = nmeaInfo->present & NMEALIB_INFO_PRESENT_MASK;

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_SMASK)) {
    nmeaInfo->smask = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_UTCDATE) || !nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_UTCTIME)) {
    nmeaTimeSet(&utc, NULL, NULL);
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_UTCDATE)) {
    nmeaInfo->utc.year = utc.year;
    nmeaInfo->utc.mon = utc.mon;
    nmeaInfo->utc.day = utc.day;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_UTCTIME)) {
    nmeaInfo->utc.hour = utc.hour;
    nmeaInfo->utc.min = utc.min;
    nmeaInfo->utc.sec = utc.sec;
    nmeaInfo->utc.hsec = utc.hsec;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_SIG)) {
    nmeaInfo->sig = NMEALIB_SIG_INVALID;
  } else {
    if ((nmeaInfo->sig < NMEALIB_SIG_FIRST) || (nmeaInfo->sig > NMEALIB_SIG_LAST)) {
      nmeaInfo->sig = NMEALIB_SIG_INVALID;
    }
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_FIX)) {
    nmeaInfo->fix = NMEALIB_FIX_BAD;
  } else {
    if ((nmeaInfo->fix < NMEALIB_FIX_FIRST) || (nmeaInfo->fix > NMEALIB_FIX_LAST)) {
      nmeaInfo->fix = NMEALIB_FIX_BAD;
    }
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_PDOP)) {
    nmeaInfo->pdop = 0;
  } else {
    nmeaInfo->pdop = fabs(nmeaInfo->pdop);
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_HDOP)) {
    nmeaInfo->hdop = 0;
  } else {
    nmeaInfo->hdop = fabs(nmeaInfo->hdop);
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_VDOP)) {
    nmeaInfo->vdop = 0;
  } else {
    nmeaInfo->vdop = fabs(nmeaInfo->vdop);
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_LAT)) {
    nmeaInfo->lat = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_LON)) {
    nmeaInfo->lon = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_ELV)) {
    nmeaInfo->elv = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_SPEED)) {
    nmeaInfo->speed = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_TRACK)) {
    nmeaInfo->track = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_MTRACK)) {
    nmeaInfo->mtrack = 0;
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_MAGVAR)) {
    nmeaInfo->magvar = 0;
  } else {
    nmeaInfo->magvar = fabs(nmeaInfo->magvar);
  }

  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_SATINUSECOUNT)) {
    nmeaInfo->satinfo.inUseCount = 0;
  }
  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_SATINUSE)) {
    memset(&nmeaInfo->satinfo.inUse, 0, sizeof(nmeaInfo->satinfo.inUse));
  }
  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_SATINVIEWCOUNT)) {
    nmeaInfo->satinfo.inViewCount = 0;
  }
  if (!nmeaInfoIsPresentAll(nmeaInfo->present, NMEALIB_PRESENT_SATINVIEW)) {
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

void nmeaInfoUnitConversion(NmeaInfo * info, bool toMetric) {
  if (!info) {
    return;
  }

  if (info->metric == toMetric) {
    return;
  }

  info->metric = toMetric;

  /* smask (already in correct format) */

  /* utc (already in correct format) */

  /* sig (already in correct format) */
  /* fix (already in correct format) */

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_PDOP)) {
    if (toMetric) {
      info->pdop = nmeaDopToMeters(info->pdop);
    } else {
      info->pdop = nmeaMetersToDop(info->pdop);
    }
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_HDOP)) {
    if (toMetric) {
      info->hdop = nmeaDopToMeters(info->hdop);
    } else {
      info->hdop = nmeaMetersToDop(info->hdop);
    }
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_VDOP)) {
    if (toMetric) {
      info->vdop = nmeaDopToMeters(info->vdop);
    } else {
      info->vdop = nmeaMetersToDop(info->vdop);
    }
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_LAT)) {
    if (toMetric) {
      info->lat = nmeaNdegToDegree(info->lat);
    } else {
      info->lat = nmeaDegreeToNdeg(info->lat);
    }
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_LON)) {
    if (toMetric) {
      info->lon = nmeaNdegToDegree(info->lon);
    } else {
      info->lon = nmeaDegreeToNdeg(info->lon);
    }
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
