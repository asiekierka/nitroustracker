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
		Screen(tobkit_pixel_t *_pixels, u32 _width, u32 _height, u32 _pitch);
		~Screen(void) {}

        void clear(tobkit_pixel_t col);

		inline void drawPixel(u32 tx, u32 ty, tobkit_pixel_t col) {
			*(pixels+pitch*ty+tx) = col;
		}

        inline void fillRow(u32 tx, u32 ty, u32 bw, u32 col) {
#if defined(TOBKIT_PLATFORM_NDS)
    		dmaFillHalfWords(col, pixels+pitch*ty+tx, bw*2);
#else
            for (u32 i = 0; i < bw; i++)
                drawPixel(tx+i, ty, col);
#endif
        }

        tobkit_pixel_t *pixels;

    private:
        u32 width, height, pitch;
};

};

#endif
