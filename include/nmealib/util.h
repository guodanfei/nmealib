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

#ifndef __NMEALIB_UTIL_H__
#define __NMEALIB_UTIL_H__

#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef INLINE
#define INLINE inline __attribute__((always_inline))
#endif

#ifndef MAX
#define MAX(x,y) (((x) >= (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) (((x) <= (y)) ? (x) : (y))
#endif

/** The power-of-2 chunk size of a buffer allocation */
#define NMEALIB_BUFFER_CHUNK_SIZE (4096UL)

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_UTIL_H__ */
