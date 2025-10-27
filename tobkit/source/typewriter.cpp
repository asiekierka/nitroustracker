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

#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tobkit/typewriter.h"
#include "typewriter.h"
#include "typewriter_hit.h"

using namespace tobkit;

#define MAX_TEXT_LEN	256 // TODO: make dynamic

#define TW_TILE_WIDTH 26
#define TW_TILE_HEIGHT 12
#define TW_TILE_X 4
#define TW_TILE_Y 16
#define TW_WIDTH	((TW_TILE_WIDTH)*8+(TW_TILE_X)*2)
#define TW_HEIGHT	((TW_TILE_HEIGHT)*8+(TW_TILE_Y)*2-1)

/* ===================== PUBLIC ===================== */

Typewriter::Typewriter(const char *_msg, u16 *_char_base, 
	u16 *_map_base, u8 _palette_offset, uint16 **_vram, vuint16* _trans_reg_x,
	vuint16* _trans_reg_y)
	:Widget((SCREEN_WIDTH-TW_WIDTH)/2, (SCREEN_HEIGHT-TW_HEIGHT)/2-15, TW_WIDTH, TW_HEIGHT, _vram),
	char_base(_char_base), map_base(_map_base), palette_offset(_palette_offset),
	kx(x+TW_TILE_X), ky(y+TW_TILE_Y),
	mode(TYPEWRITER_MODE_NORMAL),
	trans_reg_x(_trans_reg_x), trans_reg_y(_trans_reg_y), cursorpos(0), strlength(0)
{
	onOk = 0;
	onCancel = 0;

	dmaCopy(typewriterPal, BG_PALETTE_SUB+palette_offset*16, 32);
	// generate highlight palette
	for (int i = 0; i < 16; i++) {
		BG_PALETTE_SUB[palette_offset * 16 + 16 + i] = typewriterPal[i] ? 0xFFFF : 0x0000;
	}

	decompress(typewriterTiles, char_base, LZ77Vram);

	u8 msglength = getStringWidth(_msg);
	
	msglabel = new Label(x+4, y+6, msglength+4, 12, _vram, false);
	msglabel->setCaption(_msg);
	gui.registerWidget(msglabel, 0, SUB_SCREEN);
	
	label = new Label(x+msglength+8, y+4, TW_WIDTH-msglength-12, 15, _vram, true, false, false, true);
	gui.registerWidget(label, 0, SUB_SCREEN);
	
	buttonok = new Button(x+TW_WIDTH/2+2, y+TW_HEIGHT-12-4, 50, 12, _vram);
	buttonok->setCaption("ok");
	gui.registerWidget(buttonok, 0, SUB_SCREEN);
	
	buttoncancel = new Button(x+TW_WIDTH/2-50-2, y+TW_HEIGHT-12-4, 50, 12, _vram);
	buttoncancel->setCaption("cancel");
	gui.registerWidget(buttoncancel, 0, SUB_SCREEN);
	
	// Set the tile bg translation registers to move the typewriter to (x,y)
	*_trans_reg_x = -kx;
	*_trans_reg_y = -ky;
	
	text = (char*)calloc(1, MAX_TEXT_LEN+1);
}

Typewriter::~Typewriter(void)
{
	delete label;
	delete msglabel;
	delete buttonok;
	delete buttoncancel;
	free(text);
	
	for(int py=0; py<TW_TILE_HEIGHT; ++py) {
		memset(map_base + 32*py, 0, TW_TILE_WIDTH*2);
	}
}


// Drawing request
void Typewriter::pleaseDraw(void) {
	draw();
}

// Event calls
void Typewriter::penDown(u8 px, u8 py)
{
	// Inside the kb?
	if((px>=kx)&&(px<=kx+TW_TILE_WIDTH*8)&&(py>=ky)&&(py<=ky+TW_TILE_HEIGHT*8))
	{
		tilex = (px-kx)/8;
		tiley = (py-ky)/8;
		
		setTile(tilex, tiley, 4);
		
		if(tilex>=1 && tilex<(TW_TILE_WIDTH-1) && tiley<TW_TILE_HEIGHT)
		{
			char c;
			if((mode==TYPEWRITER_MODE_CAPS)||(mode==TYPEWRITER_MODE_SHIFT))
				c = typewriter_Hit_Shift[tilex+(tiley*TW_TILE_WIDTH)];
			else
				c = typewriter_Hit[tilex+(tiley*TW_TILE_WIDTH)];
			
			if(c==CAP)
			{
				if((mode==TYPEWRITER_MODE_CAPS)||(mode==TYPEWRITER_MODE_SHIFT)) {
					mode = TYPEWRITER_MODE_NORMAL;
				} else {
					mode = TYPEWRITER_MODE_CAPS;
				}
				redraw();
			}
			else if (c==SHF)
			{
				if((mode==TYPEWRITER_MODE_CAPS)||(mode==TYPEWRITER_MODE_SHIFT)) {
					mode = TYPEWRITER_MODE_NORMAL;
				} else {
					mode = TYPEWRITER_MODE_SHIFT;
				}
				redraw();
			}
			else if (c==BSP)
			{
				if(cursorpos > 0) {
					for(u16 i=cursorpos-1;i<=strlength;++i) {
						text[i] = text[i+1];
					}
					text[strlength+1] = 0;
					cursorpos--;
					strlength--;
					label->setCaption(text);
					drawCursor();
				}
			}
			else if (c==RET)
			{
				if(onOk!=0) {
					onOk();
				}
			}
			else if(c!=NOK)
			{
				if(mode==TYPEWRITER_MODE_SHIFT) {
					mode = TYPEWRITER_MODE_NORMAL;
					redraw();
				}
				if(strlength<MAX_TEXT_LEN)
				{
					for(u16 i=strlength;i>cursorpos;i--) {
						text[i] = text[i-1];
					}
					text[cursorpos] = c;
					cursorpos++;
					strlength++;
					label->setCaption(text);
					drawCursor();
				}
			}
		}
		
	// Inside the button area?
	} else if ((px>x)&&(px<x+TW_WIDTH)&&(py<y+TW_HEIGHT)&&(py>ky+TW_TILE_HEIGHT*8)) {
		gui.penDown(px, py);
	}
}

void Typewriter::penUp(u8 px, u8 py)
{
	setTile(tilex, tiley, 3);
	gui.penUp(px, py);
}

void Typewriter::buttonPress(u16 button)
{
	if((button==KEY_LEFT)&&(cursorpos>0)) {
		cursorpos--;
		redraw();
	} else if((button==KEY_RIGHT)&&(cursorpos<=strlength)) {
		cursorpos++;
		redraw();
	}
}

// Callback registration
void Typewriter::registerOkCallback(void (*onOk_)(void))
{
	onOk = onOk_;
	buttonok->registerPushCallback(onOk);
}

void Typewriter::registerCancelCallback(void (*onCancel_)(void))
{
	onCancel = onCancel_;
	buttoncancel->registerPushCallback(onCancel);
}

void Typewriter::setText(const char *text_)
{
	strncpy(text, text_, MAX_TEXT_LEN);
	text[MAX_TEXT_LEN] = 0;
	strlength = strlen(text);
	cursorpos = strlength;
	label->setCaption(text);
	redraw();
}

char *Typewriter::getText(void) {
	return text;
}

void Typewriter::show(void)
{
	Widget::show();
	gui.showAll();
	redraw();
}

void Typewriter::reveal(void)
{
	Widget::reveal();
	gui.revealAll();
	redraw();
}

void Typewriter::setTheme(Theme *theme_, u16 bgcolor_)
{
	theme = theme_;
	bgcolor = bgcolor_;
	
	label->setTheme(theme, theme->col_light_bg);
	msglabel->setTheme(theme, theme->col_light_bg);
	buttonok->setTheme(theme, theme->col_light_bg);
	buttoncancel->setTheme(theme, theme->col_light_bg);
}

/* ===================== PRIVATE ===================== */

void Typewriter::draw(void)
{
	drawFullBox(1, 1, TW_WIDTH - 2, TW_HEIGHT - 2, theme->col_light_bg);
	drawBorder(theme->col_outline);
	gui.draw();
	
	redraw();
}

// don't draw the box new
void Typewriter::redraw(void)
{
	if(!isExposed())
		return;
	
	label->pleaseDraw();
	msglabel->pleaseDraw();
	
	drawCursor();
	
	u16 map_offset;
	if((mode == TYPEWRITER_MODE_CAPS)||(mode == TYPEWRITER_MODE_SHIFT)) {
		map_offset = TW_TILE_WIDTH*TW_TILE_HEIGHT;
	} else {
		map_offset = 0;
	}
	
	u16 tile_attr = palette_offset << 12;
    for(u8 py=0; py<TW_TILE_HEIGHT; ++py) {
		for(u8 px=0; px<TW_TILE_WIDTH; ++px) {
		  map_base[32*py+px] = typewriterMap[map_offset+26*py+px] | tile_attr;
        }
    }
}

// Don't try this at home!
void Typewriter::drawCursor(void)
{
	u8 lx, ly, lw, lh;
	u16 cursorx, cursory, cursorheight;
	label->getPos(&lx, &ly, &lw, &lh);
	cursorx = lx - x + getStringWidth(text, cursorpos) + 1;
	cursory = ly - y + 1;
	cursorheight = lh - 2;
	if(cursorx<lx -x + lw) {
		drawVLine(cursorx, cursory, cursorheight, theme->col_typewriter_cursor);
	}
}

// This function is kindly sponsored by Headspin
void Typewriter::setTile(int x, int y, int pal)
{
	char c;
	int x2, y2;

	c = typewriter_Hit[(y*TW_TILE_WIDTH)+x];

	if(!c) return;

	map_base[(y*32)+x] &= ~(7 << 12);
	map_base[(y*32)+x] |= (pal << 12);

	x2 = x; y2 = y;
	while(typewriter_Hit[(y2*TW_TILE_WIDTH)+x2]==c)
	{
		map_base[(y2*32)+x2] &= ~(7 << 12);
		map_base[(y2*32)+x2] |= (pal << 12);

		x2 = x;
		while(typewriter_Hit[(y2*TW_TILE_WIDTH)+x2]==c)
		{
			map_base[(y2*32)+x2] &= ~(7 << 12);
			map_base[(y2*32)+x2] |= (pal << 12);
			x2++;
		}
		x2 = x;
		while(typewriter_Hit[(y2*TW_TILE_WIDTH)+x2]==c)
		{
			map_base[(y2*32)+x2] &= ~(7 << 12);
			map_base[(y2*32)+x2] |= (pal << 12);
			x2--;
		}

		x2 = x;
		y2++;
	}

	x2 = x; y2 = y;
	while(typewriter_Hit[(y2*TW_TILE_WIDTH)+x2]==c)
	{
		map_base[(y2*32)+x2] &= ~(7 << 12);
		map_base[(y2*32)+x2] |= (pal << 12);

		x2 = x;
		while(typewriter_Hit[(y2*TW_TILE_WIDTH)+x2]==c)
		{
			map_base[(y2*32)+x2] &= ~(7 << 12);
			map_base[(y2*32)+x2] |= (pal << 12);
			x2++;
		}
		x2 = x;
		while(typewriter_Hit[(y2*TW_TILE_WIDTH)+x2]==c)
		{
			map_base[(y2*32)+x2] &= ~(7 << 12);
			map_base[(y2*32)+x2] |= (pal << 12);
			x2--;
		}

		x2 = x;
		y2--;
	}
}