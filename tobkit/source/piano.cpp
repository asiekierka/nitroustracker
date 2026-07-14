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

#ifdef NT_PLATFORM_NDS
#include "piano.h"
#endif
#include "piano_hit.h"

using namespace tobkit;

#define clamp(v, vmin, vmax)                                                   \
	(((v) < (vmin)) ? (vmin) : ((v > (vmax)) ? (vmax) : (v)))

static const u8 halfkeys[5] = {1, 3, 6, 8, 10};
static const bool fullkeyFlag[12] = {true,  false, true,  false, true,  true,
                                     false, true,  false, true,  false, true};
static const u8 fullkeys[7] = {0, 2, 4, 5, 7, 9, 11};
static const u8 fullkeysDrawn[7] = {0, 1, 3, 5, 6, 8, 10};
static const u8 fullkeyOffset[12] = {0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6};

/* ===================== PUBLIC ===================== */
Piano::Piano(u16 _x, u16 _y, u16 _width, u16 _height, u16 *_char_base,
             u16 *_map_base, Screen *_screen)
    : Widget(_x, _y, _width, _height, _screen),
#ifdef NT_PLATFORM_NDS
      char_base(_char_base), map_base(_map_base),
#endif
      key_labels_visible(false), mapping_instrument(false), curr_note(255)
{
	onNote = 0;
	onRelease = 0;

#ifdef NT_PLATFORM_NDS
	// Piano::draw is never invoked before Piano::setTheme
	dmaCopy(pianoTiles, char_base, sizeof(pianoTiles));
#endif

	key_labels = (char *)ntxm_cmalloc(getKeyCount());
	memset(key_labels, ' ', getKeyCount());
}

Piano::~Piano()
{
	ntxm_free(key_labels);
}

void Piano::setTheme(Theme *theme_, u16 bgcolor_)
{
#ifdef NT_PLATFORM_NDS
	u16 piano_cols[9] = {
	    theme_->col_piano_full_col1,
	    theme_->col_piano_full_col2,
	    theme_->col_piano_half_col1,
	    theme_->col_piano_half_col2,
	    theme_->col_piano_full_highlight_col1,
	    theme_->col_piano_full_highlight_col2,
	    theme_->col_piano_half_highlight_col1,
	    theme_->col_piano_half_highlight_col2,
	    theme_->col_piano_outline
	};
	genPal(piano_cols, piano_Palette, piano_fullnotehighlight_Palette,
	       piano_halfnotehighlight_Palette);
#endif
	Widget::setTheme(theme_, bgcolor_);

#ifdef NT_PLATFORM_NDS
	// doesn't conflict with fx keyboard, so safe to write these
	memcpy(BG_PALETTE_SUB + 16, piano_fullnotehighlight_Palette, 32);
	memcpy(BG_PALETTE_SUB + 32, piano_halfnotehighlight_Palette, 32);
#endif

	if (!isExposed())
		return;

#ifdef NT_PLATFORM_NDS
	memcpy(BG_PALETTE_SUB, piano_Palette, 32);
#endif

	setInMappingMode(mapping_instrument);
	pleaseDraw();
}

void Piano::show(void)
{
#ifdef NT_PLATFORM_NDS
	dmaCopy(pianoTiles, char_base, sizeof(pianoTiles));
	memcpy(BG_PALETTE_SUB, piano_Palette, 32);
#endif

	Widget::show();
}

// Drawing request
void Piano::pleaseDraw(void)
{
	draw();
}

int Piano::getKeyXOffset(int key) const
{
#ifdef NT_PLATFORM_NDS
	int octaveOffset = 111;
#else
	int octaveOffset = 112;
#endif
	return (fullkeyOffset[key % 12] * 16) + ((key / 12) * octaveOffset) +
	       (isSharpNote(key) ? 11 : 0);
}

int Piano::getKeyCount(void) const
{
	int rows = width / 112;
	int keys = fullkeysDrawn[(width % 112) >> 4];
	return rows * 12 + keys;
}

// Event calls
void Piano::penDown(u16 px, u16 py)
{
	// Look up the note in the hit-array
	s16 kbx, kby;
	kbx = clamp((px - x), 0, width - 1) / 8;
	kby = clamp((py - y), 0, height - 1) / 8;

	u8 note = piano_hit[kby][kbx % 14] + ((kbx / 14) * 12);

	curr_note = note;
	drawOnKeyPressChange(curr_note, true);

	if (onNote) {
		onNote(note);
	}
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
		drawOnKeyPressChange(curr_note, false);

		if (onRelease) {
			onRelease(curr_note, true);
		}

		curr_note = note;
		drawOnKeyPressChange(curr_note, true);

		if (onNote) {
			onNote(note);
		}
	}
}

void Piano::penUp(u16 px, u16 py)
{
	drawOnKeyPressChange(curr_note, false);

	if (onRelease) {
		onRelease(curr_note, false);
	}

	curr_note = 255;
}

// Callback registration
void Piano::registerNoteCallback(void (*onNote_)(u8))
{
	onNote = onNote_;
}

void Piano::registerReleaseCallback(void (*onRelease_)(u8, bool))
{
	onRelease = onRelease_;
}

// Key label handling
void Piano::showKeyLabels(void)
{
	if (key_labels_visible) {
		return;
	}

	key_labels_visible = true;

	for (int key = 0; key < getKeyCount(); ++key)
		drawKeyLabel(key);
}

void Piano::hideKeyLabels(void)
{
	if (!key_labels_visible) {
		return;
	}

	key_labels_visible = false;

#ifdef NT_PLATFORM_NDS
	for (int key = 0; key < getKeyCount(); ++key)
		eraseKeyLabel(key);
#else
	draw();
#endif
}

void Piano::setInMappingMode(bool instmap)
{
	mapping_instrument = instmap;
	u16 col = theme->col_signal & ~RGB5A1_ALPHA_BIT;
	if (!instmap) {
		drawHLine(0, height - 1, width, theme->col_piano_half_col1);
		drawVLine(0, 1, height - 1, theme->col_piano_half_col2);
		drawVLine(width - 1, 1, height - 1, theme->col_piano_half_col1);
	}
	drawBox(0, 1, width, height - 1,
	        mapping_instrument ? col | RGB5A1_ALPHA_BIT : col);
}

void Piano::setKeyLabel(u8 key, char label)
{
#ifdef NT_PLATFORM_NDS
	if (key_labels_visible) {
		eraseKeyLabel(key);
	}
#endif

	if (key_labels[key] == label) {
		return;
	}

	key_labels[key] = label;

	if (key_labels_visible) {
#ifdef NT_PLATFORM_NDS
		drawKeyLabel(key);
#else
		drawKey(key, curr_note == key);
#endif
	}
}

/* ===================== PRIVATE ===================== */

void Piano::genPal(u16 *piano_cols_base, u16 *pal, u16 *pal_full_highlight,
                   u16 *pal_half_highlight)
{
	for (int i = 0; i < 9; ++i) {
		pal[i] = interpolateColor(piano_cols_base[3], piano_cols_base[2],
		                          (4096 / 8) * i + 1);
		pal_half_highlight[i] = interpolateColor(
		    piano_cols_base[6], piano_cols_base[7], (4096 / 9) * i + 1);

		if (i > 6)
			continue;

		pal[i + 9] = interpolateColor(piano_cols_base[0], piano_cols_base[1],
		                              (4096 / 7) * i);
		pal_full_highlight[i + 9] = interpolateColor(
		    piano_cols_base[4], piano_cols_base[5], (4096 / 7) * i);
	}

	memcpy(&pal_full_highlight[0], &pal[0], 9 * sizeof(u16));
	memcpy(&pal_half_highlight[9], &pal[9], 7 * sizeof(u16));

	pal[0] = piano_cols_base[8];
}

void Piano::drawOnKeyPressChange(u8 key, bool pressed)
{
#ifdef NT_PLATFORM_NDS
	if (pressed)
		setKeyPal(key);
	else
		resetKeyPals();
#else
	drawKey(key, pressed);
#endif
}

#ifndef NT_PLATFORM_NDS
void Piano::drawKey(int key, bool pressed, bool onlyKey)
{
	if (key >= getKeyCount())
		return;

	int subkey = key % 12;
	int key_draw_x = getKeyXOffset(key);

	if (fullkeyFlag[subkey]) {
		const u8 FULLKEY_WIDTH = 15;
		const u8 FULLKEY_HEIGHT = 38;

		u16 col1 = !pressed ? theme->col_piano_full_col1
		                    : theme->col_piano_full_highlight_col1;
		u16 col2 = !pressed ? theme->col_piano_full_col2
		                    : theme->col_piano_full_highlight_col2;

		drawFullBox(1 + key_draw_x, 1, FULLKEY_WIDTH, FULLKEY_HEIGHT, col2);
		drawHorizontalGradient(col2, col1, 2 + key_draw_x, 2, FULLKEY_WIDTH - 2,
		                       FULLKEY_HEIGHT - 2);

		if (!onlyKey) {
			// draw overlapping half-keys
			if (subkey > 0 && !fullkeyFlag[subkey - 1])
				drawKey(key - 1, curr_note == (key - 1));
			if (subkey < 11 && !fullkeyFlag[subkey + 1])
				drawKey(key + 1, curr_note == (key + 1));
		}
	} else {
		const u8 HALFKEY_WIDTH = 10;
		const u8 HALFKEY_HEIGHT = 23;

		u16 col1 = !pressed ? theme->col_piano_half_col1
		                    : theme->col_piano_half_highlight_col2;
		u16 col2 = !pressed ? theme->col_piano_half_col2
		                    : theme->col_piano_half_highlight_col1;

		drawFullBox(1 + key_draw_x, 1, HALFKEY_WIDTH, HALFKEY_HEIGHT, col2);
		drawGradient(col2, col1, 2 + key_draw_x, 2, HALFKEY_WIDTH - 2,
		             HALFKEY_HEIGHT - 2);
	}

	if (key_labels_visible)
		drawKeyLabel(key);
}
#endif

void Piano::draw(void)
{
#ifdef NT_PLATFORM_NDS
	// Fill screen with empty tiles
	for (int i = 0; i < 768; i++)
		map_base[i] = 28;

	// Copy the piano to the screen
	for (int py = 0; py < PIANO_HEIGHT_TILES; ++py) {
		memcpy(map_base + (32 * (py + y / 8) + (x / 8)),
		       pianoMap + (PIANO_WIDTH_TILES * py), PIANO_WIDTH_TILES * 2);
	}
#else
	if (!isExposed()) {
		drawFullBox(0, 0, width, height, theme->col_bg);
	} else {
		drawFullBox(0, 1, width, height - 1, theme->col_piano_outline);
		for (int i = 0; i < getKeyCount(); i++)
			if (fullkeyFlag[i % 12])
				drawKey(i, curr_note == i, true);
		for (int i = 0; i < getKeyCount(); i++)
			if (!fullkeyFlag[i % 12])
				drawKey(i, curr_note == i, true);
	}
#endif
}

#ifdef NT_PLATFORM_NDS
// Reset piano colors to normal
void Piano::resetKeyPals(void)
{
	u8 px, py;
	for (px = 0; px < PIANO_WIDTH_TILES; ++px) {
		for (py = 0; py < PIANO_HEIGHT_TILES; ++py) {
			map_base[32 * (py + y / 8) + (px + x / 8)] &=
			    ~(3 << 12); // Clear bits 12 and 13 (from the left)
		}
	}
}

// Set the key corresp. to note to palette corresp. to pal_idx
void Piano::setKeyPal(u8 note)
{
	u8 px, py, hit_row, pal_idx;

	if (isSharpNote(note)) {
		hit_row = 0;
		pal_idx = 2;
	} else {
		hit_row = 4;
		pal_idx = 1;
	}

	for (px = 0; px < PIANO_WIDTH_TILES; ++px) {
		if ((piano_hit[hit_row][px % 14] + ((px / 14) * 12)) == note) {
			for (py = 0; py < PIANO_HEIGHT_TILES; ++py) {
				map_base[32 * (py + y / 8) + (px + x / 8)] &=
				    ~(3 << 12); // Clear bits 12 and 13 (from the left)
				map_base[32 * (py + y / 8) + (px + x / 8)] |=
				    (pal_idx << 12); // Write the pal index to bits 12 and 13
			}
		}
	}
}
#endif

// 1 for halftones, 0 for fulltones
bool Piano::isSharpNote(u8 note) const
{
	return !fullkeyFlag[note % 12];
}

void Piano::drawKeyLabel(u8 key, bool visible)
{
	int xpos, ypos, offset;
	u16 col;

#ifndef NT_PLATFORM_NDS
	if (!visible)
		return;
#endif

	if (isSharpNote(key)) {
		ypos = 12;
		col = theme->col_piano_label_inv;
		offset = 3;
	} else {
		ypos = 28;
		col = theme->col_piano_label;
		offset = 5;
	}

	if (visible)
		col |= RGB5A1_ALPHA_BIT;
	else
		col &= ~RGB5A1_ALPHA_BIT;

	xpos = offset + getKeyXOffset(key);

	char label[] = {key_labels[key], 0};

	drawString(label, xpos, ypos, col);
}

void Piano::eraseKeyLabel(u8 key)
{
	drawKeyLabel(key, false);
}
