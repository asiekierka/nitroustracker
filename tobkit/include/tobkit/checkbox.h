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

#ifndef _CHECKBOX_H_
#define _CHECKBOX_H_

#include "widget.h"

namespace tobkit
{

class CheckBox : public Widget
{
public:
	CheckBox(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen,
	         bool _visible = true, bool checked = false, bool albino = false);
	~CheckBox();

	void setCaption(const char *_label);
	void setChecked(bool checked_);

	// Drawing request
	void pleaseDraw(void);

	// Event calls
	void penDown(u16 px, u16 py);

	void registerToggleCallback(void (*onToggle_)(bool));

private:
	char *label;
	bool checked;
	bool albino; // different color setting for dark bg

	void (*onToggle)(bool);

	void draw(void);
};

}; // namespace tobkit

#endif
