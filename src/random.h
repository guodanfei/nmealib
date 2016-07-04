#ifndef _NMEALIB_RANDOM_H
#define _NMEALIB_RANDOM_H

#include <nmealib/util.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NMEALIB_RANDOM_MAX LONG_MAX

static INLINE void nmeaRandomInit(void) {
  srandom((unsigned int) time(NULL));
}

static INLINE double nmeaRandom(const double min, const double max) {
  long value;
  int randomFile;
  double range = fabs(max - min);

  randomFile = open("/dev/urandom", O_RDONLY);
  if (randomFile == -1) {
    /* can't be tested */
    randomFile = open("/dev/random", O_RDONLY);
  }

  if ((randomFile == -1) //
      || (read(randomFile, &value, sizeof(value)) != sizeof(value))) {
    /* can't be tested */
    value = random();
  }

  if (randomFile != -1) {
    close(randomFile);
  }

  return min + ((fabs((double) value) * range) / (double) NMEALIB_RANDOM_MAX);
}

#endif /* _NMEALIB_RANDOM_H */
