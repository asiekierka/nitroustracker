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

#include "debug_helpers.h"
#include "tools.h"

void saveScreenshot(tobkit::Screen *top_screen, tobkit::Screen *bottom_screen)
{
    // FIXME: Reimplement
	/* debugprintf("Saving screenshot\n");
	u8 *screenbuf = (u8*)ntxm_cmalloc(256*192*3*2);
	u8 *screenptr = screenbuf;

	u16 col;
	for(u32 i=0;i<192*256;++i) {
		col = main_vram_front[i];
		*(screenptr++) = (col & 0x1F) << 3;
		col >>= 5;
		*(screenptr++) = (col & 0x1F) << 3;
		col >>= 5;
		*(screenptr++) = (col & 0x1F) << 3;
	}
	for(u32 i=0;i<192*256;++i) {
		col = sub_vram[i];
		*(screenptr++) = (col & 0x1F) << 3;
		col >>= 5;
		*(screenptr++) = (col & 0x1F) << 3;
		col >>= 5;
		*(screenptr++) = (col & 0x1F) << 3;
	}

	static u8 filenr = 0;
	char filename[255] = {0};
	sprintf(filename, "scr%02d.rgb", filenr);

	FILE *fileh;
	fileh = fopen(filename, "w");
	fwrite(screenbuf, 256*192*3*2, 1, fileh);
	fclose(fileh);

	ntxm_free(screenbuf);
	debugprintf("saved\n");

	filenr++; */
}

void dumpSample(Song *song, int instrument, int sample)
{
	static u8 smpfilenr = 0;
	char filename[255] = {0};
	sprintf(filename, "smp%02d.raw", smpfilenr);

	Instrument *inst = song->getInstrument(instrument);
	if(inst==0) return;
	Sample *smp = inst->getSample(sample);
	if(smp==0) return;
	void *data = smp->getData();
	u32 size = smp->getSize();

	debugprintf("saving sample\n");

	FILE *fileh;
	fileh = fopen(filename, "w");
	fwrite(data, size, 1, fileh);
	fclose(fileh);

	debugprintf("saved\n");
}