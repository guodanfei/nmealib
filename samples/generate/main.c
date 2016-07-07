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
#include <nmealib/nmath.h>
#include <nmealib/sentence.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
	NmeaInfo info;
	char *buff;
	size_t it;

	nmeaInfoClear(&info);
	nmeaTimeSet(&info.utc, &info.present, NULL);

	info.sig = NMEALIB_SIG_SENSITIVE;
	info.fix = NMEALIB_FIX_3D;
	info.lat = 5000.0;
	info.lon = 3600.0;
	info.speed = 2.14 * NMEALIB_TUS_MS;
	info.elv = 10.86;
	info.track = 45;
	info.mtrack = 55;
	info.magvar = 55;
	info.hdop = 2.3;
	info.vdop = 1.2;
	info.pdop = 2.594224354;

	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_SIG);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_FIX);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_LAT);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_LON);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_SPEED);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_ELV);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_TRACK);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_MTRACK);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_MAGVAR);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_HDOP);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_VDOP);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_PDOP);

	info.satinfo.inUseCount = (int) NMEALIB_MAX_SATELLITES;
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_SATINUSECOUNT);
	for (it = 0; it < NMEALIB_MAX_SATELLITES; it++) {
		info.satinfo.inUse[it] = (int) it + 1;
	}
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_SATINUSE);

	info.satinfo.inViewCount = (int) NMEALIB_MAX_SATELLITES;
	for (it = 0; it < NMEALIB_MAX_SATELLITES; it++) {
		info.satinfo.inView[it].prn = (int) it + 1;
		info.satinfo.inView[it].elevation = (int) (it * 10);
		info.satinfo.inView[it].azimuth = (int) it + 1;
		info.satinfo.inView[it].snr = 99 - (int) it;
	}
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_SATINVIEWCOUNT | NMEALIB_PRESENT_SATINVIEW);

  for (it = 0; it < 10; it++) {
    size_t gen_sz = nmeaSentenceFromInfo(&buff, &info, NMEALIB_SENTENCE_GPGGA | NMEALIB_SENTENCE_GPGSA | NMEALIB_SENTENCE_GPGSV | NMEALIB_SENTENCE_GPRMC | NMEALIB_SENTENCE_GPVTG);
    if (gen_sz && buff) {
      printf("%s\n", buff);
      free(buff);
      buff = NULL;
      usleep(500000);
      info.speed += .1;
    }
  }

	return 0;
}
