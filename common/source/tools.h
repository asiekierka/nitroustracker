/*
 * NitroTracker - An FT2-style tracker for the Nintendo DS
 *
 *                                by Tobias Weyand (0xtob)
 *
 * http://nitrotracker.tobw.net
 * http://code.google.com/p/nitrotracker
 */

/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TOOLS_H_
#define _TOOLS_H_

#include "config.h"
#include "ntxm/ntxmtools.h"

// A collection of utilities for everyday coding
#define debugprintf ntxm_dprintf
#if !defined(NT_PLATFORM_NDS)
#define sassert(...)                                                           \
	{                                                                          \
	}
#endif

#define ceil_f32toint(n) (((n) + ((1 << 12) - 1)) >> 12)

void lowercase(char *str);
void filterFilenameCharacters(char *text);
bool endsWithExtension(const char *text, const char *ext);
bool dirExists(const char *dir);
void dirCreate(const char *dir);

void PrintFreeMem(void);
void printMallInfo(void);

#ifdef __cplusplus
extern "C" {
#endif
#ifndef HAVE_STRLCAT
size_t strlcat(char *dst, const char *src, size_t dstsize);
#endif
#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t dstsize);
#endif
#ifdef __cplusplus
}
#endif

#endif
