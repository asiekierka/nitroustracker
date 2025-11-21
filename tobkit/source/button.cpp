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

#include "tobkit/button.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

Button::Button(u8 _x, u8 _y, u8 _width, u8 _height, uint16 **_vram, bool _visible)
	:Widget(_x, _y, _width, _height, _vram, _visible),
	 penIsDown(false), caption(0)
{
	onPush = 0;
}

Button::~Button()
{
	if (caption) ntxm_free(caption);
}

void Button::registerPushCallback(void (*onPush_)(void)) {
	onPush = onPush_;
}
		
// Drawing request
void Button::pleaseDraw(void) {
	draw(penIsDown);
}
		
// Event calls
void Button::penDown(u8 x, u8 y)
{
	if (!enabled) return;
	penIsDown = true;
	draw(1);
}

void Button::penUp(u8 x, u8 y)
{
	if (!enabled) return;
	penIsDown = false;
	draw(0);
	if(onPush) {
		onPush();
	}
}

void Button::penMove(u8 x, u8 y) {

}

void Button::buttonPress(u16 button) {
	if(onPush) {
		onPush();
	}
}

void Button::setCaption(const char *_caption) {
	if (caption) ntxm_free(caption);
	caption = (char*)ntxm_cmalloc(strlen(_caption)+1);
	strcpy(caption, _caption);
}

/* ===================== PRIVATE ===================== */

void Button::draw(u8 down) {
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
	
	drawString(caption, (width-getStringWidth(caption))/2, height/2-5, theme->col_text_bt);
}
