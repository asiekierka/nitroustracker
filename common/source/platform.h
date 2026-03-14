/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NT_PLATFORM_H_
#define NT_PLATFORM_H_

#include "settings.h"

#include "tobkit/screen.h"
using namespace tobkit;

#ifdef __NDS__
typedef u16 PlatformKeyMask;
#else
typedef u32 PlatformKeyMask;
#endif

extern Screen *main_screen, *sub_screen;

bool PlatformInitFilesystem(void);
void PlatformInitVideo(void);
void PlatformClearMainScreen(tobkit_pixel_t color);
void PlatformClearSubScreen(tobkit_pixel_t color);
void PlatformFlipMainScreen(void);
void PlatformWaitVBlank(void);
void PlatformVideoFadeIn(void);
bool PlatformVideoAreScreensSwapped(void);
bool PlatformVideoSwapScreens(void);

extern PlatformKeyMask PlatformKey_LEFT, PlatformKey_UP, PlatformKey_RIGHT, PlatformKey_DOWN;
extern PlatformKeyMask PlatformKey_A, PlatformKey_B, PlatformKey_X, PlatformKey_Y, PlatformKey_L, PlatformKey_R;
extern PlatformKeyMask PlatformKey_TOUCH;
extern PlatformKeyMask PlatformKeysDown, PlatformKeysUp, PlatformKeysHeld;
extern u16 PlatformTouchX, PlatformTouchY;

void PlatformSetInputLayout(Handedness handedness);
void PlatformInputUpdate(void);
void PlatformInputSetRepeat(int delay, int rate_delay);

#endif
