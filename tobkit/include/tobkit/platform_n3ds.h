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

#ifndef _PLATFORM_N3DS_H_
#define _PLATFORM_N3DS_H_

#include <3ds.h>

#define TOBKIT_CONSTANT_PITCH 240

// For printf() and scanf() defines
#include "ntxm/common.h"

typedef u16 tobkit_pixel_t;

#define RGB5A1(r, g, b, a) (((r) << 11) | ((g) << 6) | ((b) << 1) | (a))
#define RGB5A1_ALPHA_BIT 0x1
#define RGB5A1_R(c) (((c) >> 11) & 0x1F)
#define RGB5A1_G(c) (((c) >> 6) & 0x1F)
#define RGB5A1_B(c) (((c) >> 1) & 0x1F)

#ifdef __cplusplus
extern "C" {
#endif
void __ndsabi_wordset4(void *dest, size_t n, int c);
#ifdef __cplusplus
};
#endif

#endif
