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

#include <string.h>
#include <stdio.h>

#include "tobkit/widget.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

Screen::Screen(tobkit_pixel_t *_pixels, u32 _width, u32 _height, u32 _pitch)
    :pixels(_pixels), width(_width), height(_height), pitch(_pitch)
{

}

void Screen::clear(tobkit_pixel_t col) {
#if defined(__NDS__)
	u32 colcol = col * 0x10001;
	dmaFillWords(colcol, pixels, 192*256*2);
#else
    for (int iy = 0; iy < height; iy++)
        for (int ix = 0; ix < width; ix++)
            pixels[iy*pitch+ix] = col;
#endif
}