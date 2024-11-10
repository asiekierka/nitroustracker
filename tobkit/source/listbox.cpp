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

#include "tobkit/listbox.h"

#include <stdlib.h>
#include <stdio.h>

/* ===================== PUBLIC ===================== */

ListBox::ListBox(u8 _x, u8 _y, u8 _width, u8 _height, uint16 **_vram, u16 n_items,
	bool _show_numbers, bool _visible, bool _zero_offset)
	:Widget(_x, _y, _width, _height, _vram, _visible),
	buttonstate(0), activeelement(0), highlightedelement(-1), scrollpos(0),
	show_numbers(_show_numbers),
	zero_offset(_zero_offset)
{
	u16 i;
	for(i=0;i<n_items;++i) {
		elements.push_back("");
	}
	onChange = 0;
}


// Drawing request
void ListBox::pleaseDraw(void)
{
	calcScrollThingy();
	draw();
}


// Event calls
void ListBox::penDown(u8 px, u8 py)
{
	u8 relx = px-x, rely = py-y;
	if(relx<width-SCROLLBAR_WIDTH) {
		// Item select
		u16 rel_item_clicked;
		rel_item_clicked = rely / ROW_HEIGHT;
		u16 newactiveelement = rel_item_clicked + scrollpos;
		if((newactiveelement != activeelement)&&(newactiveelement<elements.size())) {
			activeelement = newactiveelement;
			if(onChange != 0) {
				onChange(activeelement);
			}
			
			draw();
		}
	} else {
		// Scroll
		if(py<y+SCROLLBUTTON_HEIGHT) {
			buttonstate=SCROLLUP;
			if(scrollpos>0) {
				scrollpos--;
				calcScrollThingy();
				draw();
			}
		} else if(py>y+height-SCROLLBUTTON_HEIGHT) {
			buttonstate=SCROLLDOWN;
			s16 max_scollpos = elements.size()-height/ROW_HEIGHT;
			if(max_scollpos<0) max_scollpos=0;
			if(scrollpos<max_scollpos) {
				scrollpos++;
				calcScrollThingy();
				draw();
			}
		} else if((py>y+SCROLLBUTTON_HEIGHT+scrollthingypos)&&(py<y+SCROLLBUTTON_HEIGHT+scrollthingypos+scrollthingyheight)) {
			// The scroll thingy
			buttonstate=SCROLLTHINGY;
			pen_y_on_scrollthingy = py - (y+SCROLLBUTTON_HEIGHT+scrollthingypos);
			draw();
		} else if((py>y+SCROLLBUTTON_HEIGHT)&&(py<y+SCROLLBUTTON_HEIGHT+scrollthingypos)) {
			// Above the scroll thingy
			u16 rows_displayed = (height-1)/ROW_HEIGHT;
			if(scrollpos>rows_displayed) {
				scrollpos -= rows_displayed;
			} else {
				scrollpos = 0;
			}
			calcScrollThingy();
			draw();
		} else if((py>y+SCROLLBUTTON_HEIGHT+scrollthingypos+scrollthingyheight)&&(py<y+height-SCROLLBUTTON_HEIGHT)) {
			// Below the scroll thingy
			u16 rows_displayed = (height-1)/ROW_HEIGHT;
			s16 max_scollpos = elements.size()-height/ROW_HEIGHT;
			if(scrollpos+rows_displayed<max_scollpos) {
				scrollpos += rows_displayed;
			} else {
				scrollpos = max_scollpos;
			}
			calcScrollThingy();
			draw();
		}
	}
	
}

void ListBox::penUp(u8 px, u8 py)
{
	u8 previousstate = buttonstate;
	buttonstate = 0;
	
	if(previousstate != 0) {
		draw();
	}
}

void ListBox::penMove(u8 px, u8 py)
{
	if(buttonstate==SCROLLTHINGY) {
		u16 oldscrollpos = scrollpos;

		// So this is what GUI code looks like .. Ugh!
		s16 new_pen_y_on_scrollthingy = py - (y+SCROLLBUTTON_HEIGHT+scrollthingypos);
		s16 new_scrollthingypos = scrollthingypos + new_pen_y_on_scrollthingy - pen_y_on_scrollthingy;
		if(new_scrollthingypos < 0) new_scrollthingypos = 0;
		scrollthingypos = new_scrollthingypos;
		u16 max_scrollthingypos = height-2*SCROLLBUTTON_HEIGHT-scrollthingyheight+2;
		if(scrollthingypos > max_scrollthingypos) {
			scrollthingypos = max_scrollthingypos;
		}
		
		// Calculate new scroll position
		s16 max_scrollpos = elements.size()-height/ROW_HEIGHT;
		if(max_scrollpos<0) max_scrollpos = 0;
		if(max_scrollthingypos == 0) {
			scrollpos = 0;
		} else {
			scrollpos = scrollthingypos * max_scrollpos / max_scrollthingypos;
		}
		
		//iprintf("scrollthingy: %u\nscroll: %u\nmaxscroll: %d\n",scrollthingypos,scrollpos,max_scrollpos);
		if(scrollpos != oldscrollpos) {
			// Snap scrollthingy position 
			scrollthingypos = scrollpos * max_scrollthingypos/ max_scrollpos;
			draw();
			oldscrollpos = scrollpos;
		}
	}
}

// Callback registration
void ListBox::registerChangeCallback(void (*onChange_)(u16)) {
	onChange = onChange_;
}

// Add / delete elements
void ListBox::add(const char *name) {
	
	elements.push_back(name);
	
	if(elements.size() <= (u16)((height-1)/ROW_HEIGHT)) {
		draw();
	}
}

// Always deletes selected item
void ListBox::del(void)
{
	if(elements.size()>0) {
		elements.erase(elements.begin()+activeelement);
		
		// Move activeelemnt up if the last active element was deleted
		if(activeelement > elements.size()-1) {
			activeelement = elements.size()-1;
		}
		
		// If an element from the bottom of the list was deleted so that
		// the last row is empty, scroll up if possible
		if(scrollpos>elements.size()-height/ROW_HEIGHT) {
			scrollpos = elements.size()-height/ROW_HEIGHT;
		}
		
		if(activeelement >= scrollpos) {
			draw();
		}
	}
}

// Inserts an element at position idx
void ListBox::ins(u16 idx, const char *name)
{
	elements.insert(elements.begin()+idx, name);
	
	draw();
}

void ListBox::set(u16 idx, const char *name)
{
	elements.at(idx) = name;
	
	if( (idx >= scrollpos) && (idx < scrollpos+(height-1)/ROW_HEIGHT) ) {
		draw();
	}
}

const char *ListBox::get(u16 idx) {
	return elements.at(idx).c_str();
}

u16 ListBox::getidx(void) {
	return activeelement;
}

void ListBox::clear(void)
{
	activeelement = 0;
	highlightedelement = -1;
	scrollpos = 0;
	elements.clear();
}

void ListBox::scrollTo(u16 idx)
{
	u16 oldscrollpos = scrollpos;

	// Scroll down if idx is under the viewport
	if(scrollpos+height/ROW_HEIGHT-1 < idx) {
		scrollpos = idx - height/ROW_HEIGHT + 1;
	}

	// Scroll up if idx is above the viewport
	if(scrollpos>idx) {
		scrollpos = idx;
	}

	if(oldscrollpos != scrollpos) {
		calcScrollThingy();
	}
}

void ListBox::highlight(s32 idx, bool scroll)
{
	highlightedelement = idx;

	if(highlightedelement >= 0 && scroll) {
		scrollTo(idx);
	}

	draw();
}

void ListBox::select(u16 idx, bool scroll)
{
	activeelement = idx;

	if(scroll) {
		scrollTo(idx);
	}

	calcScrollThingy();
	draw();
}

/* ===================== PRIVATE ===================== */

void ListBox::draw(void)
{
	if(!isExposed())
		return;

	u16 rows_displayed = (height-1)/ROW_HEIGHT;
	u16 i;
	// Horizontal Separator Lines
	for(i=0;i<rows_displayed-1;++i) {
		drawHLine(1,ROW_HEIGHT*(i+1),width-SCROLLBAR_WIDTH-1,theme->col_sepline);
	}
	
	// Fill rows
	for(i=0;i<rows_displayed;++i) {
		u16 col_bottom = theme->col_medium_bg;
		u16 col_top = theme->col_light_bg;
		if((scrollpos+i)==activeelement) {
			col_bottom = theme->col_list_highlight1;
			col_top = theme->col_list_highlight2;
		} else if((scrollpos+i)==highlightedelement) {
			col_bottom = theme->col_light_bg;
			col_top = theme->col_lighter_bg;
		}
		drawGradient(col_bottom, col_top, 1, ROW_HEIGHT*i+1, width-SCROLLBAR_WIDTH-1, ROW_HEIGHT-1);
	}

	// Vertical number separator line
	if(show_numbers) {
		drawVLine(COUNTER_WIDTH,1,height-2,theme->col_sepline);
	}
	
	// Scrollbar
	// Upper Button
	if(buttonstate==SCROLLUP) {
		drawGradient(theme->col_dark_ctrl, theme->col_light_ctrl, width-SCROLLBAR_WIDTH+1, 1, 8, 8);
	} else {
		drawGradient(theme->col_light_ctrl, theme->col_dark_ctrl, width-SCROLLBAR_WIDTH+1, 1, 8, 8);
	}
	
	// This draws the up-arrow
	s8 j, p;
	for(j=0;j<3;j++) {
		for(p=-j;p<=j;++p) {
			drawPixel(width-SCROLLBAR_WIDTH+4+p, j+3, RGB15(0,0,0) | BIT(15));
		}
	}
	
	drawBox(width-SCROLLBAR_WIDTH, 0, 9, 9);
	
	// Lower Button
	if(buttonstate==SCROLLDOWN) {
		drawGradient(theme->col_dark_ctrl, theme->col_light_ctrl, width-SCROLLBAR_WIDTH+1, height-9, 8, 8);
	} else {
		drawGradient(theme->col_light_ctrl, theme->col_dark_ctrl, width-SCROLLBAR_WIDTH+1, height-9, 8, 8);
	}
	
	// This draws the down-arrow
	for(j=2;j>=0;j--) {
		for(p=-j;p<=j;++p) {
			drawPixel(width-SCROLLBAR_WIDTH+4+p, -j+height-4, RGB15(0,0,0) | BIT(15));
		}
	}
	
	drawBox(width-SCROLLBAR_WIDTH, height-9, 9, 9);
	
	drawBox(width-SCROLLBAR_WIDTH, 0, SCROLLBAR_WIDTH,height);
	
	// Clear Scrollbar
	drawGradient(theme->col_medium_bg, theme->col_light_bg, width-SCROLLBAR_WIDTH+1, SCROLLBUTTON_HEIGHT, SCROLLBAR_WIDTH-2, height-2*SCROLLBUTTON_HEIGHT);

	if (height >= 2*SCROLLBUTTON_HEIGHT+scrollthingyheight) {
	
		// The scroll thingy
		if(buttonstate==SCROLLTHINGY) {
			drawFullBox(width-SCROLLBAR_WIDTH+1, SCROLLBUTTON_HEIGHT-1+scrollthingypos, SCROLLBAR_WIDTH-2, scrollthingyheight, theme->col_list_highlight2);
		} else {
			drawFullBox(width-SCROLLBAR_WIDTH+1, SCROLLBUTTON_HEIGHT-1+scrollthingypos, SCROLLBAR_WIDTH-2, scrollthingyheight, theme->col_list_highlight1);
		}
		drawBox(width-SCROLLBAR_WIDTH, SCROLLBUTTON_HEIGHT+scrollthingypos-1, SCROLLBAR_WIDTH, scrollthingyheight);
	}
	
	// Numbers (if enabled)
	u16 contentoffset;
	if(show_numbers) {
		char numberstr[4 + 1];
		//iprintf("%u %u\n",scrollpos,elements.size());
		u16 offset;
		if(zero_offset==true) {
			offset=0;
		} else {
			offset=1;
		}
		for(i=0;(i<height/ROW_HEIGHT)&&(scrollpos+i<elements.size());++i) {
			sniprintf(numberstr, sizeof(numberstr), "%2x", (u16) (scrollpos+i+offset));
			drawString(numberstr, 2, ROW_HEIGHT*i+2);
		}
		
		contentoffset = COUNTER_WIDTH;
	} else {
		contentoffset = 0;
	}
	
	// Content
	for(i=0;(i<height/ROW_HEIGHT)&&(scrollpos+i<elements.size());++i) {
		drawString(elements.at(scrollpos+i).c_str(), contentoffset+2, ROW_HEIGHT*i+2,
			width-contentoffset-2-SCROLLBAR_WIDTH-2,
			RGB15(0,0,0)|BIT(15),
			height-(ROW_HEIGHT*i+4)
		);
	}
	
	drawBorder();
}


// Calculate height and position of the scroll thingy
void ListBox::calcScrollThingy(void)
{
	if(elements.size() < height/ROW_HEIGHT)
		scrollthingyheight = height-2*SCROLLBUTTON_HEIGHT+2;
	else
		scrollthingyheight = (height-2*SCROLLBUTTON_HEIGHT+2) * (height/ROW_HEIGHT)/elements.size();
	
	if(scrollthingyheight > height-2*SCROLLBUTTON_HEIGHT+2)
		scrollthingyheight = height-2*SCROLLBUTTON_HEIGHT+2;
	
	if(scrollthingyheight < MIN_SCROLLTHINGYHEIGHT)
		scrollthingyheight = MIN_SCROLLTHINGYHEIGHT;
	
	s16 max_scrollpos = elements.size()-height/ROW_HEIGHT;
	if(max_scrollpos < 0)
		max_scrollpos = 0;
	
	u16 max_scrollthingypos = height-2*SCROLLBUTTON_HEIGHT-scrollthingyheight+2;
	if(max_scrollpos == 0)
		scrollthingypos = 0;
	else
		scrollthingypos = max_scrollthingypos * scrollpos/max_scrollpos;
}
