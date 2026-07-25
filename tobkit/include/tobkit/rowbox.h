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

#ifndef ROWBOX_H
#define ROWBOX_H

#include "containerwidget.h"
#include "gui.h"

#include <vector>

namespace tobkit
{

class RowBox : public ContainerWidget
{
public:
	RowBox(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen,
	       bool _visible = true);

	// Drawing request
	void pleaseDraw(void);

	void addRow(const char *caption, std::initializer_list<Widget *> widgets);
	void setTheme(Theme *theme_, u16 bgcolor_);

protected:
	GUI *currentGui(void) override;

private:
	std::vector<u16> row_y_offsets;

	void draw(void);

	GUI gui;
	tobkit_pixel_t col_odd;
	tobkit_pixel_t col_even;
};

}; // namespace tobkit

#endif
