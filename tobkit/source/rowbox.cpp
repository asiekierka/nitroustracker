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

#include "tobkit/rowbox.h"
#include "tobkit/gui.h"
#include "tobkit/label.h"
#include <initializer_list>

using namespace tobkit;

/* ===================== PUBLIC ===================== */

RowBox::RowBox(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen,
               bool _visible)
    : ContainerWidget(_x, _y, _width, _height, _screen, _visible)
{
	row_y_offsets.push_back(y);
}

void RowBox::addRow(const char *caption,
                    std::initializer_list<Widget *> widgets)
{
	int x_padding = 3;
	int y_padding = 2;
	int label_height = 10;

	u16 w_width = 0;
	for (Widget *w : widgets) {
		w_width = std::max(w_width, w->getWidth());
	}

	int rowStartY = row_y_offsets.back();
	int rowY = rowStartY + y_padding;
	for (Widget *w : widgets) {
		gui.registerWidget(w, 0, SUB_SCREEN);
		w->setPos(x + width - w_width - x_padding, rowY);
		rowY += w->getHeight() + y_padding;
	}
	row_y_offsets.push_back(rowY);

	Label *label = new Label(
	    x + x_padding, rowStartY + ((rowY - rowStartY - label_height) >> 1),
	    width - w_width - x_padding * 2, label_height, screen, false);
	label->setCaption(caption);
	gui.registerWidget(label, 0, SUB_SCREEN);

	if (theme)
		setTheme(theme, bgcolor);
}

// Drawing request
void RowBox::pleaseDraw(void)
{
	draw();
	reveal();
}

GUI *RowBox::currentGui(void)
{
	return &gui;
}

void RowBox::setTheme(Theme *theme_, u16 bgcolor_)
{
	theme = theme_;
	bgcolor = bgcolor_;

	col_even = theme->col_list_2;
	col_odd = interpolateColor(theme->col_list_1, theme->col_list_2, 2048);

	gui.setTheme(theme, theme->col_light_bg);
	int i = 0;
	for (Widget *w : gui.getWidgets()) {
		while (row_y_offsets.at(i + 1) <= w->getY()) {
			i++;
		}

		w->setTheme(theme, (i & 1) ? col_odd : col_even);
	}
}

/* ===================== PRIVATE ===================== */

void RowBox::draw(void)
{
	// Draw rows
	for (int i = 0; i < row_y_offsets.size() - 1; i++) {
		drawFullBox(0, row_y_offsets.at(i) - y, width,
		            row_y_offsets.at(i + 1) - row_y_offsets.at(i),
		            (i & 1) ? col_odd : col_even);
	}

	// Draw gui
	gui.draw();
}
