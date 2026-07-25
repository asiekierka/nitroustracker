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

#ifndef CONTAINERWIDGET_H
#define CONTAINERWIDGET_H

#include "gui.h"
#include "widget.h"

#include <vector>

namespace tobkit
{

class ContainerWidget : public Widget
{
public:
	ContainerWidget(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen,
	                bool _visible = true);

	// Event calls
	virtual void penDown(u16 px, u16 py);
	virtual void penUp(u16 px, u16 py);
	virtual void penMove(u16 px, u16 py);
	virtual void buttonPress(u16 buttons);

	// Drawing helpers
	void show(void);
	void hide(void);
	void occlude(void);
	void reveal(void);

protected:
	virtual GUI *currentGui(void) = 0;
	virtual void draw(void);
};

}; // namespace tobkit

#endif
