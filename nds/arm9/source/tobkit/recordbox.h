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

#ifndef _RECORDBOX_H_
#define _RECORDBOX_H_

#include "tobkit/widget.h"
#include "tobkit/button.h"
#include "tobkit/label.h"

#include "ntxm/song.h"

namespace tobkit {

#define RECORDBOX_WIDTH			150
#define RECORDBOX_HEIGHT		64
#define RECORDBOX_SOUNDDATA_SIZE	(256*1024) // 8 seconds at 16384 Hz, 16 Bit
#define RECORDBOX_SAMPLING_FREQ		16384
#define RECORDBOX_CROP_SAMPLES_END	500
#define RECORDBOX_CROP_SAMPLES_START	100

class RecordBox: public Widget {
	public:
		// Constructor sets base variables
		RecordBox(u16 **_vram, void (*_onOk)(void), void (*_onCancel)(void), Sample *_sample, Instrument *_instrument,
			    u8 _smpidx);
		~RecordBox(void);
		
		// Drawing request
		void pleaseDraw(void);
		
		// Event calls
		void penDown(u8 px, u8 py);
		void penUp(u8 px, u8 py);
		void buttonPress(u16 button);
		void buttonRelease(u16 button);
		
		Sample *getSample(void);
		void setTheme(Theme *theme_, u16 bgcolor_);
		
	private:
		void draw(void);
		void startRecording(void);
		void stopRecording(void);
		
		bool recording, btndown;

		void (*onOk)(void);
		void (*onCancel)(void);
		
		const char *title;
		Label *labelmsg, *labelmsg2, *labelrec;
		Button *buttoncancel;
		Sample *sample;
		Instrument *instrument;
		u8 smpidx;
		u16 *sound_data;
};

};

#endif
