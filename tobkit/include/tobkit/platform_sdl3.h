/*====================================================================
Copyright 2025 Adrian "asie" Siekierka

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

#ifndef _PLATFORM_SDL3_H_
#define _PLATFORM_SDL3_H_

#include "ntxm/common.h"
#include <SDL3/SDL.h>

typedef u16 tobkit_pixel_t;

#define RGB5A1(r, g, b, a) (((b) << 10) | ((g) << 5) | (r) | ((a) << 15))
#define RGB5A1_ALPHA_BIT 0x8000
#define RGB5A1_R(c) ((c) & 0x1F)
#define RGB5A1_G(c) (((c) >> 5) & 0x1F)
#define RGB5A1_B(c) (((c) >> 10) & 0x1F)

#define KEY_UP (1 << 0)
#define KEY_LEFT (1 << 1)
#define KEY_RIGHT (1 << 2)
#define KEY_DOWN (1 << 3)
#define KEY_A (1 << 4)
#define KEY_B (1 << 5)
#define KEY_X (1 << 6)
#define KEY_Y (1 << 7)
#define KEY_L (1 << 8)
#define KEY_R (1 << 9)
#define KEY_START (1 << 10)
#define KEY_SELECT (1 << 11)
#define KEY_TOUCH (1 << 12)

#endif
