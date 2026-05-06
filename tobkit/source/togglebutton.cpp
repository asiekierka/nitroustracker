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

#include <string.h>
#include <stdlib.h>

#include "tobkit/togglebutton.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

ToggleButton::ToggleButton(u8 _x, u8 _y, u8 _width, u8 _height, u16 **_vram, bool _visible, bool _is_record)
	:Widget(_x, _y, _width, _height, _vram, _visible),
	penIsDown(false), on(false), has_bitmap(false), is_record(_is_record)
{
	onToggle = 0;
	caption = NULL;
}

ToggleButton::~ToggleButton()
{
	if (this->caption != NULL)
		ntxm_free(this->caption);
}

// Callback registration
void ToggleButton::registerToggleCallback(void (*onToggle_)(bool)) {
	onToggle = onToggle_;
}

// Drawing request
void ToggleButton::pleaseDraw(void) {
	draw();
}

// Event calls
void ToggleButton::penDown(u8 x, u8 y)
{
	if (!enabled) return;
	penIsDown = true;
	on = !on;
	draw();
	if(onToggle) {
		onToggle(on);
	}
}

void ToggleButton::penUp(u8 x, u8 y)
{
	penIsDown = false;
	draw();
}

void ToggleButton::buttonPress(u16 button)
{
	on = !on;
	draw();
	if(onToggle) {
		onToggle(on);
	}
}

void ToggleButton::setCaption(const char *_caption)
{
	if (this->caption != NULL)
		ntxm_free(this->caption);

	this->caption = ntxm_cstrdup(_caption);
}

void ToggleButton::setBitmap(const u8 *_bmp, int _width, int _height)
{
	has_bitmap = true;
	bitmap = _bmp;
	bmpwidth = _width;
	bmpheight = _height;
}

void ToggleButton::setState(bool _on)
{
	if(on != _on)
	{
		on = _on;
		draw();
		if(onToggle) {
			onToggle(on);
		}
	}
}

bool ToggleButton::getState(void)
{
	return on;
}

/* ===================== PRIVATE ===================== */

#define MAX(x,y)	((x)>(y)?(x):(y))

void ToggleButton::draw(void)
{
	if(!isExposed()) return;
	u16 bg1 = on ? theme->col_tb_bg_on_col1 : theme->col_tb_bg_off_col1;
	u16 bg2 = on ? theme->col_tb_bg_on_col2 : theme->col_tb_bg_off_col2;

	bg1 = enabled ? bg1 : theme->col_dark_ctrl_disabled;
	bg2 = enabled ? bg2 : theme->col_dark_ctrl_disabled;  

	drawGradient(bg1, bg2, 1, 1, width - 2, height - 2);
	drawBorder(theme->col_outline);
	
	u16 col;

	if(penIsDown) {
		if(on) {
			col = bg1;
		} else {
			col = theme->col_tb_fg_on;
		}
	} else {
		if(on) {
			col = theme->col_tb_fg_on;
		} else {
			col = is_record ? theme->col_signal_off : theme->col_tb_fg_off;
		}
	}
	if(has_bitmap) {
		drawMonochromeIcon(2, 2, bmpwidth, bmpheight, bitmap, col);
	}

	if (caption != NULL)
		drawString(caption, MAX(2, ((width-getStringWidth(caption))/2) ), height/2-5, col);
}

