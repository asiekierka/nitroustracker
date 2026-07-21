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

#ifndef TWOTABBOX_H
#define TWOTABBOX_H

#include "gui.h"
#include "widget.h"

#include <vector>

namespace tobkit
{

class TwoTabBox : public Widget
{
public:
	TwoTabBox(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen,
	          u8 icon_size, u8 sub_icon_size, bool _visible = true);

	u8 addTab(const u8 *icon);
	u8 addSubTab(const u8 *icon);

	// Adds a widget and specifies which button it listens to
	// Touches on widget's area are redirected to the widget
	void registerWidget(Widget *w, u16 listeningButtons, u8 tabidx,
	                    u8 screen = SUB_SCREEN);

	// Event calls
	void penDown(u16 px, u16 py);
	void penUp(u16 px, u16 py);
	void penMove(u16 px, u16 py);
	void buttonPress(u16 buttons);

	// Callback registration
	void registerTabChangeCallback(void (*onTabChange_)(u8 tab, u8 subtab));

	// Drawing request
	void pleaseDraw(void);

	void show(void);
	void hide(void);
	void occlude(void);
	void reveal(void);

	void setTheme(Theme *theme_, u16 bgcolor_);
	void setIcon(u8 tabidx, const u8 *icon);

private:
	u8 firstSubtab(u8 tabidx);
	int anyTabToGuiIdx(u8 tabidx);
	void draw(void);
	void drawIcon(u8 guiidx);
	void occludeOthers(void);

	u8 icon_size;
	u8 sub_icon_size;
	u8 currentgui;
	u8 currenttab;
	u8 currentsubtab;
	std::vector<u8> subtab_count_per_tab;
	std::vector<u8> subtab_to_tab_idx;
	std::vector<u8> subtab_to_gui_idx;
	std::vector<u8> tab_to_gui_idx;
	std::vector<const u8 *> tab_icons;
	std::vector<const u8 *> subtab_icons;
	std::vector<GUI> guis;

	void (*onTabChange)(u8 tab, u8 subtab);
};

}; // namespace tobkit

#endif
