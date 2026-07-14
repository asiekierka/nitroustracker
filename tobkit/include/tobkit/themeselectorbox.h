/*====================================================================
Copyright 2025 R Ferreira

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

#ifndef THEMESELECTORBOX_H
#define THEMESELECTORBOX_H

#include "button.h"
#include "fileselector.h"
#include "gui.h"
#include "listbox.h"
#include "widget.h"

#include <map>
#include <string>
#include <vector>

namespace tobkit
{

class ThemeSelectorBox : public Widget
{
public:
	ThemeSelectorBox(Screen *_screen, void (*_onSelect)(File),
	                 void (*_onOk)(void), void (*_onReset)(void),
	                 void (*_onCancel)(void));
	~ThemeSelectorBox(void);

	// Event calls
	void penDown(u16 px, u16 py);
	void penMove(u16 px, u16 py);
	void penUp(u16 px, u16 py);

	// Drawing request
	void pleaseDraw(void);

	void show(void);
	void reveal(void);
	void setTheme(Theme *theme_, u16 bgcolor_);

	void setDir(std::string dir);
	std::string getDir(void);

	FileSelector *filesel;

protected:
	void draw(void);

	void (*onSelect)(File);
	void (*onOk)(void);
	void (*onReset)(void);
	void (*onCancel)(void);

	GUI gui;
	const char *title;
	Button *buttonok, *buttoncancel, *buttonreset;

private:
};

}; // namespace tobkit

#endif
