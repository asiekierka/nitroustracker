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
DS Tracker Button Class
Very basic
Only Push event
**********************/

#ifndef BUTTON_H
#define BUTTON_H

#include "widget.h"

namespace tobkit {

class Button: public Widget {
	public:
		Button(u8 _x, u8 _y, u8 _width, u8 _height, Screen *_screen, bool _visible=true);
	
		~Button();
		
		// Callback registration
		void registerPushCallback(void (*onPush_)(void));
		
		// Drawing request
		void pleaseDraw(void);
		
		// Event calls
		void penDown(u8 x, u8 y);
		void penUp(u8 x, u8 y);
		void penMove(u8 x, u8 y);
		void buttonPress(u16 button);
		
		inline bool isPenDown(void) const { return penIsDown; }
		void setCaption(const char *caption);
		
	private:
		void (*onPush)(void);
		bool penIsDown;
		
		void draw(u8 down);
		char *caption;
};

};

#endif
