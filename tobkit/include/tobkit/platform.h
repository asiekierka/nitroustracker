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

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#if defined(NT_PLATFORM_NDS)
#include "platform_nds.h"
#elif defined(NT_PLATFORM_3DS)
#include "platform_n3ds.h"
#elif defined(NT_PLATFORM_SDL3)
#include "platform_sdl3.h"
#else
#error No platform defined!
#endif

#if !defined(NT_PLATFORM_NDS)
#define ITCM_CODE
#define div32(a,b) ((a)/(b))
#endif

#endif
