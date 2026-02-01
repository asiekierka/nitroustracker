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

#pragma GCC diagnostic ignored "-Wnarrowing"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fxkeyboard.h"
#include "effectinput.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */


FXKeyboard::FXKeyboard(u8 _x, u8 _y, u16 *_char_base, u16 *_map_base, uint16** _vram, void (*_onFxKeypress)(u8 pressedValue, bool key_enabled), bool _visible)
	: Widget(_x, _y, FXKEYBOARD_WIDTH, FXKEYBOARD_HEIGHT, _vram, _visible),
	char_base(_char_base), map_base(_map_base), last_cmd(0), caption(0), darken_title(false), onFxKeypress(_onFxKeypress)
{
	setCaption("");
}

// Drawing request
void FXKeyboard::pleaseDraw(void) {
	draw();
}

// Event calls
void FXKeyboard::penDown(u8 px, u8 py)
{
	s16 tx, rx;
	tx = ntxm_clamp((px - x)/24, 0, 8 - 1);
	rx = (px - x) % 24;
	bool r = rx > 11;
	u8 bt_ind = (tx * 2) + (r ? 1 : 0);

	if (fxkb_state[bt_ind] != FXBUTTON_DISABLED)
		fxkb_state[bt_ind] |= 0x1; // set pushed

	updateCaptionForFx(fxkb_vals[bt_ind]);
	useDarkTitle(fxkb_state[bt_ind] == FXBUTTON_DISABLED);
	onFxKeypress(fxkb_vals[bt_ind], fxkb_state[bt_ind] != FXBUTTON_DISABLED);
	draw();
}


void FXKeyboard::penUp(u8 px, u8 py)
{
	for (int i=0;i<NUM_FXKEYS;++i) fxkb_state[i] &= ~0x1; // clear pushed
	draw();
}

void FXKeyboard::setTheme(Theme* theme_, u16 bgcolor_)
{
	u16 fxkb_cols[6] = {
		theme_->col_bg, theme_->col_fxkeyboard_col1, theme_->col_fxkeyboard_col2,
		theme_->col_fxkeyboard_col1_disabled, theme_->col_fxkeyboard_col2_disabled,
		theme_->col_outline
	};

	genPal(fxkb_cols, fxkb_pal);
	memcpy(BG_PALETTE_SUB, fxkb_pal, 32);
	theme = theme_;
	bgcolor = bgcolor_;
	draw();
}

void FXKeyboard::hide(void)
{
	Widget::hide();
	drawFullBox(0, 0, width+FXKEYBOARD_R_OVERDRAW, height, 0x0000); // show the piano
}

void tobkit::FXKeyboard::show(void)
{
	dmaCopy(effectinputTiles, char_base, sizeof(effectinputTiles));
	memcpy(BG_PALETTE_SUB, fxkb_pal, 32);

	Widget::show();
}

void FXKeyboard::setCaption(const char* _caption) {
	if (caption) ntxm_free(caption);
	caption = (char*)ntxm_cmalloc(strlen(_caption) + 1);
	strcpy(caption, _caption);
}

void FXKeyboard::useDarkTitle(bool _darken_title)
{
	darken_title = _darken_title;
}

void FXKeyboard::updateCaptionForFx(u8 val)
{
	if (category == FX_CATEGORY_NORMAL)
		setCaption(button_captions[val]);
	else if (category == FX_CATEGORY_E)
		setCaption(E_captions[val]); 

	drawCaption();
}

void FXKeyboard::setCategory(u8 newcat) {
	eraseButtonLabels();

	category = newcat;

	memset(fxkb_state, FXBUTTON_NORMAL, NUM_FXKEYS);
	setCaption(category_captions[category]);

	switch (newcat)
	{
		case FX_CATEGORY_NORMAL:
			fxkb_vals[14] = 0xF;
			memset(&fxkb_state[5], FXBUTTON_DISABLED, 3);
			break;

		case FX_CATEGORY_E:
			fxkb_vals[14] = 0xE;
			fxkb_state[0] = FXBUTTON_DISABLED;
			memset(&fxkb_state[6], FXBUTTON_DISABLED, 6);
			break;

		default:
			useDarkTitle(true);
			memset(fxkb_state, FXBUTTON_DISABLED, NUM_FXKEYS);
			break;
	}
	
	draw();
}

u8 FXKeyboard::getCategory(void)
{
	return category;
}

void FXKeyboard::setLastCmd(u8 _last_e_cmd)
{
	last_cmd = _last_e_cmd;
}

u8 FXKeyboard::getLastCmd(void)
{
	return last_cmd;
}

/* ===================== PRIVATE ===================== */

void FXKeyboard::genPal(u16 *fxkb_cols_base, u16 *pal) {
	for (int i = 0; i < 7; ++i) {
		pal[i + 3] = interpolateColor(fxkb_cols_base[3], fxkb_cols_base[4], (4096 / 6) * i);
		pal[i + 9] = interpolateColor(fxkb_cols_base[1], fxkb_cols_base[2], (4096 / 6) * i);
	}

	pal[0] = fxkb_cols_base[0];
	pal[1] = fxkb_cols_base[5];
}

void FXKeyboard::drawCaption(void) {
	if (!isExposed()) return;
	
	drawFullBox(1, 3, width, 5, bgcolor);
	u16 capcol = darken_title ? theme->col_fxkeyboard_cmd_desc_disabled : theme->col_fxkeyboard_cmd_desc;
	drawSmallString(caption, ((NUM_FXKEYS * FXBUTTON_WIDTH) / 2) - (2 * strlen(caption)), 3, capcol);  // width=(3px char+1px space) / 2 
}

void FXKeyboard::drawButtonLabel(u8 key, u8 cat, bool visible)
{
	u8 xpos = key * 12 + 3;
	u16 col = theme->col_fxkeyboard_btn_label;
	u16 smallcol1 = theme->col_fxkeyboard_minilabel_x;
	u16 smallcol2 = theme->col_fxkeyboard_minilabel_y;
	
	if(visible == true) {
		col |= BIT(15);
		smallcol1 |= BIT(15);
		smallcol2 |= BIT(15);
	}
		
	char label[] = {fxlabels[cat][key], 0};
	
	drawString(label, xpos, 15, col);

	char small_caption[3] = {0}; // one wasted byte...noone will notice

	if (cat == FX_CATEGORY_E)
		snprintf(small_caption, 3, "E%1X", fxkb_vals[key]);
	else if (cat == FX_CATEGORY_NORMAL)
		sprintf(small_caption, "X%s", ((labels_cat0 >> key) & 0x1) ? "Y" : "X");
	else
		return; // buttons in the other two categories do not have labels yet

	// the small "XX"/"XY" to show parameter format
	drawSmallChar(GLYPH_3X5(small_caption[0]), xpos, 27, smallcol1);
	drawSmallChar(GLYPH_3X5(small_caption[1]), xpos + 4, 27, small_caption[1] == 'Y' ? smallcol2 : smallcol1);
}

void FXKeyboard::drawButtonLabels(void)
{
	for (int i=0;i<NUM_FXKEYS;++i)
		drawButtonLabel(i,  category, true);
}

void FXKeyboard::eraseButtonLabels(void)
{
	for (int i=0;i<NUM_FXKEYS;++i)
		drawButtonLabel(i,  category, false);
}

void FXKeyboard::draw(void) {
	if (!isExposed()) return;

	int row;
	u16 lstate, rstate;

	/* 
				|-------|-------|------- tiles
				|-----------|----------- buttons

	  buttons are 12px wide, but tiles are 8px wide
	  to preserve exelotl's nice design we draw the tiles in
	  pairs, consisting of 3 horizontal tiles per button pair

	  the tileset is arranged such that adding the state of 
	  a button to a tile index in the default state produces 
	  the tile index of the desired state

	  basically this depends on the order of the tiles in
	  effectinput.png, so dont rearrange them (˘︶˘)
	*/
	for (int pair=0;pair<NUM_FXKEYS/2;++pair)
	{
		lstate=fxkb_state[pair*2];
		rstate=fxkb_state[pair*2+1];

		u16 buttontiles[15]={
			TILE_BLANK, TILE_LCORNER+lstate, 			 TILE_LEDGE+lstate, 			TILE_LEDGE+lstate, 			VFLIP(TILE_LCORNER+lstate),
			TILE_BLANK, TILE_MID_EDGE+(3*rstate)+lstate, TILE_MID+(3*rstate)+lstate, 	TILE_MID+(3*rstate)+lstate, VFLIP(TILE_MID_EDGE+(3*rstate)+lstate),
			TILE_BLANK, TILE_EVEN_END_EDGE+rstate, 		 TILE_EVEN_END_MID+rstate, 		TILE_EVEN_END_MID+rstate, 	VFLIP(TILE_EVEN_END_EDGE+rstate), 		//  btm edge=y flipped top edge
		};

		int abs_col=pair*3;

		for (int j=0;j<3;++j)
		{
			for (row=0;row<5;++row)
			{
				u8 pos = (row * 28) + abs_col + j;
				fxkb_map[pos]=buttontiles[row + (j*5)];
			}
		}
	}

	// we have 15 buttons so draw the odd-end tiles at the end
	if (NUM_FXKEYS % 2 == 1)
	{
		rstate=fxkb_state[NUM_FXKEYS-1];

		u16 oddend[10]={
			TILE_BLANK, TILE_LCORNER+rstate, TILE_LEDGE+rstate, TILE_LEDGE+rstate, VFLIP(TILE_LCORNER+rstate), 
			TILE_BLANK, TILE_ODD_END_EDGE+rstate, TILE_ODD_END_MID+rstate, TILE_ODD_END_MID+rstate, VFLIP(TILE_ODD_END_EDGE+rstate)
		};

		for (int j=0;j<2;++j)
		{
			for (row=0;row<5;++row)
			{
				u8 pos = (row * 28) + ((NUM_FXKEYS/2)*3) + j;
				fxkb_map[pos]=oddend[row + (j*5)];
			}
		}
	}

	for(int py=0; py<5; ++py)
		memcpy(map_base + (32*(py+y/8)+(x/8)), fxkb_map + (28 * py), 28 * 2);
	
	drawButtonLabels();
	drawCaption();
}
