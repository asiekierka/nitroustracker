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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <nds.h>

#include "tobkit/bitbutton.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

BitButton::BitButton(u8 _x, u8 _y, u8 _width, u8 _height, uint16 **_vram, const u8 *_bitmap, u8 _bmpwidth,
	u8 _bmpheight, u8 _bmpx, u8 _bmpy, bool _visible)
	:Widget(_x, _y, _width, _height, _vram, _visible),
	 penIsDown(false), bitmap(_bitmap), bmpwidth(_bmpwidth), bmpheight(_bmpheight), bmpx(_bmpx), bmpy(_bmpy)
{
	onPush = 0;
}

void BitButton::registerPushCallback(void (*onPush_)(void)) {
	onPush = onPush_;
}

// Drawing request
void BitButton::pleaseDraw(void) {
	draw(penIsDown);
}

// Event calls
void BitButton::penDown(u8 x, u8 y)
{
	if(!enabled)
		return;
	penIsDown = true;
	draw(1);
}

void BitButton::penUp(u8 x, u8 y)
{
	penIsDown = false;
	draw(0);
	if(onPush) {
		onPush();
	}
}

void BitButton::buttonPress(u16 button)
{
	if(onPush) {
		onPush();
	}
}

/* ===================== PRIVATE ===================== */

void BitButton::draw(u8 down)
{
	if(!isExposed()) return;

	if(enabled)
	{
		if(down) {
			drawGradient(theme->col_light_ctrl, theme->col_dark_ctrl, 1, 1, width - 2, height - 2);
		} else {
			drawGradient(theme->col_dark_ctrl, theme->col_light_ctrl, 1, 1, width - 2, height - 2);
		}
	} else {
		drawGradient(theme->col_light_ctrl_disabled, theme->col_dark_ctrl_disabled, 1, 1, width - 2, height - 2);
	}
	drawBorder(theme->col_outline);

	drawMonochromeIcon(bmpx, bmpy, bmpwidth, bmpheight, bitmap, theme->col_text_bt);
}
