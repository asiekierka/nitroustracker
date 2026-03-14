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

#ifndef FXKEYBOARD_H
#define FXKEYBOARD_H

#include "tobkit/label.h"

#define FXBUTTON_WIDTH		12
#define FXBUTTON_HEIGHT		27

#define FXKEYBOARD_WIDTH		176
#define FXKEYBOARD_HEIGHT		40

// the fx keyboard isn't quite as wide as the piano, but we want to draw blank tiles
// over where the piano was anyway
#define FXKEYBOARD_WIDTH_TILES	28
#define FXKEYBOARD_HEIGHT_TILES	5

#define FXKEYBOARD_R_OVERDRAW	44
#define FXKEYBOARD_LMARGIN		2
#define FXKEYBOARD_YMARGIN		12

#define NUM_FXKEYS				15
#define NUM_CATEGORIES			4

#define FX_CATEGORY_NORMAL 		0
#define FX_CATEGORY_E 			1
#define FX_CATEGORY_FT 			2
#define FX_CATEGORY_VOL 		3

// button state, can be added to tilemap index to get tile in said state
// eg TILE_LCORNER + FXBUTTON_DISABLED = TILE_LCORNER (disabled)
#define FXBUTTON_NORMAL			0
#define FXBUTTON_DOWN			1
#define FXBUTTON_DISABLED		2

#define NO_EFFECT				255

// Tilemap indices
#define TILE_BLANK				0x0000
#define TILE_LCORNER			0x0001
#define TILE_MID_EDGE			0x0004
#define TILE_EVEN_END_EDGE		0x000d
#define TILE_ODD_END_EDGE		0x0010
#define TILE_LEDGE				0x0013
#define TILE_MID				0x0016
#define TILE_EVEN_END_MID		0x001f
#define TILE_ODD_END_MID		0x0022

#define VFLIP(t)				((t) | BIT(11))

namespace tobkit {


	class FXKeyboard : public Widget {
	public:
		FXKeyboard(u8 _x, u8 _y, u16* _char_base, u16* _map_base, u16** _vram,
			void (*onFxKeypress)(u8 pressedValue), bool _visible = true);

		// Drawing request
		void pleaseDraw(void);

		// Event calls
		void penDown(u8 x, u8 y);
		void penUp(u8 x, u8 y);

		void setTheme(Theme* theme_, u16 bgcolor_);
		void hide(void);
		void show(void);

		void setCaption(const char* caption);
		void updateCaptionForFx(u8 val);
		void useDarkTitle(bool _darken_title);

		void setCategory(u8 newcat);
		u8 getCategory(void);

		void setLastCmd(u8 _last_cmd);
		u8 getLastCmd(void);

	private:
		uint16* char_base, * map_base;

		u8 category;
		u8 last_cmd;

		u16 fxkb_map[FXKEYBOARD_WIDTH_TILES*FXKEYBOARD_HEIGHT_TILES] __attribute__((aligned(4))) = { 0 };
		u16 fxkb_pal[16];
		u8 fxkb_state[NUM_FXKEYS] = { 0 };
		u8 fxkb_vals[NUM_FXKEYS] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xf };

		u16 labels_cat0 = 0b10011110001; // whether to show "XX" or "XY" on an fx key

		char* caption;
		bool darken_title;

		const char* fxlabels[NUM_CATEGORIES] = { "0123456789abcdf", "0123456789abcde", "GHKLPRTX       ", "+-DLMPRSUV    " };
		const char* category_captions[NUM_CATEGORIES] = { "EFFECT COMMANDS", "EXY: EXTENDED COMMANDS", "FT2 COMMANDS", "VOLUME COLUMN COMMANDS" };
		const char* button_captions[16] = { "0XY: ARPEGGIO", "1XX: PORTAMENTO UP", "2XX: PORTAMENTO DOWN",
									 "3XX: PORTAMENTO TO NOTE", "4XY: VIBRATO", "5XY: PORTAMENTO TO NOTE \x60 VOLUME SLIDE",
									 "6XY: VIBRATO WITH VOLUME SLIDE", "7XY: TREMOLO", "8XX: SET NOTE PANNING POSITION",
									 "9XX: SAMPLE OFFSET", "AXY: VOLUME SLIDE", "BXX: JUMP TO ORDER", "CXX: SET NOTE VOLUME",
									 "DXX: PATTERN BREAK", "UNUSED", "FXX: SET SONG SPEED \x60 BPM" };

		const char* E_captions[16] = { "E0X: AMIGA LED FILTER TOGGLE", "E1X: FINE PORTAMENTO UP",
									 "E2X: FINE PORTAMENTO DOWN", "E3X: GLISSANDO CONTROL",
									 "E4X: VIBRATO CONTROL", "E5X: SET NOTE FINETUNE",
									 "E6X: PATTERN LOOP", "E7X: TREMOLO CONTROL",
									 "E8X: SET NOTE PANNING POSITION", "E9X: RETRIGGER NOTE",
									 "EAX: FINE VOLUME SLIDE UP", "EBX: FINE VOLUME SLIDE DOWN",
									 "ECX: NOTE CUT", "EDX: NOTE DELAY", "EEX: PATTERN DELAY", "EFX: FUNK IT" };

		void genPal(u16* fxkb_cols_base, u16* pal);

		void (*onFxKeypress)(u8 val);

		void drawCaption(void);

		void drawButtonLabel(u8 key, u8 cat, bool visible);
		void drawButtonLabels(void);
		void eraseButtonLabels(void);

		void draw(void);
	};

};

#endif
