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

#ifndef NUMBERSLIDERRELNOTE_H
#define NUMBERSLIDERRELNOTE_H

#include "tobkit/widget.h"

namespace tobkit {

class NumberSliderRelNote: public Widget {
	public:
		NumberSliderRelNote(u8 _x, u8 _y, u8 _width, u8 _height, uint16 **_vram, s32 _value=0);
	
		// Drawing request
		void pleaseDraw(void);
		
		// Event calls
		void penDown(u8 px, u8 py);
		void penUp(u8 px, u8 py);
		void penMove(u8 px, u8 py);

		void setValue(s32 val);
		s32 getValue(void);
		
		// Callback registration
		void registerChangeCallback(void (*onChange_)(s32));
		
	private:
		void draw(void);
		
		void (*onChange)(s32);
		
		s32 value;
		u8 lasty;
		bool btnstate;
		s32 min, max;
};

};

#endif
