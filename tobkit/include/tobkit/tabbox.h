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

#ifndef TABBOX_H
#define TABBOX_H

#include "gui.h"
#include "widget.h"

#include <vector>

namespace tobkit {

static constexpr u32 TABBOX_ORIENTATION_TOP = 0;
static constexpr u32 TABBOX_ORIENTATION_LEFT = 1;

class TabBox: public Widget {
	public:
		TabBox(u8 _x, u8 _y, u8 _width, u8 _height, Screen *_screen, u8 orientation, u8 icon_size, bool _visible=true);
		
		void addTab(const u8 *icon, u8 tabidx);
		
		// Adds a widget and specifies which button it listens to
		// Touches on widget's area are redirected to the widget
		void registerWidget(Widget *w, u16 listeningButtons, u8 tabidx, u8 screen=SUB_SCREEN);
		
		// Event calls
		void penDown(u8 px, u8 py);
		void penUp(u8 px, u8 py);
		void penMove(u8 px, u8 py);
		void buttonPress(u16 buttons);
		
		// Callback registration
		void registerTabChangeCallback(void (*onTabChange_)(u8 tab));
		
		// Drawing request
		void pleaseDraw(void);
		
		void show(void);
		void hide(void);
		void occlude(void);
		void reveal(void);

		int getCount(void);
		void setTheme(Theme *theme_, u16 bgcolor_);
		void setIcon(u8 guiidx, const u8 *icon);
	private:
		u8 findGuiIdx(u8 tabidx);
		void draw(void);
		void drawIcon(u8 guiidx);
		void updateVisibilities(void);
	
		u8 orientation;
		u8 icon_size;
		u8 currentgui;
		std::vector<u8> tab_idx_map;
		std::vector<const u8*> icons;
		std::vector<bool> highlighted_tabs;
		std::vector<GUI> guis;
	
		void (*onTabChange)(u8 tab);
};

};

#endif
