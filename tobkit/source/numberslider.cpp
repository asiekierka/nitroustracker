/*====================================================================
Copyright 2006 Tobias Weyand

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

#include <stdlib.h>
#include <stdio.h>

#include "tobkit/numberslider.h"
#include "tobkit/numberbox.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

NumberSlider::NumberSlider(u8 _x, u8 _y, u8 _width, u8 _height, uint16 **_vram, s32 _value, s32 _min, s32 _max, bool _hex, bool _is_8bit)
	:Widget(_x, _y, _width, _height, _vram),
	value(_value), btnstate(0), min(_min), max(_max), hex(_hex), is_8bit(_is_8bit)
{
	onChange = 0;
	onPostChange = 0;
}

// Drawing request
void NumberSlider::pleaseDraw(void) {
	draw();
}

// Event calls
void NumberSlider::penDown(u8 px, u8 py)
{
	if((px>x)&&(px<x+32)&&(py>y)&&(py<y+17)) {
		btnstate = true;
		lasty = py;
	}
	
	if((px>x)&&(px<x+9)&&(py>y)&&(py<y+9)) {
		if(value<max)
		{
			value++;
			if(onChange!=0) {
				onChange(value);
			}
		}
	} else if((px>x)&&(px<x+9)&&(py>y+9)&&(py<y+18)) {
		if(value>min)
		{
			value--;
			if(onChange!=0) {
				onChange(value);
			}
		}
	}
	
	draw();
}

void NumberSlider::penUp(u8 px, u8 py)
{
	btnstate = false;

	if(onPostChange!=0) {
		onPostChange(value);
	}

	draw();
}

void NumberSlider::penMove(u8 px, u8 py)
{
	s16 dy = lasty-py;
	if(abs(dy)>1) {
		int inc = dy*dy/8;
		if(inc == 0)
				inc = 1;
		if(dy < 0)
			inc = -inc;

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

void NumberSlider::setValue(s32 val)
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
		
		draw();
	}
	
}

s32 NumberSlider::getValue(void) {
	return value;
}

// Callback registration
void NumberSlider::registerPostChangeCallback(void (*onPostChange_)(s32)) {
	onPostChange = onPostChange_;
}

void NumberSlider::registerChangeCallback(void (*onChange_)(s32)) {
	onChange = onChange_;
}

/* ===================== PRIVATE ===================== */

void NumberSlider::draw(void)
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
	int_fast8_t i,j;
	for(j=0;j<3;j++) {
		for(i=-j;i<=j;++i) {
			drawPixel(4+i, j+3, theme->col_text_bt);
		}
	}
	
	// This draws the connection
	drawVLine(4, 6, 5, theme->col_text_bt);
	
	// This draws the down-arrow
	for(j=2;j>=0;j--) {
		for(i=-j;i<=j;++i) {
			drawPixel(4+i, -j+13, theme->col_text_bt);
		}
	}
	
	drawBox(0, 0, 9, 17, theme->col_outline);
	
	// Number display
	drawFullBox(9,1,width-10,height-2,theme->col_lighter_bg);
	char numberstr[5];
	if (is_8bit)
	{
		snprintf(numberstr, sizeof(numberstr), hex ? "%2hhx" : "%3hhd", (int)value);
	} else {
		snprintf(numberstr, sizeof(numberstr), hex ? "%2lx" : "%3ld", value);
	}
	drawString(numberstr, 10, 5, theme->col_text_value);
	
	// Border
	drawBorder(theme->col_outline);
}
