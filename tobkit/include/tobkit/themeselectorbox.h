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

#ifndef THEMESELECTORBOX_H
#define THEMESELECTORBOX_H

#include "widget.h"
#include "gui.h"
#include "listbox.h"
#include "fileselector.h"
#include "button.h"
#include <nds.h>

#include <vector>
#include <map>
#include <string>

namespace tobkit {

class ThemeSelectorBox: public Widget {
	public:
		ThemeSelectorBox(u16 **_vram, void (*_onSelect)(File), void (*_onOk)(void), void (*_onReset)(void), void (*_onCancel)(void));
		~ThemeSelectorBox(void);

		// Event calls
		void penDown(u8 px, u8 py);
		void penMove(u8 px, u8 py);
		void penUp(u8 px, u8 py);
	
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

};

#endif
