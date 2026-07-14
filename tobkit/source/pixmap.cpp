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

#include "tobkit/pixmap.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

Pixmap::Pixmap(u16 _x, u16 _y, u16 _width, u16 _height, const u16 *_image,
               Screen *_screen, bool _visible)
    : Widget(_x, _y, _width, _height, _screen, _visible), onPush(0),
      image(_image)
{
}

Pixmap::~Pixmap()
{
}

// Callback registration
void Pixmap::registerPushCallback(void (*onPush_)(void))
{
	onPush = onPush_;
}

// Event calls
void Pixmap::penDown(u16 x, u16 y)
{
	if (onPush)
		onPush();
}

// Drawing request
void Pixmap::pleaseDraw(void)
{
	draw();
}

/* ===================== PRIVATE ===================== */

void Pixmap::draw(void)
{
	for (u16 j = 0; j < height; ++j) {
		for (u16 i = 0; i < width; ++i) {
			if (image[width * j + i] & RGB5A1_ALPHA_BIT)
				drawPixel(i, j, image[width * j + i]);
		}
	}
}
