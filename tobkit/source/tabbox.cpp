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

#include "tobkit/tabbox.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

TabBox::TabBox(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen,
               u8 _orientation, u8 _icon_size, bool _visible)
    : Widget(_x, _y, _width, _height, _screen, _visible),
      orientation(_orientation), icon_size(_icon_size), currentgui(0)
{
	onTabChange = 0;
}

u8 TabBox::findGuiIdx(u8 tabidx)
{
	for (u8 i = 0; i < tab_idx_map.size(); i++)
		if (tab_idx_map.at(i) == tabidx)
			return i;
	return 255;
}

void TabBox::addTab(const u8 *icon, u8 tabidx)
{
	tab_idx_map.push_back(tabidx);
	icons.push_back(icon);
	GUI gui;
	gui.setTheme(theme, theme->col_light_bg);
	guis.push_back(gui);
	highlighted_tabs.push_back(false);
}

// Adds a widget and specifies which button it listens to
// Touches on widget's area are redirected to the widget
void TabBox::registerWidget(Widget *w, u16 listeningButtons, u8 tabidx,
                            u8 screen)
{
	u8 guiidx = findGuiIdx(tabidx);
	if (guiidx >= guis.size())
		return;

	guis.at(guiidx).registerWidget(w, listeningButtons, screen);

	if (guiidx != currentgui) {
		w->occlude();
	} else {
		if (isExposed())
			w->reveal();
	}
}

// Event calls
void TabBox::penDown(u16 px, u16 py)
{
	u8 size_border = icon_size + 2;
	u8 size_full = size_border;

	bool on_tab = false;
	u8 gui_hit = 255;

	if (orientation == TABBOX_ORIENTATION_TOP) {
		on_tab = (py - y) < size_full;
		gui_hit = (px - x - 3) / size_full;
	} else if (orientation == TABBOX_ORIENTATION_LEFT) {
		on_tab = (px - x) < size_full;
		gui_hit = (py - y - 3) / size_full;
	}

	// If it's on the tabs
	if (on_tab) {
		if (gui_hit < guis.size()) {
			currentgui = gui_hit;
			draw();
			updateVisibilities();
			if (onTabChange != 0) {
				onTabChange(tab_idx_map.at(gui_hit));
			}
		}
	} else {
		// If its in the box
		guis.at(currentgui).penDown(px, py);
	}
}

void TabBox::penUp(u16 px, u16 py)
{
	guis.at(currentgui).penUp(px, py);
}

void TabBox::penMove(u16 px, u16 py)
{
	// If it's on the tabs

	// If its in the box
	guis.at(currentgui).penMove(px, py);
}

void TabBox::buttonPress(u16 buttons)
{
	guis.at(currentgui).buttonPress(buttons);
}

// Callback registration
void TabBox::registerTabChangeCallback(void (*onTabChange_)(u8 tab))
{
	onTabChange = onTabChange_;
}

// Drawing request
void TabBox::pleaseDraw(void)
{
	draw();
	updateVisibilities();
}

void TabBox::show(void)
{
	Widget::show();
	guis.at(currentgui).showAll();
}

void TabBox::hide(void)
{
	Widget::hide();
	guis.at(currentgui).hideAll();
}

void TabBox::occlude(void)
{
	Widget::occlude();
	guis.at(currentgui).occludeAll();
}

void TabBox::reveal(void)
{
	Widget::reveal();
	guis.at(currentgui).revealAll();
}

int TabBox::getCount(void)
{
	return guis.size();
}

void TabBox::setTheme(Theme *theme_, u16 bgcolor_)
{
	theme = theme_;
	bgcolor = bgcolor_;

	for (u8 gui_id = 0; gui_id < guis.size(); ++gui_id) {
		guis.at(gui_id).setTheme(theme_, theme->col_light_bg);
	}
}

void TabBox::setIcon(u8 guiidx, const u8 *icon)
{
	icons.at(guiidx) = icon;
	drawIcon(guiidx);
}

/* ===================== PRIVATE ===================== */

void TabBox::drawIcon(u8 guiidx)
{
	u8 size_border = icon_size + 2;
	u8 size_full = size_border;
	u8 offset = guiidx == currentgui ? 0 : 3;
	u16 black = theme->col_tab_outline;
	u16 col = highlighted_tabs.at(guiidx) ? theme->col_tab_outline
	                                      : theme->col_tab_outline;

	if (orientation == TABBOX_ORIENTATION_TOP) {
		bool selected = guiidx == currentgui;
		u8 offset = selected ? 0 : 3;

		drawFullBox(3 + size_full * guiidx, 1 + offset, size_border,
		            size_border - offset,
		            selected ? theme->col_selected_tab
		                     : theme->col_unselected_tab);
		drawVLine(2 + size_full * guiidx, 1 + offset, size_border - offset,
		          col);
		drawHLine(3 + size_full * guiidx, 0 + offset, size_border - 1, col);
		drawVLine(2 + size_full * (guiidx + 1), 1 + offset,
		          size_border - offset, col);
		if (!selected)
			drawPixel(3 + size_full * guiidx + size_border - 1, 0 + offset,
			          theme->col_bg);
		drawMonochromeIcon(4 + size_full * guiidx, 2 + offset, icon_size,
		                   icon_size - offset, icons.at(guiidx),
		                   selected ? theme->col_tab_icon_highlight
		                            : theme->col_tab_icon);
	} else {
		bool selected = guiidx == currentgui;
		u8 offset = selected ? 0 : 3;

		drawFullBox(1 + offset, 2 + size_full * guiidx, size_border - offset,
		            size_border,
		            selected ? theme->col_selected_tab
		                     : theme->col_unselected_tab);
		drawHLine(1 + offset, 2 + size_full * guiidx, size_border - offset,
		          col);
		drawVLine(0 + offset, 3 + size_full * guiidx, size_border - 1, col);
		drawHLine(1 + offset, 2 + size_full * (guiidx + 1),
		          size_border - offset - 1, col);
		if (!selected)
			drawPixel(offset, 2 + size_full * guiidx + size_border,
			          theme->col_light_bg);
		drawMonochromeIconOffset(
		    2 + offset, 4 + size_full * guiidx, icon_size - offset, icon_size,
		    0, 0, icon_size, icon_size, icons.at(guiidx),
		    selected ? theme->col_tab_icon_highlight : theme->col_tab_icon);
	}
}

void TabBox::draw(void)
{
	u8 size_border = icon_size + 2;
	u8 size_full = size_border;

	if (orientation == TABBOX_ORIENTATION_TOP) {
		// Draw box
		drawFullBox(1, size_full + 1, width - 2, height - (size_border + 2),
		            theme->col_light_bg);
		drawBox(0, size_border, width, height - size_border,
		        theme->col_tab_outline);

		// Draw tabs
		drawFullBox(0, 0, 3 + size_full * guis.size(), 3, theme->col_bg);

		for (u8 guiidx = 0; guiidx < guis.size(); ++guiidx) {

			drawIcon(guiidx);
		}
	} else {
		// Draw box
		drawFullBox(14, 1, width - 15, height - 2, theme->col_light_bg);
		drawBox(13, 0, width - 13, height, theme->col_tab_outline);

		// Draw tabs
		drawFullBox(0, 0, 3, 3 + 13 * guis.size(), theme->col_light_bg);

		for (u8 guiidx = 0; guiidx < guis.size(); ++guiidx) {

			drawIcon(guiidx);
		}
	}

	// Draw gui
	guis.at(currentgui).draw();
}

void TabBox::updateVisibilities(void)
{
	for (u8 gui_id = 0; gui_id < guis.size(); ++gui_id) {
		if (gui_id != currentgui) {
			guis.at(gui_id).occludeAll();
		}
	}
	guis.at(currentgui).revealAll();
}
