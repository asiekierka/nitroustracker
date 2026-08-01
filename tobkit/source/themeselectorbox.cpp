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

#include <limits.h>
#include <unistd.h>

#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "tobkit/themeselectorbox.h"

#define THEMESELBOX_WIDTH 180
#define THEMESELBOX_HEIGHT 144

using namespace tobkit;

/* ===================== PUBLIC ===================== */

ThemeSelectorBox::ThemeSelectorBox(Screen *_screen, void (*_onSelect)(File),
                                   void (*_onOk)(void), void (*_onReset)(void),
                                   void (*_onCancel)(void), void (*_onTypeChange)(int))
    : Widget((_screen->getWidth() - THEMESELBOX_WIDTH) / 2,
#if defined(NT_PLATFORM_NDS)
             153  - THEMESELBOX_HEIGHT,
#else
			 (_screen->getHeight() - THEMESELBOX_HEIGHT) / 2,
#endif
             THEMESELBOX_WIDTH,
             THEMESELBOX_HEIGHT, _screen),
    changingtype(false)
{
	title = "choose a theme";

	rbglocation = new RadioButton::RadioButtonGroup();

	rbbuiltin = new RadioButton(x + 10, y + 24, 60, 14, _screen, rbglocation);
	rbbuiltin->setCaption("builtin");
	rbexternal = new RadioButton(x + (THEMESELBOX_WIDTH / 2), y + 24, 60, 14, _screen, rbglocation);
	rbexternal->setCaption("external");

	rbglocation->registerChangeCallback(_onTypeChange);

	buttonok = new Button(x + (THEMESELBOX_WIDTH - 50) / 2 - 55,
	                      y + THEMESELBOX_HEIGHT - 20, 50, 14, _screen);
	buttonok->setCaption("apply");
	buttonok->registerPushCallback(_onOk);

	buttonreset = new Button(x + (THEMESELBOX_WIDTH - 50) / 2,
	                         y + THEMESELBOX_HEIGHT - 20, 50, 14, _screen);
	buttonreset->setCaption("reset");
	buttonreset->registerPushCallback(_onReset);

	buttoncancel = new Button(x + (THEMESELBOX_WIDTH - 50) / 2 + 55,
	                          y + THEMESELBOX_HEIGHT - 20, 50, 14, _screen);
	buttoncancel->setCaption("cancel");
	buttoncancel->registerPushCallback(_onCancel);

	filesel = new FileSelector(x + 10, y + 40, THEMESELBOX_WIDTH - 20,
	                           THEMESELBOX_HEIGHT - 66, _screen, true);
	filesel->registerFileSelectCallback(_onSelect);
	std::vector<std::string> themefilter;
	themefilter.push_back("nttheme");

	filesel->addFilter("theme", themefilter);

	gui.registerWidget(rbbuiltin, 0);
	gui.registerWidget(rbexternal, 0);
	gui.registerWidget(buttonok, 0);
	gui.registerWidget(buttoncancel, 0);
	gui.registerWidget(buttonreset, 0);
	gui.registerWidget(filesel, 0);
}

ThemeSelectorBox::~ThemeSelectorBox(void)
{
	delete buttonok;
	delete buttonreset;
	delete buttoncancel;
	delete filesel;
	delete rbbuiltin;
	delete rbexternal;
	delete rbglocation;
}

void ThemeSelectorBox::setDir(std::string dir)
{
	if (filesel) {
		filesel->setDir(dir);
		gui.draw();
	}
	if (rbglocation && !changingtype) {
		int newlocationtype = IsPathBuiltin(dir) ? THEMESELBOX_BUILTIN : THEMESELBOX_EXTERNAL;
		if (newlocationtype != (rbexternal->getActive() ? 1 : 0)) {
			changingtype = true;
			rbglocation->setActive(newlocationtype);
			changingtype = false;
		}
	}
}

std::string ThemeSelectorBox::getDir(void)
{
	return filesel->getDir();
}
// Event calls
void ThemeSelectorBox::penDown(u16 px, u16 py)
{
	gui.penDown(px, py);
}

void ThemeSelectorBox::penUp(u16 px, u16 py)
{
	gui.penUp(px, py);
}

void ThemeSelectorBox::penMove(u16 px, u16 py)
{
	gui.penMove(px, py);
}

void ThemeSelectorBox::show(void)
{
	gui.showAll();
	Widget::show();
}

void ThemeSelectorBox::reveal(void)
{
	Widget::reveal();
	gui.revealAll();
}

void ThemeSelectorBox::setTheme(Theme *theme_, u16 bgcolor_)
{
	theme = theme_;
	bgcolor = bgcolor_;
	gui.setTheme(theme, theme->col_light_bg);
}

// Drawing request
void ThemeSelectorBox::pleaseDraw(void)
{
	draw();
}

/* ===================== PROTECTED ===================== */

void ThemeSelectorBox::draw(void)
{
	drawGradient(theme->col_messagebox_title_col1,
	             theme->col_messagebox_title_col2, 1, 1, width - 2, 15);
	drawHLine(1, 16, width - 2, theme->col_outline);
	drawFullBox(1, 17, width - 2, THEMESELBOX_HEIGHT - 17, theme->col_light_bg);
	drawBorder(theme->col_outline);

	u8 titlewidth = getStringWidth(title) + 5;
	drawString(title, (THEMESELBOX_WIDTH - titlewidth) / 2, 4,
	           theme->col_messagebox_title_text, titlewidth + 5);

	gui.draw();
}

/* ===================== PRIVATE ===================== */
