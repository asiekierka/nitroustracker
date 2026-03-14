/*====================================================================
Copyright 2025 R Ferreira

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
======================================================================*/

/*
like a numberbox, but it has two pairs of arrows,
and you can slide on it
*/

#include <stdlib.h>
#include <stdio.h>

#include "tobkit/digitbox.h"

using namespace tobkit;

#define DB_MARGIN_TOP 12

/* ===================== PUBLIC ===================== */

DigitBox::DigitBox(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen, u8 _value, u8 _min, u8 _max, u8 _digits)
	:Widget(_x, _y - DB_MARGIN_TOP, _width, _height + DB_MARGIN_TOP, _screen),
	value(_value), min(_min), max(_max), digits(_digits), btnstate(0), lasty(0), lastx(0)
{
	onChange = 0;
}	

void DigitBox::penMove(u16 px, u16 py)
{
	if (!enabled) return;

	if (lasty == 0)
	{
		lasty=py;
		return;
	}

	s16 dy = lasty-py;
	if(abs(dy)>0 && px > x + 8 &&  px < x + width - 8) {
		int inc = (dy*dy) >> 3;
		if (dy == 0)
			inc = 1;
		if(dy < 0)
			inc = -(inc * 2);

		s16 newval = value+inc;
			
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

// Drawing request
void DigitBox::pleaseDraw(void) {
	draw();
}

// Event calls
void DigitBox::penDown(u16 px, u16 py) {
	
	u8 oldvalue = value;
	py -= DB_MARGIN_TOP;

	if((px>x)&&(px<x+width/4)&&(py>y)&&(py<y+9)&&digits==2) { // top left arrow 
		btnstate = 1;
		if (value > max-0x10) value = value + 0x10 - max - 1; // wrap around if (value + 0x10) > 0xff. so 0xf0 wraps to 0x00
		else value+=0x10;
	} else if((px>x)&&(px<x+width/4)&&(py>y+9)&&(py<y+18)&&digits==2) { // bottom left arrow
		btnstate = 2;
		if (value < 0x10) value = max - 0x10 + value + 1; // likewise wrap around downwards
		else value-=0x10;
	} else if((px>x)&&(px>x+(width/4)*3)&&(py>y)&&(py<y+9)) { // top right arrow
		btnstate = 3;
		if(value<max) value++; else value = min;
	} else if((px>x)&&(px>x+(width/4)*3)&&(py>y+9)&&(py<y+18)) { // bottom right arrow
		btnstate = 4;
		if(value>min) value--; else value = max;
	}
	
	if(value!=oldvalue) {
		onChange(value);
		draw();
	}
}

void DigitBox::penUp(u16 px, u16 py) {
	lasty=0;
	btnstate = 0;
	draw();
}

void DigitBox::setValue(u8 val)
{
	s32 oldval = value;
	
	if(val > max)
		value = max;
	else if (val < min)
		value = min;
	else
		value = val;

	if(oldval != value)
	{
		if(onChange!=0)
			onChange(val);
		if(isExposed())
			draw();
	}
}

u8 DigitBox::getValue(void)
{
	return value;
}

void DigitBox::setSingleDigit(bool single_digit_mode)
{
	digits = single_digit_mode ? 1 : 2;
	
	if (theme)
		pleaseDraw();
}

// Callback registration
void DigitBox::registerChangeCallback(void (*onChange_)(u8)) {
	onChange = onChange_;
}

/* ===================== PRIVATE ===================== */

void DigitBox::draw(void)
{
	// Number display
	drawFullBox(9, 1 + DB_MARGIN_TOP, width-9, 17-1, theme->col_lighter_bg);
	
	u8 extra_offs = 0;
	char numberstr[5];

	char formatstr2[] = "%02x";
	char formatstr1[] = "%1x";
	if (digits != 2)
	{
		extra_offs = 3;
	}

	// Set no of digits (hacky, but there's no other way)
	// formatstr[1] = digits+48;
	snprintf(numberstr, sizeof(numberstr), digits == 2 ? formatstr2 : formatstr1, (digits == 2) ? value : (value & 0x0f));
	drawString(numberstr, 11 + extra_offs, 5 + DB_MARGIN_TOP, theme->col_text_value);

	// Probably quite dumb but avoids code duplication lol
	// k=0: left hand side of arrow buttons, k=1: right hand side of arrow buttons
	for (int k = 0; k < 2; ++k)
	{
		bool disable_left = (k == 0 && digits == 1);

		u8 offset = k * (width - 9); // right arrow pair offset

		u16 col1 = disable_left ? theme->col_dark_ctrl_disabled : theme->col_dark_ctrl;
		u16 col2 = disable_left ? theme->col_light_ctrl_disabled : theme->col_light_ctrl;
		
		// Upper Button
		if(btnstate==1 + 2 * k) { // button is currently pressed
			drawGradient(col1, col2, 1+offset, 1 + DB_MARGIN_TOP, 8, 8);
		} else {
			drawGradient(col2, col1, 1+offset, 1 + DB_MARGIN_TOP, 8, 8);
		}
		
		// This draws the up-arrow
		int_fast8_t i,j;
		for(j=0;j<3;j++) {
			for(i=-j;i<=j;++i) {
				drawPixel(4+i+offset, j+3+DB_MARGIN_TOP, theme->col_text_bt);
			}
		}
		
		drawBox(0+offset, 0+DB_MARGIN_TOP, 9, 9, theme->col_outline);
		
		// Lower Button
		if(btnstate==2 + 2 * k) {
			drawGradient(col1, col2, 1+offset, 8+DB_MARGIN_TOP, 8, 8);
		} else {
			drawGradient(col2, col1, 1+offset, 8+DB_MARGIN_TOP, 8, 8);
		}
		
		// This draws the down-arrow
		for(j=2;j>=0;j--) {
			for(i=-j;i<=j;++i) {
				drawPixel(4+i+offset, -j+13+DB_MARGIN_TOP, theme->col_text_bt);
			}
		}
		
		drawBox(0+offset, 8+DB_MARGIN_TOP, 9, 9, theme->col_outline);
	}
	
	
	
	// Border
	drawBox(0, 0+DB_MARGIN_TOP, width, height-DB_MARGIN_TOP, theme->col_outline);
}
