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
TobKit BitButton Class
Bitmap Button. Has a nxn bitmap.
Should be n+4 x n+4 pixels in size.
**********************/

#ifndef BITBUTTON_H
#define BITBUTTON_H

#include "widget.h"

namespace tobkit {

class BitButton: public Widget {
	public:
		BitButton(u8 _x, u8 _y, u8 _width, u8 _height, Screen *_screen, const u8 *_bitmap,
			  u8 _bmpwidth=13, u8 _bmpheight=13, u8 _bmpx=2, u8 _bmpy=2, bool _visible=true);
		
		// Callback registration
		void registerPushCallback(void (*onPush_)(void));
		
		// Drawing request
		void pleaseDraw(void);
		
		// Event calls
		void penDown(u8 x, u8 y);
		void penUp(u8 x, u8 y);
		void buttonPress(u16 button);
		
	private:
		void (*onPush)(void);
		bool penIsDown;
		
		void draw(u8 down);
		
		const u8 *bitmap;
		u8 bmpwidth, bmpheight, bmpx, bmpy;
};

};

#endif
