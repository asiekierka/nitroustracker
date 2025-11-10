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

#include <sys/syslimits.h>
#include <unistd.h>

#include <fat.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>

#include "tobkit/themeselectorbox.h"


#define THEMESELBOX_WIDTH		180
#define THEMESELBOX_HEIGHT		128

using namespace tobkit;

/* ===================== PUBLIC ===================== */

ThemeSelectorBox::ThemeSelectorBox(u16 **_vram, void (*_onSelect)(File), void(*_onOk)(void), void (*_onReset)(void), void (*_onCancel)(void))
	:Widget((SCREEN_WIDTH-THEMESELBOX_WIDTH)/2, (SCREEN_HEIGHT-THEMESELBOX_HEIGHT)/3,
		THEMESELBOX_WIDTH, THEMESELBOX_HEIGHT, _vram), onSelect(_onSelect), onOk(_onOk), onReset(_onReset), onCancel(_onCancel)
{
    title = "choose a theme";

	buttonok = new Button(x+(THEMESELBOX_WIDTH-50)/2 - 55, y+THEMESELBOX_HEIGHT-20, 50, 14, _vram);
	buttonok->setCaption("apply");
	buttonok->registerPushCallback(onOk);

	buttonreset = new Button(x+(THEMESELBOX_WIDTH-50)/2, y+THEMESELBOX_HEIGHT-20, 50, 14, _vram);
	buttonreset->setCaption("reset");
	buttonreset->registerPushCallback(onReset);

	buttoncancel = new Button(x+(THEMESELBOX_WIDTH-50)/2 + 55, y+THEMESELBOX_HEIGHT-20, 50, 14, _vram);
	buttoncancel->setCaption("cancel");
	buttoncancel->registerPushCallback(onCancel);
	
	filesel = new FileSelector(x + 10, y + 25, THEMESELBOX_WIDTH-20, THEMESELBOX_HEIGHT - 50, _vram, true);
	filesel->registerFileSelectCallback(onSelect);
	std::vector<std::string> themefilter;
	themefilter.push_back("nttheme");

	filesel->addFilter("theme", themefilter);

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
}

void ThemeSelectorBox::setDir(std::string dir)
{
	if (filesel)
		filesel->setDir(dir);
}

std::string ThemeSelectorBox::getDir(void)
{
	return filesel->getDir();
}
// Event calls
void ThemeSelectorBox::penDown(u8 px, u8 py) {
	gui.penDown(px, py);
}

void ThemeSelectorBox::penUp(u8 px, u8 py) {
	gui.penUp(px, py);
}

void ThemeSelectorBox::penMove(u8 px, u8 py) {
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
	bgcolor=bgcolor_;
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
	drawGradient(theme->col_list_highlight1, theme->col_list_highlight2, 1, 1, width - 2, 15);
	drawHLine(1, 16, width - 2, theme->col_outline);
	drawFullBox(1, 17, width - 2, THEMESELBOX_HEIGHT-17, theme->col_light_bg);
	drawBorder(theme->col_outline);
	
	u8 titlewidth = getStringWidth(title)+5;
	drawString(title, (THEMESELBOX_WIDTH-titlewidth)/2, 4, theme->col_text, titlewidth+5);
	
	gui.draw();
}

/* ===================== PRIVATE ===================== */


