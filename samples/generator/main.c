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
#include <nmealib/sentence.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
	NmeaGenerator *gen;
	NmeaInfo info;
	char *buff;
	size_t it;

	nmeaInfoClear(&info);
	nmeaTimeSet(&info.utc, &info.present, NULL);

	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_PDOP);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_HDOP);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_VDOP);
	nmeaInfoSetPresent(&info.present, NMEALIB_PRESENT_ELV);

	if (0 == (gen = nmea_create_generator(NMEALIB_GEN_ROTATE, &info)))
		return -1;

  for (it = 0; it < 10000; it++) {
    size_t gen_sz = nmea_generate_from(&buff, &info, gen, GPGGA | GPGSA | GPGSV | GPRMC | GPVTG);
    if (gen_sz && buff) {
      printf("%s\n", buff);
      free(buff);
      buff = NULL;
      usleep(500000);
    }
  }

	nmea_gen_destroy(gen);

	return 0;
}
