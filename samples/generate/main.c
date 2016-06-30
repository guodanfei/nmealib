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

#include <nmealib/gmath.h>
#include <nmealib/sentence.h>

#include <stdio.h>
#include <unistd.h>

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
	NmeaInfo info;
	char buff[2048];
	size_t it;

	nmeaInfoClear(&info);

	info.sig = 3;
	info.fix = 3;
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

	nmeaInfoSetPresent(&info.present, SIG);
	nmeaInfoSetPresent(&info.present, FIX);
	nmeaInfoSetPresent(&info.present, LAT);
	nmeaInfoSetPresent(&info.present, LON);
	nmeaInfoSetPresent(&info.present, SPEED);
	nmeaInfoSetPresent(&info.present, ELV);
	nmeaInfoSetPresent(&info.present, TRACK);
	nmeaInfoSetPresent(&info.present, MTRACK);
	nmeaInfoSetPresent(&info.present, MAGVAR);
	nmeaInfoSetPresent(&info.present, HDOP);
	nmeaInfoSetPresent(&info.present, VDOP);
	nmeaInfoSetPresent(&info.present, PDOP);

	info.satinfo.inUseCount = (int) NMEALIB_MAX_SATELLITES;
	nmeaInfoSetPresent(&info.present, SATINUSECOUNT);
	for (it = 0; it < NMEALIB_MAX_SATELLITES; it++) {
		info.satinfo.inUse[it] = it + 1;
	}
	nmeaInfoSetPresent(&info.present, SATINUSE);

	info.satinfo.inViewCount = (int) NMEALIB_MAX_SATELLITES;
	for (it = 0; it < NMEALIB_MAX_SATELLITES; it++) {
		info.satinfo.inView[it].prn = it + 1;
		info.satinfo.inView[it].elevation = (it * 10);
		info.satinfo.inView[it].azimuth = it + 1;
		info.satinfo.inView[it].snr = 99 - it;
	}
	nmeaInfoSetPresent(&info.present, SATINVIEWCOUNT | SATINVIEW);

	for (it = 0; it < 10; it++) {
		size_t gen_sz = nmeaSentenceFromInfo(&buff[0], 2048, &info, GPGGA | GPGSA | GPGSV | GPRMC | GPVTG);

		buff[gen_sz] = 0;
		printf("%s\n", &buff[0]);

		usleep(500000);

		info.speed += .1;
	}

	return 0;
}
