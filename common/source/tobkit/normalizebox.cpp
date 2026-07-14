/*
 * NitroTracker - An FT2-style tracker for the Nintendo DS
 *
 *                                by Tobias Weyand (0xtob)
 *
 * http://nitrotracker.tobw.net
 * http://code.google.com/p/nitrotracker
 */

/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "normalizebox.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ntxm/instrument.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

// Constructor sets base variables
NormalizeBox::NormalizeBox(Screen *_screen, void (*_onOk)(void),
                           void (*_onAuto)(void), void (*_onCancel)(void))
    : Widget((_screen->getWidth() - NORMALIZEBOX_WIDTH) / 2,
             (_screen->getHeight() - NORMALIZEBOX_HEIGHT) / 2,
             NORMALIZEBOX_WIDTH, NORMALIZEBOX_HEIGHT, _screen),
      onOk(_onOk), onAuto(_onAuto), onCancel(_onCancel)
{
	title = "adjust amplitude";

	nspercent = new NumberSlider(x + (NORMALIZEBOX_WIDTH - 32) / 2, y + 20, 32,
	                             17, _screen, 100, 0, 500);

	labelpercent = new Label(x + (NORMALIZEBOX_WIDTH - 32) / 2 + 34, y + 25, 20,
	                         12, _screen, false);
	labelpercent->setCaption("%");

	buttonok = new Button(x + (NORMALIZEBOX_WIDTH - 50) / 2 - 45, y + 40, 45,
	                      14, _screen);
	buttonok->setCaption("ok");
	buttonok->registerPushCallback(_onOk);

	buttonauto = new Button(x + (NORMALIZEBOX_WIDTH - 50) / 2 + 5, y + 40, 40,
	                        14, _screen);
	buttonauto->setCaption("auto");
	buttonauto->registerPushCallback(_onAuto);

	buttoncancel = new Button(x + (NORMALIZEBOX_WIDTH - 50) / 2 + 50, y + 40,
	                          45, 14, _screen);
	buttoncancel->setCaption("cancel");
	buttoncancel->registerPushCallback(_onCancel);

	gui.registerWidget(nspercent, 0);
	gui.registerWidget(labelpercent, 0);
	gui.registerWidget(buttonok, 0);
	gui.registerWidget(buttonauto, 0);
	gui.registerWidget(buttoncancel, 0);
}

NormalizeBox::~NormalizeBox(void)
{
	delete nspercent;
	delete labelpercent;
	delete buttonok;
	delete buttonauto;
	delete buttoncancel;
}

// Drawing request
void NormalizeBox::pleaseDraw(void)
{
	draw();
}

// Event calls
void NormalizeBox::penDown(u16 px, u16 py)
{
	gui.penDown(px, py);
}

void NormalizeBox::penUp(u16 px, u16 py)
{
	gui.penUp(px, py);
}

void NormalizeBox::penMove(u16 px, u16 py)
{
	gui.penMove(px, py);
}

s16 NormalizeBox::getValue(void)
{
	return nspercent->getValue();
}

void NormalizeBox::show(void)
{
	gui.showAll();
	Widget::show();
}

void NormalizeBox::reveal(void)
{
	Widget::reveal();
	gui.revealAll();
}

void NormalizeBox::setTheme(Theme *theme_, u16 bgcolor_)
{
	theme = theme_;
	bgcolor = bgcolor_;
	gui.setTheme(theme, theme->col_light_bg);
}

/* ===================== PRIVATE ===================== */

void NormalizeBox::draw(void)
{
	drawGradient(theme->col_messagebox_title_col1,
	             theme->col_messagebox_title_col2, 1, 1, width - 2, 15);
	drawHLine(1, 16, width - 2, theme->col_outline);
	drawFullBox(1, 17, width - 2, NORMALIZEBOX_HEIGHT - 17,
	            theme->col_light_bg);
	drawBorder(theme->col_outline);

	u8 titlewidth = getStringWidth(title) + 5;
	drawString(title, (NORMALIZEBOX_WIDTH - titlewidth) / 2, 3,
	           theme->col_messagebox_title_text, titlewidth + 5);

	gui.draw();
}
