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

#include "numbersliderrelnote.h"

#include <cstdlib>
#include <cstdio>

#include "tobkit/numberbox.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

NumberSliderRelNote::NumberSliderRelNote(u8 _x, u8 _y, u8 _width, u8 _height, uint16 **_vram, s32 _value)
	:Widget(_x, _y, _width, _height, _vram),
	value(_value), btnstate(0), min(-48), max(71)
{
	onChange = 0;
}

// Drawing request
void NumberSliderRelNote::pleaseDraw(void) {
	draw();
}

// Event calls
void NumberSliderRelNote::penDown(u8 px, u8 py)
{
	if((px>x)&&(px<x+32)&&(py>y)&&(py<y+17)) {
		btnstate = true;
		lasty = py;
	}
	
	draw();
}

void NumberSliderRelNote::penUp(u8 px, u8 py)
{
	btnstate = false;
	draw();
}

void NumberSliderRelNote::penMove(u8 px, u8 py)
{
	s16 dy = lasty-py;
	if(abs(dy)>3) {
		s16 newval = value+dy/4;
		if(newval > max) {
			value=max;
		} else if(newval<min) {
			value=min;
		} else {
			value=newval;
		}
		draw();
		
		if(onChange!=0) {
			onChange(value);
		}
		
		lasty = py;
	}
}

void NumberSliderRelNote::setValue(s32 val)
{
	if((val<=max)&&(val>=min)) {
		value = val;
		if(onChange!=0) {
			onChange(value);
		}
		draw();
	}
}

s32 NumberSliderRelNote::getValue(void) {
	return value;
}

// Callback registration
void NumberSliderRelNote::registerChangeCallback(void (*onChange_)(s32)) {
	onChange = onChange_;
}

/* ===================== PRIVATE ===================== */

void NumberSliderRelNote::draw(void)
{
	if(!isExposed())
		return;
	
	// Slider thingy
	if(btnstate==true) {
		drawGradient(theme->col_dark_ctrl, theme->col_light_ctrl, 1, 1, 8, 15);
	} else {
		drawGradient(theme->col_light_ctrl, theme->col_dark_ctrl, 1, 1, 8, 15);
	}
	
	// This draws the up-arrow
	s8 i,j;
	for(j=0;j<3;j++) {
		for(i=-j;i<=j;++i) {
			*(*vram+SCREEN_WIDTH*(y+j+3)+x+4+i) = theme->col_text_bt;
		}
	}
	
	// This draws the connection
	drawVLine(4, 6, 5, theme->col_text_bt);
	
	// This draws the down-arrow
	for(j=2;j>=0;j--) {
		for(i=-j;i<=j;++i) {
			*(*vram+SCREEN_WIDTH*(y-j+13)+x+4+i) = theme->col_text_bt;
		}
	}
	
	drawBox(0, 0, 9, 17, theme->col_outline);
	
	// Number display
	drawFullBox(9,1,width-10,height-2,theme->col_lighter_bg);
	
	u8 octave = (value+4*12) / 12;
	u8 note = (value+4*12)%12;
	
	const char *nstr = "";
	
	switch(note) {
		case(0): nstr = "c-"; break;
		case(1): nstr = "c#"; break;
		case(2): nstr = "d-"; break;
		case(3): nstr = "d#"; break;
		case(4): nstr = "e-"; break;
		case(5): nstr = "f-"; break;
		case(6): nstr = "f#"; break;
		case(7): nstr = "g-"; break;
		case(8): nstr = "g#"; break;
		case(9): nstr = "a-"; break;
		case(10): nstr = "a#"; break;
		case(11): nstr = "h-"; break;
	}
	
	char notestr[4];
	snprintf(notestr, sizeof(notestr), "%s%u", nstr, octave);
	drawString(notestr, 10, 5, theme->col_text_value);
	
	// Border
	drawBorder(theme->col_outline);
}
