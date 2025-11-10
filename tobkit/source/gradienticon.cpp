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

#include "tobkit/gradienticon.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

GradientIcon::GradientIcon(u8 _x, u8 _y, u8 _width, u8 _height, const u32* _image, u16 **_vram, bool _visible)
	:Widget(_x, _y, _width, _height, _vram, _visible),
	onPush(0), image(_image)
{
	
}
	
GradientIcon::~GradientIcon()
{
	
}

// Callback registration
void GradientIcon::registerPushCallback(void (*onPush_)(void)) {
	onPush = onPush_;
}

// Event calls
void GradientIcon::penDown(u8 x, u8 y) {
	if(onPush) onPush();
}

// Drawing request
void GradientIcon::pleaseDraw(void)
{
	draw();
}

/* ===================== PRIVATE ===================== */

void GradientIcon::draw(void)
{
	u32 pos = 0;
	u32 pixel = 0;
	u16 colorFg;
	u32 colorStep = div32((1<<12), height);

	for(u32 j=0; j<height; j++) {
		colorFg = interpolateColor(theme->col_light_ctrl, theme->col_dark_ctrl, colorStep * j);
		for(u32 i=0; i<width; i++, pos++) {
			if (!(pos & 0x0F)) {
				pixel = image[pos >> 4];
			} else {
				pixel >>= 2;
			}
			if(pixel & 3)
				(*vram)[SCREEN_WIDTH*(y+j)+x+i] = (pixel & 2) ? theme->col_outline : colorFg;
		}
	}
}
