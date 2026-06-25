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

#include <stdio.h>
#include <string.h>

#include "tobkit/piano.h"

#ifdef TOBKIT_PLATFORM_NDS
#include "piano.h"
#endif
#include "piano_hit.h"

using namespace tobkit;

#define clamp(v, vmin, vmax) (((v) < (vmin)) ? (vmin) : ((v > (vmax)) ? (vmax) : (v)))

static u8 halfkeys[5] = {1, 3, 6, 8, 10};
static u8 fullkeys[7] = {0, 2, 4, 5, 7, 9, 11};
static u8 fullkeyoffset[12] = {0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6};

/* ===================== PUBLIC ===================== */
Piano::Piano(u16 _x, u16 _y, u16 _width, u16 _height, u16 *_char_base, u16 *_map_base, Screen *_screen)
:Widget(_x, _y, _width, _height, _screen),
char_base(_char_base), map_base(_map_base), key_labels_visible(false), mapping_instrument(false), curr_note(255)
{
	onNote = 0;
	onRelease = 0;

#ifdef TOBKIT_PLATFORM_NDS
	// Piano::draw is never invoked before Piano::setTheme
	dmaCopy(pianoTiles, char_base, sizeof(pianoTiles));
#endif

    key_labels = (char*) ntxm_cmalloc((_width + 7) >> 3);
	memset(key_labels, ' ', getWidthTiles());
}

Piano::~Piano() {
    ntxm_free(key_labels);
}

void Piano::setTheme(Theme *theme_, u16 bgcolor_) {
	u16 piano_cols[9] = { theme_->col_piano_full_col1, theme_->col_piano_full_col2, theme_->col_piano_half_col1, theme_->col_piano_half_col2,
						theme_->col_piano_full_highlight_col1, theme_->col_piano_full_highlight_col2, theme_->col_piano_half_highlight_col1,
						theme_->col_piano_half_highlight_col2, theme_->col_piano_outline};
	genPal(piano_cols, piano_Palette, piano_fullnotehighlight_Palette, piano_halfnotehighlight_Palette);
	Widget::setTheme(theme_, bgcolor_);

#ifdef TOBKIT_PLATFORM_NDS
	// doesn't conflict with fx keyboard, so safe to write these
	memcpy(BG_PALETTE_SUB+16, piano_fullnotehighlight_Palette, 32);
	memcpy(BG_PALETTE_SUB+32, piano_halfnotehighlight_Palette, 32);
#endif

	if (!isExposed()) return;

#ifdef TOBKIT_PLATFORM_NDS
	memcpy(BG_PALETTE_SUB, piano_Palette, 32);
#endif

	setInMappingMode(mapping_instrument);
	pleaseDraw();
}

void Piano::show(void)
{
#ifdef TOBKIT_PLATFORM_NDS
	dmaCopy(pianoTiles, char_base, sizeof(pianoTiles));
	memcpy(BG_PALETTE_SUB, piano_Palette, 32);
#endif

	Widget::show();
}

// Drawing request
void Piano::pleaseDraw(void) {
	draw();
}


// Event calls
void Piano::penDown(u16 px, u16 py)
{
	// Look up the note in the hit-array
	s16 kbx, kby;
	kbx = clamp((px - x), 0, width - 1) / 8;
	kby = clamp((py - y), 0, height - 1) / 8;

	u8 note = piano_hit[kby][kbx % 14] + ((kbx / 14) * 12);

	setKeyPal(note);
#ifndef TOBKIT_PLATFORM_NDS
	pleaseDraw();
#endif
	if(onNote) {
		onNote(note);
	}

	curr_note = note;
}

void Piano::penMove(u16 px, u16 py)
{
	// Look up the note in the hit-array
	s16 kbx, kby;
	kbx = clamp((px - x), 0, width - 1) / 8;
	kby = clamp((py - y), 0, height - 1) / 8;

	u8 note = piano_hit[kby][kbx % 14] + ((kbx / 14) * 12);

	// Only when it moves to another note
	if (note != curr_note) {
		resetPals();
		if(onRelease) {
			onRelease(curr_note, true);
		}

		setKeyPal(note);

		if(onNote) {
			onNote(note);
		}

		curr_note = note;

		// draw();
	}

	#ifndef TOBKIT_PLATFORM_NDS
		pleaseDraw();
	#endif
}


void Piano::penUp(u16 px, u16 py)
{
	resetPals();

	if(onRelease)
	{
		onRelease(curr_note, false);
	}
#ifndef TOBKIT_PLATFORM_NDS
	curr_note = 255;
	pleaseDraw();
#endif
}

// Callback registration
void Piano::registerNoteCallback(void (*onNote_)(u8)) {
	onNote = onNote_;
}

void Piano::registerReleaseCallback(void (*onRelease_)(u8, bool)) {
	onRelease = onRelease_;
}

// Key label handling
void Piano::showKeyLabels(void)
{
	key_labels_visible = true;

	for(u8 key=0; key<getWidthTiles(); ++key)
		drawKeyLabel(key);
}

void Piano::hideKeyLabels(void)
{
	key_labels_visible = false;

	for(u8 key=0; key<getWidthTiles(); ++key)
		eraseKeyLabel(key);
}

void Piano::setInMappingMode(bool instmap)
{
	mapping_instrument = instmap;
	u16 col = theme->col_signal & ~RGB5A1_ALPHA_BIT;
	if (!instmap)
	{
		drawHLine(0, height-1, width, theme->col_piano_half_col1);
		drawVLine(0, 1, height-1, theme->col_piano_half_col2);
		drawVLine(width-1, 1, height-1, theme->col_piano_half_col1);
	}
	drawBox(0, 1, width, height-1, mapping_instrument ? col | RGB5A1_ALPHA_BIT : col);
}

void Piano::setKeyLabel(u8 key, char label)
{
	eraseKeyLabel(key);

	key_labels[key] = label;

	drawKeyLabel(key);
}

/* ===================== PRIVATE ===================== */

void Piano::genPal(u16 *piano_cols_base, u16 *pal, u16 *pal_full_highlight, u16 *pal_half_highlight) {
	for (int i = 0; i < 9; ++i) {
		pal[i] = interpolateColor(piano_cols_base[3], piano_cols_base[2], (4096 / 8) * i + 1);
		pal_half_highlight[i] = interpolateColor(piano_cols_base[6], piano_cols_base[7], (4096 / 9) * i + 1);

		if (i > 6) continue;

		pal[i + 9] = interpolateColor(piano_cols_base[0], piano_cols_base[1], (4096 / 7) * i);
		pal_full_highlight[i + 9] = interpolateColor(piano_cols_base[4], piano_cols_base[5], (4096 / 7) * i);
	}

	memcpy(&pal_full_highlight[0], &pal[0], 9 * sizeof(u16));
	memcpy(&pal_half_highlight[9], &pal[9], 7 * sizeof(u16));

	pal[0] = piano_cols_base[8];
}

void Piano::draw(void)
{
#ifdef TOBKIT_PLATFORM_NDS
	// Fill screen with empty tiles
	for (int i = 0; i < 768; i++) map_base[i] = 28;

	// Copy the piano to the screen
	for(int py=0; py<PIANO_HEIGHT_TILES; ++py)
	{
		memcpy(map_base + (32*(py+y/8)+(x/8)), pianoMap + (PIANO_WIDTH_TILES * py), PIANO_WIDTH_TILES * 2);
	}
#else
	if (!isExposed()) {
		drawFullBox(0, 0, width, height, theme->col_bg);
	} else {
		drawFullBox(0, 1, width, height-1, theme->col_piano_outline);

		const u8 FULLKEY_WIDTH = 15;
		const u8 FULLKEY_HEIGHT = 38;

		const u8 HALFKEY_WIDTH = 10;
		const u8 HALFKEY_HEIGHT = 23;

		for (int i = 0,key_draw_x = 0;i<(width / 16); ++i,key_draw_x+=16)
		{
			u16 col1 = curr_note != (fullkeys[i % 7] + ((i / 7) * 12)) ? theme->col_piano_full_col1 : theme->col_piano_full_highlight_col1;
			u16 col2 = curr_note != (fullkeys[i % 7] + ((i / 7) * 12)) ? theme->col_piano_full_col2 : theme->col_piano_full_highlight_col2;

			drawFullBox(1+key_draw_x, 1, FULLKEY_WIDTH, FULLKEY_HEIGHT, col2);
			drawHorizontalGradient(col2, col1, 2 + key_draw_x, 2, FULLKEY_WIDTH-2, FULLKEY_HEIGHT-2);
		}

		for (int j = 0, key_draw_x = 0; ; ++j,key_draw_x+=16)
		{
			if ((j % 5)==0||(j % 5)==2)
				key_draw_x += 16;
			if (key_draw_x > (width - 16))
			    break;

			u16 col1 = curr_note != (halfkeys[j % 5] + ((j / 5) * 12)) ? theme->col_piano_half_col1 : theme->col_piano_half_highlight_col2;
			u16 col2 = curr_note != (halfkeys[j % 5] + ((j / 5) * 12)) ? theme->col_piano_half_col2 : theme->col_piano_half_highlight_col1;

			drawFullBox(1+key_draw_x - 5, 1, HALFKEY_WIDTH, HALFKEY_HEIGHT, col2);
			drawGradient(col2, col1, 2 + key_draw_x - 5, 2, HALFKEY_WIDTH-2, HALFKEY_HEIGHT-2);
		}
	}
#endif
}

// Set the key corresp. to note to palette corresp. to pal_idx
void Piano::setKeyPal(u8 note)
{
#ifdef TOBKIT_PLATFORM_NDS
  u8 px, py, hit_row, pal_idx;

  if(isHalfTone(note))
  {
    hit_row = 0;
    pal_idx = 2;
  }
  else
  {
    hit_row = 4;
    pal_idx = 1;
  }

  for(px=0; px<PIANO_WIDTH_TILES; ++px)
  {
    if((piano_hit[hit_row][px % 14] + ((px / 14) * 12)) == note)
	{
      for(py=0; py<PIANO_HEIGHT_TILES; ++py)
	  {
      	map_base[32*(py+y/8)+(px+x/8)] &= ~(3 << 12); // Clear bits 12 and 13 (from the left)
        map_base[32*(py+y/8)+(px+x/8)] |= (pal_idx << 12); // Write the pal index to bits 12 and 13
      }
    }
  }
#endif
}

// 1 for halftones, 0 for fulltones
u8 Piano::isHalfTone(u8 note)
{
	for(int i=0;i<5;++i) {
		if((note%12)==halfkeys[i]) return 1;
	}
	return 0;
}

// Reset piano colors to normal
void Piano::resetPals(void)
{
#ifdef TOBKIT_PLATFORM_NDS
  u8 px,py;
  for(px=0; px<PIANO_WIDTH_TILES; ++px) {
    for(py=0; py<PIANO_HEIGHT_TILES; ++py) {
      map_base[32*(py+y/8)+(px+x/8)] &= ~(3 << 12); // Clear bits 12 and 13 (from the left)
    }
  }
#endif
}

void Piano::drawKeyLabel(u8 key, bool visible)
{
	u8 xpos, ypos, offset;
	u16 col;

	if(isHalfTone(key) == true)
	{
		ypos = 12;
		col = theme->col_piano_label_inv;
		offset = 14;
	}
	else
	{
		ypos = 28;
		col = theme->col_piano_label;
		offset = 5;
	}

	if(visible == true)
		col |= RGB5A1_ALPHA_BIT;

	xpos = offset + (fullkeyoffset[key % 12] * 16) + ((key / 12) * 112);

	char label[] = {key_labels[key], 0};

	drawString(label, xpos, ypos, col);
}

void Piano::eraseKeyLabel(u8 key)
{
	drawKeyLabel(key, false);
}
