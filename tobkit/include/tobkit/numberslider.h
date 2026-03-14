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

#ifndef NUMBERSLIDER_H
#define NUMBERSLIDER_H

#include "widget.h"

namespace tobkit {

class NumberSlider: public Widget {
	public:
		NumberSlider(u8 _x, u8 _y, u8 _width, u8 _height, Screen *_screen, s32 _value=0, s32 _min=0, s32 _max=255,
						bool _hex=false, bool _is_8bit=false);
	
		// Drawing request
		void pleaseDraw(void);
		
		// Event calls
		void penDown(u8 px, u8 py);
		void penUp(u8 px, u8 py);
		void penMove(u8 px, u8 py);

		void setValue(s32 val);
		s32 getValue(void);
		
		// Callback registration
		void registerPostChangeCallback(void (*onPostChange_)(s32));
		void registerChangeCallback(void (*onChange_)(s32));
		
	private:
		void draw(void);
		
		void (*onPostChange)(s32);
		void (*onChange)(s32);
		
		s32 value;
		u8 lasty;
		bool btnstate;
		s32 min;
		s32 max;
		bool hex;
		bool is_8bit;
};

};

#endif
