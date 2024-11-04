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

#include <stdlib.h>
#include <stdio.h>
#include <nds.h>
#include <algorithm>

// A collection of utilities for everyday DS coding

#ifdef DEBUG
#define debugprintf iprintf
#else
static inline void debugprintf(...) {}
#endif

#ifdef __BLOCKSDS__
#define iprintf printf
#define fiprintf fprintf
#define siprintf sprintf
#define sniprintf snprintf
#endif

#define ceil_f32toint(n) (((n) + ((1 << 12) - 1)) >> 12)

void lowercase(char *str);
void dbgWaitButton(void);
bool dirExists(const char *dir);
void dirCreate(const char *dir);

void PrintFreeMem(void);
void printMallInfo(void);

#endif
