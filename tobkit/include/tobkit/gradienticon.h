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

#ifndef _GRADIENTICON_H_
#define _GRADIENTICON_H_

#include "widget.h"

namespace tobkit {

/**
 * @brief 2bpp gradient icon pixmap.
 */
class GradientIcon: public Widget {
	public:
		GradientIcon(u8 _x, u8 _y, u8 _width, u8 _height, const u32* _image, u16 **_vram, bool _visible=true);
	
		~GradientIcon();
		
		// Callback registration
		void registerPushCallback(void (*onPush_)(void));	
		
		// Event calls
		void penDown(u8 x, u8 y);
		
		// Drawing request
		void pleaseDraw(void);
		
	private:
		void draw(void);
		
		void (*onPush)(void);
		const u32 *image;
};

};

#endif
