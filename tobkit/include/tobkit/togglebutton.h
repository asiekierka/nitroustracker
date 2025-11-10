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

#ifndef TOGGLEBUTTON_H
#define TOGGLEBUTTON_H

#include "widget.h"

namespace tobkit {

/* A button that can be toggled on and off, like a switch. It can have a caption and an icon. */
class ToggleButton: public Widget {
	public:
		ToggleButton(u8 _x, u8 _y, u8 _width, u8 _height, u16 **_vram, bool _visible=true);
		~ToggleButton();

		// Callback registration
		void registerToggleCallback(void (*onToggle_)(bool));

		// Drawing request
		void pleaseDraw(void);

		// Event calls
		void penDown(u8 x, u8 y);
		void penUp(u8 x, u8 y);
		void buttonPress(u16 button);

		void setCaption(const char *_caption);
		void setBitmap(const u8 *_bmp, int width=13, int height=13);

		void setState(bool _on);
		bool getState(void);

	private:
		void draw(void);

		void (*onToggle)(bool);

		char *caption;
		const u8 *bitmap;
		bool penIsDown;
		bool on;
		bool has_bitmap;
		u8 bmpwidth, bmpheight;
};

};

#endif
