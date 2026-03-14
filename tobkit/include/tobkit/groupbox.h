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

#ifndef _GROUPBOX_H_
#define _GROUPBOX_H_

#include "widget.h"

namespace tobkit {

class GroupBox: public Widget
{
	public:
		GroupBox(u8 _x, u8 _y, u8 _width, u8 _height, Screen *_screen, bool _visible=true);
		~GroupBox();
		
		void pleaseDraw(void);
		
		void setText(const char *text);
		
	private:
		char *text;
		
		void draw(void);
};

};

#endif
