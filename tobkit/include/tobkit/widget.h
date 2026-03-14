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

/**********************
DS Tracker Widget Class
Has rectangular area
**********************/

#ifndef WIDGET_H
#define WIDGET_H

#include <limits.h>

#include "theme.h"
#include "platform.h"
#include "screen.h"

namespace tobkit {

struct Font {
	u8 width, height;
	const u8* char_index;
	const u8* char_widths;
	const u8* data;
};

#define GLYPH_3X5_COUNT 47
#define GLYPH_3X5(c) (c == ':') ? 40 : ((c < 58 && c > 47) ? (c - 48) : (c - 55))
#define GLYPH_3X5_WIDTH 4

class Widget {
	public:
		// Constructor sets base variables
		Widget(u16 _x, u16 _y, u16 _width, u16 _height, Screen *screen, bool _visible=true, bool _occluded=true);
		virtual ~Widget(void) {}

		// Callback registration
		// ... is done in the class

		// Drawing request
		virtual void pleaseDraw(void) {};

		// Get/set position
		void getPos(u16 *_x, u16 *_y, u16 *_width, u16 *_height);
		void setPos(u16 _x, u16 _y);

		// Toggle visibility
		// Objects can be hidden either explicitly (using show/hide) or implicitly
		// (using reveal/occlude). Reveal/occlude are used eg if the object is on
		// a tab of a tabbox that is currently not selected. Show/hide is used to
		// hide controls that shall not be seen at the moment.

		virtual void show(void);
		virtual void hide(void);
		bool is_visible(void) { return visible; }
		bool set_visible(bool value);
		void set_overdraw(bool value);

		virtual void occlude(void);
		virtual void reveal(void);
		bool is_occluded(void) { return occluded; }
		bool set_occluded(bool value);

		// Resize
		void resize(u16 w, u16 h);

		// Toggle enabled/disabled
		virtual void enable(void);
		virtual void disable(void);
		bool is_enabled(void) { return enabled; }
		bool set_enabled(bool value);

		// Event calls
		virtual void penDown(u16 px, u16 py) {};
		virtual void penUp(u16 px, u16 py) {};
		virtual void penMove(u16 px, u16 py) {};
		virtual void buttonPress(u16 button) {};
		virtual void buttonRelease(u16 button) {};

		virtual void setTheme(Theme *theme_, u16 bgcolor_) { theme = theme_; bgcolor = bgcolor_;}

	protected:
		u16 x, y, width, height;
		bool enabled;
		bool do_overdraw;
		Screen *screen;
		Theme *theme;
		u16 bgcolor; // Color of the background (for hiding the widget)

		// Draw utility functions
		void drawString(const char* str, u16 tx, u16 ty, u16 color, u16 maxwidth=255, u16 maxheight=255);

		inline void drawSmallString(const char *message, u16 sx, u16 sy, u16 col)
		{
			size_t n_chars = strlen(message);
			for (size_t c = 0; c < n_chars; ++c)
			{
				char _c = message[c];
				if (_c == ' ') continue;

				drawSmallChar(GLYPH_3X5(_c), sx + c * GLYPH_3X5_WIDTH, sy, col);
			}
		}

		void drawSmallChar(u8 c, u16 cx, u16 cy, u16 col);
		void drawBox(u16 tx, u16 ty, u16 tw, u16 th, u16 col);
		void drawFullBox(u16 tx, u16 ty, u16 tw, u16 th, u16 col);
		void drawBorder(u16 col);
		void drawHLine(u16 tx, u16 ty, u16 length, u16 col);
		void drawVLine(u16 tx, u16 ty, u16 length, u16 col);
		void drawBresLine(u16 tx1, u16 ty1, u16 tx2, u16 ty2, u16 col);
		inline void drawPixel(u16 tx, u16 ty, u16 col) {
			screen->drawPixel(x+tx, y+ty, col);
		}
		void drawGradient(u16 col1, u16 col2, u16 tx, u16 ty, u16 tw, u16 th);

		inline const u16 interpolateColor(u16 col1, u16 col2, int alpha /* 0..4095 */) {
			return RGB5A1(
				(((RGB5A1_R(col1) - RGB5A1_R(col2)) * alpha) + (RGB5A1_R(col2) << 12)) >> 12,
				(((RGB5A1_G(col1) - RGB5A1_G(col2)) * alpha) + (RGB5A1_G(col2) << 12)) >> 12,
				(((RGB5A1_B(col1) - RGB5A1_B(col2)) * alpha) + (RGB5A1_B(col2) << 12)) >> 12,
				1);
		}

		void drawMonochromeIcon(u16 tx, u16 ty, u16 tw, u16 th, const u8 *icon, u16 color);
		void drawMonochromeIconOffset(u16 tx, u16 ty, u16 tw, u16 th, u16 ix, u16 iy, u16 iw, u16 ih, const u8 *icon, u16 color);

		// Stylus utility functions
		bool isInRect(u16 x, u16 y, u16 x1, u16 y1, u16 x2, u16 y2);

		// How wide is the string when rendered?
		u32 getStringWidth(const char *str, u16 limit=USHRT_MAX);

		// Can the widget be seen by the user?
		bool isExposed(void);

	private:
		bool visible, occluded;

		// Overdraw the object with its background color
		void overdraw(void);
};

};

#endif
