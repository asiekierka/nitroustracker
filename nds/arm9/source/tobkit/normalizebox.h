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

#ifndef _NORMALIZEBOX_H_
#define _NORMALIZEBOX_H_

#include "tobkit/widget.h"
#include "tobkit/button.h"
#include "tobkit/label.h"
#include "tobkit/numberslider.h"
#include "tobkit/gui.h"

namespace tobkit {

#define NORMALIZEBOX_WIDTH		150
#define NORMALIZEBOX_HEIGHT		60

class NormalizeBox: public Widget {
	public:
		// Constructor sets base variables
		NormalizeBox(u16 **_vram, void (*_onOk)(void), void (*_onAuto)(void), void (*_onCancel)(void));
		~NormalizeBox(void);
		
		// Drawing request
		void pleaseDraw(void);
		
		// Event calls
		void penDown(u8 px, u8 py);
		void penMove(u8 px, u8 py);
		void penUp(u8 px, u8 py);
		
		s16 getValue(void);
		
		void show(void);
		void reveal(void);
		void setTheme(Theme *theme_, u16 bgcolor_);
		
	private:
		void draw(void);
		
		void (*onOk)(void);
		void (*onAuto)(void);
		void (*onCancel)(void);
		
		GUI gui;
		const char *title;
		Label *labelpercent;
		Button *buttonok, *buttonauto, *buttoncancel;
		NumberSlider *nspercent;
	
};

};

#endif
