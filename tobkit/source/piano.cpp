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

#include "piano.h"
#include "piano_hit.h"

using namespace tobkit;

#define clamp(v, vmin, vmax) (((v) < (vmin)) ? (vmin) : ((v > (vmax)) ? (vmax) : (v)))

static u8 halftones[] = {1, 3, 6, 8, 10, 13, 15, 18, 20, 22};
static u8 x_offsets[] = {0, 11, 16, 27, 32, 48, 59, 64, 75, 80, 91, 96};



/* ===================== PUBLIC ===================== */
Piano::Piano(u8 _x, u8 _y, u8 _width, u8 _height, u16 *_char_base, u16 *_map_base, u16 **_vram)
:Widget(_x, _y, _width, _height, _vram),
char_base(_char_base), map_base(_map_base), key_labels_visible(false), mapping_instrument(false)
{
	onNote = 0;
	onRelease = 0;

	// Piano::draw is never invoked before Piano::setTheme
	dmaCopy(pianoTiles, char_base, sizeof(pianoTiles));
	
	memset(key_labels, ' ', 24);
}

void Piano::setTheme(Theme *theme_, u16 bgcolor_) {
	u16 piano_cols[8] = { theme_->col_piano_full_col1, theme_->col_piano_full_col2, theme_->col_piano_half_col1, theme_->col_piano_half_col2, 
						theme_->col_piano_full_highlight_col1, theme_->col_piano_full_highlight_col2, theme_->col_piano_half_highlight_col1, theme_->col_piano_half_highlight_col2};
	genPal(piano_cols, piano_Palette, piano_fullnotehighlight_Palette, piano_halfnotehighlight_Palette);
	Widget::setTheme(theme_, bgcolor_);
	memcpy(BG_PALETTE_SUB, piano_Palette, 32);
	memcpy(BG_PALETTE_SUB+16, piano_fullnotehighlight_Palette, 32);
	memcpy(BG_PALETTE_SUB+32, piano_halfnotehighlight_Palette, 32);
	setInMappingMode(mapping_instrument);
	pleaseDraw();
}


// Drawing request
void Piano::pleaseDraw(void) {
	draw();
}


// Event calls
void Piano::penDown(u8 px, u8 py)
{
	// Look up the note in the hit-array
	s16 kbx, kby;
	kbx = clamp((px - x)/8, 0, PIANO_WIDTH_TILES - 1);
	kby = clamp((py - y)/8, 0, PIANO_HEIGHT_TILES - 1);
	
	u8 note = piano_hit[kby][kbx];
	
	setKeyPal(note);
	
	if(onNote) {
		onNote(note);
	}
	
	curr_note = note;
}

void Piano::penMove(u8 px, u8 py)
{
	// Look up the note in the hit-array
	s16 kbx, kby;
	kbx = clamp((px - x)/8, 0, PIANO_WIDTH_TILES - 1);
	kby = clamp((py - y)/8, 0, PIANO_HEIGHT_TILES - 1);

	u8 note = piano_hit[kby][kbx];
	
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
}


void Piano::penUp(u8 px, u8 py)
{
	resetPals();
	
	if(onRelease)
	{
		onRelease(curr_note, false);
	}
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
	
	for(u8 key=0; key<24; ++key)
		drawKeyLabel(key);
}

void Piano::hideKeyLabels(void)
{
	key_labels_visible = false;
	
	for(u8 key=0; key<24; ++key)
		eraseKeyLabel(key);
}

void Piano::setInMappingMode(bool instmap)
{
	mapping_instrument = instmap;
	u16 col = theme->col_signal & ~BIT(15);
	if (!instmap)
	{
		drawHLine(0, height-1, width, theme->col_piano_half_col1);
		drawVLine(0, 1, height-1, theme->col_piano_half_col2);
		drawVLine(width-1, 1, height-1, theme->col_piano_half_col1);
	}
	drawBox(0, 1, width, height-1, mapping_instrument ? col | BIT(15) : col);
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
		pal[i] = interpolateColor(piano_cols_base[3], piano_cols_base[2], (4096 / 9) * i + 1);
		pal_half_highlight[i] = interpolateColor(piano_cols_base[6], piano_cols_base[7], (4096 / 9) * i + 1);
	}
	memcpy(&pal_full_highlight[0], &pal[0], 9 * sizeof(u16));

	for (int i = 0; i < 7; ++i) {
		pal[i + 9] = interpolateColor(piano_cols_base[0], piano_cols_base[1], (4096 / 7) * i);
		pal_full_highlight[i + 9] = interpolateColor(piano_cols_base[4], piano_cols_base[5], (4096 / 7) * i);
	}
	// pal_full_highlight[9] = piano_cols_base[1];
	memcpy(&pal_half_highlight[9], &pal[9], 7 * sizeof(u16));
}

void Piano::draw(void)
{
	// Fill screen with empty tiles
	for (int i = 0; i < 768; i++) map_base[i] = 28;
	
	// Copy the piano to the screen
	for(int py=0; py<PIANO_HEIGHT_TILES; ++py)
	{
		memcpy(map_base + (32*(py+y/8)+(x/8)), pianoMap + (PIANO_WIDTH_TILES * py), PIANO_WIDTH_TILES * 2);
	}	
}

// Set the key corresp. to note to palette corresp. to pal_idx
void Piano::setKeyPal(u8 note)
{
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
    if(piano_hit[hit_row][px] == note)
	{
      for(py=0; py<PIANO_HEIGHT_TILES; ++py)
	  {
      	map_base[32*(py+y/8)+(px+x/8)] &= ~(3 << 12); // Clear bits 12 and 13 (from the left)
        map_base[32*(py+y/8)+(px+x/8)] |= (pal_idx << 12); // Write the pal index to bits 12 and 13
      }
    }
  }
}

// 1 for halftones, 0 for fulltones
u8 Piano::isHalfTone(u8 note)
{
	u8 i;
	for(i=0;i<10;++i) {
		if(note==halftones[i]) return 1;
	}
	return 0;
}

// Reset piano colors to normal
void Piano::resetPals(void)
{
  u8 px,py;
  for(px=0; px<PIANO_WIDTH_TILES; ++px) {
    for(py=0; py<PIANO_HEIGHT_TILES; ++py) {
      map_base[32*(py+y/8)+(px+x/8)] &= ~(3 << 12); // Clear bits 12 and 13 (from the left)
    }
  }
}

void Piano::drawKeyLabel(u8 key, bool visible)
{
	u8 xpos, ypos, offset;
	u16 col;
	
	if(isHalfTone(key) == true)
	{
		ypos = 12;
		col = theme->col_piano_label_inv;
		offset = 3;
	}
	else
	{
		ypos = 28;
		col = theme->col_piano_label;
		offset = 5;
	}
	
	if(visible == true)
		col |= BIT(15);
	
	xpos = offset + x_offsets[key % 12];
	
	if(key > 11)
		xpos += 111;
	
	char label[] = {key_labels[key], 0};
	
	drawString(label, xpos, ypos, col);
}

void Piano::eraseKeyLabel(u8 key)
{
	drawKeyLabel(key, false);
}
