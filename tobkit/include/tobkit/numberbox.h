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

#ifndef NUMBERBOX_H
#define NUMBERBOX_H

#include "widget.h"

namespace tobkit {

class NumberBox: public Widget {
	public:
		NumberBox(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen, u8 _value=0, u8 _min=0, u8 _max=255, u8 _digits=2);
	
		// Drawing request
		void pleaseDraw(void);
		
		// Event calls
		void penDown(u16 px, u16 py);
		void penUp(u16 px, u16 py);

		void setValue(u8 val);
		
		// Callback registration
		void registerChangeCallback(void (*onChange_)(u8));
	private:
		void draw(void);
		
		void (*onChange)(u8);
		
		u8 value;
		u8 min, max, digits;
		u8 btnstate;
};

};

#endif
