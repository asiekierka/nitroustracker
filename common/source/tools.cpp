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

#include "tools.h"
#include <ntxm/ntxmtools.h>

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/dir.h>

// Helper for converting a string to lower case
void lowercase(char *str)
{
	for(u8 i=0;i<strlen(str);++i) {
		if((str[i]>=65)&&(str[i]<=90)) {
			str[i]+=32;
		}
	}
}

bool dirExists(const char *path) {
	DIR *dir;
	if (!(dir = opendir(path))) {
		return false;
	} else {
		closedir(dir);
		return true;
	}
}

void dirCreate(const char *path) {
	if (!dirExists(path)) {
		mkdir(path, 0777);
	}
}

/* end */

void PrintFreeMem(void)
{
	printf("FreeMem=%dbyte    \n", ntxm_getFreeMem());
}

void printMallInfo(void)
{
	struct mallinfo mi = mallinfo();
	printf("non-inuse chunks: %d\n", mi.fordblks);
	printf("sbrk bytes:       %d\n", mi.arena);
	printf("mmap bytes:       %d\n", mi.hblkhd);
	printf("malloc chunks:    %d\n", mi.uordblks);
}