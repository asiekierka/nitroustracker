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

#ifndef LISTBOX_H
#define LISTBOX_H

#include "widget.h"
#include <string>
#include <vector>

namespace tobkit
{

#define ROW_HEIGHT 11
#define COUNTER_WIDTH 17
#define SCROLLBAR_WIDTH 9
#define SCROLLBUTTON_HEIGHT 9

#define SCROLLUP 1
#define SCROLLDOWN 2
#define SCROLLTHINGY 3

#define MIN_SCROLLTHINGYHEIGHT 15

class ListBox : public Widget
{
public:
	ListBox(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen,
	        u16 n_items, bool _show_numbers, bool _visible = true,
	        bool _zero_offset = true);

	// Drawing request
	void pleaseDraw(void);

	// Event calls
	void penDown(u16 px, u16 py);
	void penUp(u16 px, u16 py);
	void penMove(u16 px, u16 py);

	// Callback registration
	void registerChangeCallback(void (*onChange_)(u16));

	// Add / delete /set /get elements
	void add(const char *name);
	void del(void); // Always deletes selected item
	void ins(u16 idx, const char *name);
	void set(u16 idx, const char *name); // Inserts an element at position idx
	const char *get(u16 idx);
	u16 getidx(void); // get index of selected element
	void clear(int n_items = 0);
	void select(u16 idx, bool scroll = true); // set selected element
	void highlight(s32 idx,
	               bool scroll = true); // set highlighted element, <0 disables

protected:
	void draw(void);
	void scrollTo(u16 idx);
	// Calculate height and position of the scroll thingy
	void calcScrollThingy(void);
	// Ensure active element/scroll positions are not out of bounds
	void clipPositions(void);

	void (*onChange)(u16);

	u8 buttonstate;
	u16 activeelement;
	s32 highlightedelement;
	u16 scrollpos;
	u16 scrollthingypos, scrollthingyheight, pen_y_on_scrollthingy;
	bool show_numbers, zero_offset; // zero_offset: If false, offset=1
	std::vector<std::string> elements;
};

}; // namespace tobkit

#endif
