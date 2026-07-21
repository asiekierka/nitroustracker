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

#include "tobkit/twotabbox.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

TwoTabBox::TwoTabBox(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen,
                     u8 _icon_size, u8 _sub_icon_size, bool _visible)
    : Widget(_x, _y, _width, _height, _screen, _visible), icon_size(_icon_size),
      sub_icon_size(_sub_icon_size), currentgui(0)
{
	onTabChange = 0;
}

u8 TwoTabBox::addTab(const u8 *icon)
{
	u8 idx = subtab_count_per_tab.size();
	subtab_count_per_tab.push_back(0);
	tab_to_gui_idx.push_back(guis.size());
	tab_icons.push_back(icon);
	GUI gui;
	gui.setTheme(theme, theme->col_light_bg);
	guis.push_back(gui);
	return idx;
}

u8 TwoTabBox::addSubTab(const u8 *icon)
{
	u8 tab_idx = subtab_count_per_tab.size() - 1;
	subtab_count_per_tab[tab_idx]++;
	u8 idx = subtab_to_tab_idx.size();
	subtab_to_tab_idx.push_back(tab_idx);
	subtab_icons.push_back(icon);
	if (subtab_count_per_tab[tab_idx] > 1) {
		GUI gui;
		gui.setTheme(theme, theme->col_light_bg);
		guis.push_back(gui);
	}
	subtab_to_gui_idx.push_back(guis.size() - 1);
	return idx | 0x80;
}

int TwoTabBox::anyTabToGuiIdx(u8 tabidx)
{
	u8 count = tabidx & 0x7F;
	return tabidx >= 0x80
	           ? (count < subtab_to_gui_idx.size() ? subtab_to_gui_idx[count]
	                                               : -1)
	           : (count < tab_to_gui_idx.size() ? tab_to_gui_idx[tabidx] : -1);
}

// Adds a widget and specifies which button it listens to
// Touches on widget's area are redirected to the widget
void TwoTabBox::registerWidget(Widget *w, u16 listeningButtons, u8 tabidx,
                               u8 screen)
{
	int minGui = anyTabToGuiIdx(tabidx);
	int maxGui = minGui;
	if (tabidx < 0x80 && subtab_count_per_tab[tabidx] > 1) {
		maxGui += subtab_count_per_tab[tabidx] - 1;
	}

	bool visible = false;
	for (int i = minGui; i <= maxGui; i++) {
		guis.at(i).registerWidget(w, listeningButtons, screen);
		visible |= i == currentgui;
	}

	if (visible) {
		w->occlude();
	} else {
		if (isExposed())
			w->reveal();
	}
}

u8 TwoTabBox::firstSubtab(u8 tabidx)
{
	u8 tab = 0x80;
	for (int i = 0; i < tabidx; i++)
		tab += subtab_count_per_tab[i];
	return tab;
}

// Event calls
void TwoTabBox::penDown(u16 px, u16 py)
{
	u8 size_top = icon_size + 2;
	u8 size_bottom = sub_icon_size + 2;

	int tab_hit = -1;
	if ((py - y) < size_top) {
		tab_hit = (px - x - 3) / size_top;
	} else if ((py - y) >= (height - size_bottom) &&
	           subtab_count_per_tab.at(currenttab) > 0) {
		tab_hit = ((px - x - 3) / size_bottom);
		if (tab_hit < subtab_count_per_tab.at(currenttab)) {
			tab_hit += firstSubtab(currenttab);
		} else {
			tab_hit = -1;
		}
	}

	// If it's on the tabs
	if (tab_hit >= 0) {
		int gui_idx = anyTabToGuiIdx(tab_hit);
		if (gui_idx >= 0) {
			currentgui = gui_idx;
			if (tab_hit & 0x80) {
				currentsubtab = tab_hit;
			} else {
				currenttab = tab_hit;
				currentsubtab = firstSubtab(tab_hit);
			}
			pleaseDraw();
			if (onTabChange != 0) {
				onTabChange(currenttab, currentsubtab);
			}
		}
	} else {
		// If its in the box
		guis.at(currentgui).penDown(px, py);
	}
}

void TwoTabBox::penUp(u16 px, u16 py)
{
	guis.at(currentgui).penUp(px, py);
}

void TwoTabBox::penMove(u16 px, u16 py)
{
	// If it's on the tabs

	// If its in the box
	guis.at(currentgui).penMove(px, py);
}

void TwoTabBox::buttonPress(u16 buttons)
{
	guis.at(currentgui).buttonPress(buttons);
}

// Callback registration
void TwoTabBox::registerTabChangeCallback(void (*onTabChange_)(u8 tab,
                                                               u8 subtab))
{
	onTabChange = onTabChange_;
}

// Drawing request
void TwoTabBox::pleaseDraw(void)
{
	occludeOthers();
	draw();
	reveal();
}

void TwoTabBox::show(void)
{
	Widget::show();
	guis.at(currentgui).showAll();
}

void TwoTabBox::hide(void)
{
	Widget::hide();
	guis.at(currentgui).hideAll();
}

void TwoTabBox::occlude(void)
{
	Widget::occlude();
	guis.at(currentgui).occludeAll();
}

void TwoTabBox::reveal(void)
{
	Widget::reveal();
	guis.at(currentgui).revealAll();
}

void TwoTabBox::setTheme(Theme *theme_, u16 bgcolor_)
{
	theme = theme_;
	bgcolor = bgcolor_;

	for (u8 gui_id = 0; gui_id < guis.size(); ++gui_id) {
		guis.at(gui_id).setTheme(theme_, theme->col_light_bg);
	}
}

void TwoTabBox::setIcon(u8 tabidx, const u8 *icon)
{
	if (tabidx & 0x80) {
		subtab_icons.at(tabidx & 0x7F) = icon;
	} else {
		tab_icons.at(tabidx) = icon;
	}
	drawIcon(tabidx);
}

/* ===================== PRIVATE ===================== */

void TwoTabBox::drawIcon(u8 tabidx)
{
	const u8 *icon;
	bool subtab = false;
	bool selected = false;
	if (tabidx >= 0x80) {
		selected = tabidx == currentsubtab;
		subtab = true;
		tabidx &= 0x7F;
		icon = subtab_icons.at(tabidx);
	} else {
		selected = tabidx == currenttab;
		icon = tab_icons.at(tabidx);
	}

	u8 size_icon = (subtab ? sub_icon_size : icon_size);
	u8 size_border = size_icon + 2;
	u8 size_full = size_border;
	int bottom = height - size_full;

	u8 offset = selected ? 0 : 3;
	u16 col = theme->col_tab_outline;

	drawFullBox(3 + size_full * tabidx, subtab ? bottom : 1 + offset,
	            size_border, size_border - offset,
	            selected ? theme->col_selected_tab : theme->col_unselected_tab);
	drawVLine(2 + size_full * tabidx, subtab ? bottom : 1 + offset,
	          size_border - offset, col);
	drawHLine(3 + size_full * tabidx, subtab ? height - 1 - offset : 0 + offset,
	          size_border - 1, col);
	drawVLine(2 + size_full * (tabidx + 1), subtab ? bottom : 1 + offset,
	          size_border - offset, col);
	if (!selected)
		drawPixel(3 + size_full * tabidx + size_border - 1,
		          subtab ? height - 1 - offset : 0 + offset, theme->col_bg);
	drawMonochromeIconOffset(
	    4 + size_full * tabidx, subtab ? bottom : 2 + offset, size_icon,
	    size_icon - offset, 0, subtab ? offset : 0, size_icon,
	    size_icon - offset, icon,
	    selected ? theme->col_tab_icon_highlight : theme->col_tab_icon);
}

void TwoTabBox::draw(void)
{
	bool has_subtabs = subtab_count_per_tab.at(currenttab) > 0;
	u8 size_top = icon_size + 2;
	u8 size_bottom = has_subtabs ? (sub_icon_size + 2) : 0;
	int inner_height = height - size_top - size_bottom;

	// Draw box
	drawFullBox(1, size_top + 1, width - 2, inner_height - 2,
	            theme->col_light_bg);
	drawBox(0, size_top, width, inner_height, theme->col_tab_outline);

	// Draw tabs
	drawFullBox(0, 0, 3 + size_top * subtab_count_per_tab.size(), 3,
	            theme->col_bg);
	for (u8 tabidx = 0; tabidx < subtab_count_per_tab.size(); ++tabidx) {
		drawIcon(tabidx);
	}
	if (has_subtabs) {
		drawFullBox(0, height - size_bottom, width, size_bottom, theme->col_bg);
		u8 offset = firstSubtab(currenttab);
		for (u8 tabidx = 0; tabidx < subtab_count_per_tab.at(currenttab);
		     ++tabidx) {
			drawIcon(offset + tabidx);
		}
	}

	// Draw gui
	guis.at(currentgui).draw();
}

void TwoTabBox::occludeOthers(void)
{
	for (u8 gui_id = 0; gui_id < guis.size(); ++gui_id) {
		if (gui_id != currentgui) {
			guis.at(gui_id).occludeAll();
		}
	}
}
