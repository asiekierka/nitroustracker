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

#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include "widget.h"
#include <stdio.h>
#include <vector>

namespace tobkit {

class RadioButton: public Widget {
	public:
		class RadioButtonGroup {
			public:
				RadioButtonGroup();
				
				void add(RadioButton *rb);
				void pushed(RadioButton *rb);
				void setActive(u8 idx);
				void registerChangeCallback(void (*onChange_)(u8));
	
			private:
				std::vector<RadioButton*> rbvec;
				void (*onChange)(u8);
		};
		
		RadioButton(u8 _x, u8 _y, u8 _width, u8 _height, Screen *_screen,
				RadioButtonGroup *_rbg, bool _visible=true);
		
		// Drawing request
		void pleaseDraw(void);
		
		// Event calls
		void penDown(u8 px, u8 py);
		
		void setCaption(const char *caption);
		void setActive(bool _active);
		bool getActive(void);
		
	private:
		void draw(void);
	
		RadioButtonGroup *rbg;
		bool active;
		const char *label;
};

};

#endif
