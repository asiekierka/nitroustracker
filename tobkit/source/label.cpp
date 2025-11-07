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
#include <stdio.h>

#include <nds.h>

#include "tobkit/label.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

Label::Label(u8 _x, u8 _y, u8 _width, u8 _height, uint16 **_vram, bool _has_border, bool _albino, bool _no_bg, bool _right_aligned)
	:Widget(_x, _y, _width, _height, _vram),
	onPush(0), caption(0), has_border(_has_border), is_albino(_albino), no_bg(_no_bg), right_aligned(_right_aligned)
{

}

Label::~Label(void)
{
	if (caption) free(caption);
}

// Callback registration
void Label::registerPushCallback(void (*onPush_)(void))
{
	onPush = onPush_;
}

// Drawing request
void Label::pleaseDraw(void) {
	if(isExposed())
		draw();
}

// Event calls
void Label::penDown(u8 x, u8 y)
{
	if(onPush != 0) {
		onPush();
	}
}

void Label::setCaption(const char *_caption)
{
	if (caption) free(caption);

	caption = (char*)malloc(strlen(_caption)+1);
	strcpy(caption, _caption);

	draw();
}

char *Label::getCaption(void) {
	return caption;
}

/* ===================== PRIVATE ===================== */

void Label::draw(void)
{
	if(!isExposed())
		return;

	u16 col_bg, col_text;
	if(is_albino)
	{
		col_bg = theme->col_bg;
		col_text = theme->col_text_light;
	}
	else
	{
		col_bg = theme->col_light_bg;
		col_text = theme->col_text;
	}

	int caption_x_offset = 0;
	int caption_y_offset = 0;

	if(has_border)
	{
		if(!no_bg) {
			drawFullBox(1, 1, width - 2, height - 2, theme->col_lighter_bg);
			col_text = theme->col_text_value; 
		}

		drawBorder(theme->col_outline);
		caption_x_offset += 2;
		caption_y_offset += 2;
		
	}
	else
	{
		if(!no_bg)
			drawFullBox(0, 0, width, height, col_bg);
	}

	if(caption) {
		int caption_width = width - (caption_x_offset * 2);
		if(right_aligned) {
			// TODO: this is a kludge...
			const char *caption_local = caption;
			int string_width = getStringWidth(caption_local);
			while (string_width > caption_width) {
				caption_local++;
				string_width = getStringWidth(caption_local);
			}
			drawString(caption_local,caption_x_offset,caption_y_offset,col_text,caption_width);
		} else {
			drawString(caption,caption_x_offset,caption_y_offset,col_text,caption_width);
		}
	}
}
