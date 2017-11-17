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

#ifndef __NMEALIB_WINDOWSSUPPORT_H__
#define __NMEALIB_WINDOWSSUPPORT_H__

#ifdef _WIN32
# include <windows.h>
# define __attribute__(A) /* do nothing */
# define INLINE inline
# define gmtime_r(x, y) gmtime_s(y, x)
# define srandom(x) srand(x)
# define random(x) rand(x)
#else
# include <sys/time.h>
# include <unistd.h>
# define INLINE inline __attribute__((always_inline))
#endif

#endif /* __NMEALIB_WINDOWSSUPPORT_H__ */
