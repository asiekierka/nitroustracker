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
#include "radiobutton.h"
#include "widget.h"

#include <map>
#include <string>
#include <vector>

#define THEMESELBOX_BUILTIN 0
#define THEMESELBOX_EXTERNAL 1
#if defined(NT_PLATFORM_NDS)
#define THEMESELBOX_DEFAULT_PATH_BUILTIN "nitro:/themes"
#elif defined(NT_PLATFORM_3DS)
#define THEMESELBOX_DEFAULT_PATH_BUILTIN "romfs:/themes"
#else
#define THEMESELBOX_DEFAULT_PATH_BUILTIN "/"
#endif

namespace tobkit
{

class ThemeSelectorBox : public Widget
{
public:
	ThemeSelectorBox(Screen *_screen, void (*_onSelect)(File),
	                 void (*_onOk)(void), void (*_onReset)(void),
	                 void (*_onCancel)(void), void (*_onTypeChange)(int));
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

	GUI gui;
	const char *title;
	RadioButton::RadioButtonGroup *rbglocation;
	RadioButton *rbexternal, *rbbuiltin;
	Button *buttonok, *buttoncancel, *buttonreset;
	bool changingtype;

private:
};

}; // namespace tobkit

#endif
