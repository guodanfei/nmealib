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

#include <nmealib/generator.h>

#include <nmealib/context.h>
#include <nmealib/nmath.h>
#include <nmealib/sentence.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

bool nmeaGeneratorLoopNoise(NmeaGenerator *gen, NmeaInfo *info);

bool nmeaGeneratorLoopStatic(NmeaGenerator *gen, NmeaInfo *info);
bool nmeaGeneratorResetStatic(NmeaGenerator *gen, NmeaInfo *info);
bool nmeaGeneratorInitStatic(NmeaGenerator *gen, NmeaInfo *info);

bool nmeaGeneratorLoopRotate(NmeaGenerator *gen, NmeaInfo *info);
bool nmeaGeneratorResetRotate(NmeaGenerator *gen, NmeaInfo *info);
bool nmeaGeneratorInitRotate(NmeaGenerator *gen, NmeaInfo *info);

bool nmeaGeneratorInitRandomMove(NmeaGenerator *gen, NmeaInfo *info);
bool nmeaGeneratorLoopRandomMove(NmeaGenerator *gen, NmeaInfo *info);

/*
 * NOISE generator
 */

/**
 * NOISE Generator loop function.
 * Does not touch smask and utc in info.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return true on success, false otherwise
 */
bool nmeaGeneratorLoopNoise(NmeaGenerator *gen __attribute__ ((unused)), NmeaInfo *info) {
  size_t it;
  size_t in_use;

  info->sig = (int) lrint(nmeaRandom(NMEALIB_SIG_FIX, NMEALIB_SIG_SENSITIVE));
  info->fix = (int) lrint(nmeaRandom(NMEALIB_FIX_2D, NMEALIB_FIX_3D));
  info->pdop = nmeaRandom(0, 9);
  info->hdop = nmeaRandom(0, 9);
  info->vdop = nmeaRandom(0, 9);
  info->lat = nmeaRandom(0, 100);
  info->lon = nmeaRandom(0, 100);
  info->elv = nmeaRandom(-100, 100);
  info->speed = nmeaRandom(0, 100);
  info->track = nmeaRandom(0, 360);
  info->mtrack = nmeaRandom(0, 360);
  info->magvar = nmeaRandom(0, 360);

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SIG);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_FIX);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_PDOP);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_HDOP);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_VDOP);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LAT);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LON);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_ELV);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SPEED);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_TRACK);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_MTRACK);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_MAGVAR);

  info->satinfo.inUseCount = 0;
  info->satinfo.inViewCount = 0;

  for (it = 0; it < NMEALIB_MAX_SATELLITES; it++) {
    in_use = (size_t) labs(lrint(nmeaRandom(0, 3)));
    info->satinfo.inUse[it] = in_use ?
        (int) it :
        0;
    info->satinfo.inView[it].prn = (int) it;
    info->satinfo.inView[it].elevation = (int) lrint(nmeaRandom(0, 90));
    info->satinfo.inView[it].azimuth = (int) lrint(nmeaRandom(0, 359));
    info->satinfo.inView[it].snr = in_use ?
        (int) lrint(nmeaRandom(40, 99)) :
        (int) lrint(nmeaRandom(0, 40));

    if (in_use)
      info->satinfo.inUseCount++;
    if (info->satinfo.inView[it].snr > 0)
      info->satinfo.inViewCount++;
  }

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINUSECOUNT);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINUSE);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEWCOUNT);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEW);

  return true;
}

/*
 * STATIC generator
 */

/**
 * STATIC Generator loop function.
 * Only touches utc in info.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return true on success, false otherwise
 */
bool nmeaGeneratorLoopStatic(NmeaGenerator *gen __attribute__ ((unused)), NmeaInfo *info) {
  nmeaTimeSet(&info->utc, &info->present, NULL);
  return true;
}

/**
 * STATIC Generator reset function.
 * Resets only the satinfo to 4 sats in use and in view.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return true on success, false otherwise
 */
bool nmeaGeneratorResetStatic(NmeaGenerator *gen __attribute__ ((unused)), NmeaInfo *info) {
  info->satinfo.inUseCount = 4;
  info->satinfo.inViewCount = 4;

  info->satinfo.inUse[0] = 1;
  info->satinfo.inView[0].prn = 1;
  info->satinfo.inView[0].elevation = 50;
  info->satinfo.inView[0].azimuth = 0;
  info->satinfo.inView[0].snr = 99;

  info->satinfo.inUse[1] = 2;
  info->satinfo.inView[1].prn = 2;
  info->satinfo.inView[1].elevation = 50;
  info->satinfo.inView[1].azimuth = 90;
  info->satinfo.inView[1].snr = 99;

  info->satinfo.inUse[2] = 3;
  info->satinfo.inView[2].prn = 3;
  info->satinfo.inView[2].elevation = 50;
  info->satinfo.inView[2].azimuth = 180;
  info->satinfo.inView[2].snr = 99;

  info->satinfo.inUse[3] = 4;
  info->satinfo.inView[3].prn = 4;
  info->satinfo.inView[3].elevation = 50;
  info->satinfo.inView[3].azimuth = 270;
  info->satinfo.inView[3].snr = 99;

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINUSECOUNT);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINUSE);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEWCOUNT);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEW);

  return true;
}

/**
 * STATIC Generator initialiser function.
 * Only touches sig, fix and satinfo in info.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return true on success, false otherwise
 */
bool nmeaGeneratorInitStatic(NmeaGenerator *gen, NmeaInfo *info) {
  info->sig = NMEALIB_SIG_SENSITIVE;
  info->fix = NMEALIB_FIX_3D;

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SIG);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_FIX);

  nmeaGeneratorResetStatic(gen, info);

  return true;
}

/*
 * SAT_ROTATE generator
 */

/**
 * SAT_ROTATE Generator loop function.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return true on success, false otherwise
 */
bool nmeaGeneratorLoopRotate(NmeaGenerator *gen __attribute__ ((unused)), NmeaInfo *info) {
  int it;
  int count = info->satinfo.inViewCount;
  double deg = 360.0 / (count ?
      count :
      1);
  double srt = (count ?
      (info->satinfo.inView[0].azimuth) :
      0) + 5;

  nmeaTimeSet(&info->utc, &info->present, NULL);

  for (it = 0; it < count; it++) {
    info->satinfo.inView[it].azimuth = (int) ((srt >= 360) ?
        srt - 360 :
        srt);
    srt += deg;
  }

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEWCOUNT);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEW);

  return true;
}

/**
 * SAT_ROTATE Generator reset function.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return true on success, false otherwise
 */
bool nmeaGeneratorResetRotate(NmeaGenerator *gen __attribute__ ((unused)), NmeaInfo *info) {
  int it;
  double deg = 360 / 8;
  double srt = 0;

  info->satinfo.inUseCount = 8;
  info->satinfo.inViewCount = 8;

  for (it = 0; it < info->satinfo.inViewCount; it++) {
    info->satinfo.inUse[it] = it + 1;
    info->satinfo.inView[it].prn = it + 1;
    info->satinfo.inView[it].elevation = 5;
    info->satinfo.inView[it].azimuth = (int) srt;
    info->satinfo.inView[it].snr = 80;
    srt += deg;
  }

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINUSECOUNT);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINUSE);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEWCOUNT);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEW);

  return true;
}

/**
 * SAT_ROTATE Generator initialiser function.
 * Only touches sig, fix and satinfo in info.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return true on success, false otherwise
 */
bool nmeaGeneratorInitRotate(NmeaGenerator *gen, NmeaInfo *info) {
  info->sig = NMEALIB_SIG_SENSITIVE;
  info->fix = NMEALIB_FIX_3D;

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SIG);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_FIX);

  nmeaGeneratorResetRotate(gen, info);

  return true;
}

/*
 * POS_RANDMOVE generator
 */

/**
 * POS_RANDMOVE Generator initialiser function.
 * Only touches sig, fix, track, mtrack, magvar and speed in info.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return true on success, false otherwise
 */
bool nmeaGeneratorInitRandomMove(NmeaGenerator *gen __attribute__ ((unused)), NmeaInfo *info) {
  info->sig = NMEALIB_SIG_SENSITIVE;
  info->fix = NMEALIB_FIX_3D;
  info->speed = 20;
  info->track = 0;
  info->mtrack = 0;
  info->magvar = 0;

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SIG);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_FIX);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SPEED);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_TRACK);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_MTRACK);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_MAGVAR);

  return true;
}

/**
 * POS_RANDMOVE Generator loop function.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return true on success, false otherwise
 */
bool nmeaGeneratorLoopRandomMove(NmeaGenerator *gen __attribute__ ((unused)), NmeaInfo *info) {
  NmeaPosition crd;

  info->track += nmeaRandom(-10, 10);
  info->mtrack += nmeaRandom(-10, 10);
  info->speed += nmeaRandom(-2, 3);

  if (info->track < 0) {
    info->track = 359 + info->track;
  }
  if (info->track > 359) {
    info->track -= 359;
  }
  if (info->mtrack < 0) {
    info->mtrack = 359 + info->mtrack;
  }
  if (info->mtrack > 359) {
    info->mtrack -= 359;
  }

  if (info->speed > 40)
    info->speed = 40;
  if (info->speed < 1)
    info->speed = 1;

  nmeaMathInfoToPosition(info, &crd);
  nmeaMathMoveFlat(&crd, &crd, info->track, info->speed / 3600);
  nmeaMathPositionToInfo(&crd, info);

  info->magvar = info->track;

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LAT);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LON);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SPEED);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_TRACK);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_MTRACK);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_MAGVAR);

  return true;
}

/*
 * Generator
 */

bool nmeaGeneratorInit(NmeaGenerator *gen, NmeaInfo *info) {
  bool retval = true;
  uint32_t present = info->present;
  uint32_t smask = info->smask;
  NmeaGenerator *igen = gen;

  nmeaRandomInit();

  nmeaInfoClear(info);
  nmeaTimeSet(&info->utc, &info->present, NULL);

  info->present = present;
  info->smask = smask;
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SMASK);

  info->lat = NMEALIB_LATITUDE_DEFAULT_NDEG;
  info->lon = NMEALIB_LONGITUDE_DEFAULT_NDEG;
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LAT);
  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LON);

  while (retval && igen) {
    if (igen->init_call)
      retval = (*igen->init_call)(igen, info);
    igen = igen->next;
  }

  return retval;
}

NmeaGenerator * nmeaGeneratorCreate(NmeaGeneratorType type, NmeaInfo *info) {
  NmeaGenerator *gen = 0;

  switch (type) {
    case NMEALIB_GENERATOR_NOISE:
      if (!(gen = malloc(sizeof(NmeaGenerator))))
        nmeaContextError("%s: insufficient memory!", __FUNCTION__);
      else {
        memset(gen, 0, sizeof(NmeaGenerator));
        gen->loop_call = &nmeaGeneratorLoopNoise;
      }
      break;

    case NMEALIB_GENERATOR_STATIC:
    case NMEALIB_GENERATOR_SAT_STATIC:
      if (!(gen = malloc(sizeof(NmeaGenerator))))
        nmeaContextError("%s: insufficient memory!", __FUNCTION__);
      else {
        memset(gen, 0, sizeof(NmeaGenerator));
        gen->init_call = &nmeaGeneratorInitStatic;
        gen->loop_call = &nmeaGeneratorLoopStatic;
        gen->reset_call = &nmeaGeneratorResetStatic;
      }
      break;

    case NMEALIB_GENERATOR_SAT_ROTATE:
      if (!(gen = malloc(sizeof(NmeaGenerator))))
        nmeaContextError("%s: insufficient memory!", __FUNCTION__);
      else {
        memset(gen, 0, sizeof(NmeaGenerator));
        gen->init_call = &nmeaGeneratorInitRotate;
        gen->loop_call = &nmeaGeneratorLoopRotate;
        gen->reset_call = &nmeaGeneratorResetRotate;
      }
      break;

    case NMEALIB_GENERATOR_POS_RANDMOVE:
      if (!(gen = malloc(sizeof(NmeaGenerator))))
        nmeaContextError("%s: insufficient memory!", __FUNCTION__);
      else {
        memset(gen, 0, sizeof(NmeaGenerator));
        gen->init_call = &nmeaGeneratorInitRandomMove;
        gen->loop_call = &nmeaGeneratorLoopRandomMove;
      }
      break;

    case NMEALIB_GENERATOR_ROTATE:
    default:
      gen = nmeaGeneratorCreate(NMEALIB_GENERATOR_SAT_ROTATE, info);
      nmeaGeneratorAdd(gen, nmeaGeneratorCreate(NMEALIB_GENERATOR_POS_RANDMOVE, info));
      break;
  };

  if (gen) {
    nmeaGeneratorInit(gen, info);
  }

  return gen;
}

bool nmeaGeneratorReset(NmeaGenerator *gen, NmeaInfo *info) {
  bool RetVal = 1;

  if (gen->reset_call)
    RetVal = (*gen->reset_call)(gen, info);

  return RetVal;
}

void nmeaGeneratorDestroy(NmeaGenerator *gen) {
  if (gen->next) {
    nmeaGeneratorDestroy(gen->next);
    gen->next = 0;
  }

  if (gen->destroy_call)
    (*gen->destroy_call)(gen);

  free(gen);
}

bool nmeaGeneratorLoop(NmeaGenerator *gen, NmeaInfo *info) {
  bool retVal = true;

  if (gen->loop_call)
    retVal = (*gen->loop_call)(gen, info);

  if (retVal && gen->next)
    retVal = nmeaGeneratorLoop(gen->next, info);

  return retVal;
}

void nmeaGeneratorAdd(NmeaGenerator *to, NmeaGenerator *gen) {
  NmeaGenerator * next = to;
  while (next->next)
    next = to->next;

  next->next = gen;
}

size_t nmeaGeneratorGenerateFrom(char **s, NmeaInfo *info, NmeaGenerator *gen, unsigned int generate_mask) {
  size_t retval;

  if ((retval = nmeaGeneratorLoop(gen, info)))
    retval = nmeaSentenceFromInfo(s, info, generate_mask);

  return retval;
}
