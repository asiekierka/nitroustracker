/*====================================================================
Copyright (c) 2026 Adrian "asie" Siekierka

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

#ifndef SCREEN_H
#define SCREEN_H

#include <limits.h>

#include "theme.h"
#include "platform.h"

namespace tobkit {

class Screen {
	public:
		Screen(tobkit_pixel_t *_pixels, int _width, int _height, int _pitch);
		~Screen(void) {}

        void clear(tobkit_pixel_t col);
        void setSize(int _width, int _height, int _pitch);

        inline int getWidth(void) const { return width; }
        inline int getHeight(void) const { return height; }
        inline int getPitch(void) const {
#if defined(TOBKIT_CONSTANT_PITCH)
            return TOBKIT_CONSTANT_PITCH;
#else
            return pitch;
#endif
        }

		inline void drawPixel(u32 tx, u32 ty, tobkit_pixel_t col) {
#if defined(NT_PLATFORM_3DS)
            *(pixels+getPitch()*tx+getPitch()-1-ty) = col;
#else
            *(pixels+getPitch()*ty+tx) = col;
#endif
        }

        inline void fillRow(u32 tx, u32 ty, u32 bw, u32 col) {
#if defined(NT_PLATFORM_NDS)
    		dmaFillHalfWords(col, pixels+getPitch()*ty+tx, bw*2);
#else
            for (u32 i = 0; i < bw; i++)
                drawPixel(tx+i, ty, col);
#endif
        }

        inline void fillColumn(u32 tx, u32 ty, u32 bh, u32 col) {
#if defined(NT_PLATFORM_3DS)
			u32 offset = ty+bh;
			if (!(offset & 1)) {
				u32 colcol = col * 0x10001;
				__ndsabi_wordset4(pixels+getPitch()*tx+getPitch()-offset, bh*2, colcol);
				return;
			}
#endif
            for (u32 i = 0; i < bh; i++)
                drawPixel(tx, ty+i, col);
        }

        tobkit_pixel_t *pixels;

    private:
        int width, height, pitch;
};

};

#endif
