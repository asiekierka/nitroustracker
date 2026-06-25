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

#ifndef PIANO_H
#define PIANO_H

#include "widget.h"

namespace tobkit {

static constexpr int PIANO_WIDTH_TILES = 28;
static constexpr int PIANO_HEIGHT_TILES = 5;

class Piano: public Widget {
	public:
		Piano(u16 _x, u16 _y, u16 _width, u16 _height, u16 *_char_base, u16 *_screen_base, Screen *_screen);
		virtual ~Piano();

		// Drawing request
		void pleaseDraw(void);

		// Event calls
		void penDown(u16 px, u16 py);
		void penUp(u16 px, u16 py);
		void penMove(u16 px, u16 py);

		// Callback registration
		void registerNoteCallback(void (*onNote_)(u8));
		void registerReleaseCallback(void (*onRelease_)(u8, bool));

		// Key label handling
		void showKeyLabels(void);
		void hideKeyLabels(void);
		void setKeyLabel(u8 key, char label);
		void setInMappingMode(bool instmap);
		void setTheme(Theme *theme_, u16 bgcolor_);
		void show(void);
		int getKeyCount(void) const;

	private:
		void (*onNote)(u8);
		void (*onRelease)(u8, bool);
		u16 *char_base, *map_base;

		unsigned short piano_Palette[16], piano_fullnotehighlight_Palette[16], piano_halfnotehighlight_Palette[16];

		void draw(void);
		void setKeyPal(u8 note);
		int getKeyXOffset(int key) const;
		bool isSharpNote(u8 note) const;
		void resetPals(void);
		void genPal(u16 *piano_cols_base, u16 *pal, u16 *pal_full_highlight, u16 *pal_half_highlight);
		void drawKeyLabel(u8 key, bool visible=true);
		void eraseKeyLabel(u8 key);

		char *key_labels;
		bool key_labels_visible;
		bool mapping_instrument;
		u16 curr_note;

};

};

#endif
