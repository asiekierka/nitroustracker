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

#include <3ds.h>
#include "platform.h"

Screen *main_screen, *sub_screen;

bool PlatformInitFilesystem(void) {
    return true;
}

bool PlatformInit(void) {
	csndInit();

    gfxInit(GSP_RGB5_A1_OES, GSP_RGB5_A1_OES, false);
    gfxSetDoubleBuffering(GFX_TOP, false);
    gfxSetDoubleBuffering(GFX_BOTTOM, false);

    u16 *fb_main_l = (u16*) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    u16 *fb_sub = (u16*) gfxGetFramebuffer(GFX_BOTTOM, GFX_RIGHT, NULL, NULL);

	main_screen = new Screen(fb_main_l, 400, 240, 240);
	sub_screen = new Screen(fb_sub, 320, 240, 240);

	return true;
}

void PlatformExit(void) {
	gfxExit();

	csndExit();
}

void PlatformFlipMainScreen(void) {
}

void PlatformClearMainScreen(tobkit_pixel_t color) {
	main_screen->clear(color);
}

void PlatformClearSubScreen(tobkit_pixel_t color) {
	sub_screen->clear(color);
}

bool PlatformWaitVBlank(void) {
	if (!aptMainLoop()) return false;
	gfxFlushBuffers();
    gspWaitForVBlank();
	return true;
}

void PlatformVideoFadeIn(void) {

}

bool PlatformVideoAreScreensSwapped(void) {
    return false; // TODO
}

bool PlatformVideoSwapScreens(void) {
    return false; // TODO
}

PlatformKeyMask PlatformKey_LEFT = KEY_LEFT, PlatformKey_UP = KEY_UP, PlatformKey_RIGHT = KEY_RIGHT, PlatformKey_DOWN = KEY_DOWN;
PlatformKeyMask PlatformKey_A = KEY_A, PlatformKey_B = KEY_B, PlatformKey_X = KEY_X, PlatformKey_Y = KEY_Y, PlatformKey_L = KEY_L, PlatformKey_R = KEY_R;
PlatformKeyMask PlatformKey_TOUCH = KEY_TOUCH;
PlatformKeyMask PlatformKeysHeld, PlatformKeysDown, PlatformKeysUp;
u16 PlatformTouchX, PlatformTouchY;

static PlatformKeyMask keys_that_are_repeated = KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT;

void PlatformSetInputLayout(Handedness handedness)
{
	if(handedness == LEFT_HANDED)
	{
		PlatformKey_UP = KEY_X;
		PlatformKey_DOWN = KEY_B;
		PlatformKey_LEFT = KEY_Y;
		PlatformKey_RIGHT = KEY_A;
		PlatformKey_L = KEY_R;
		PlatformKey_R = KEY_L;
		PlatformKey_A = KEY_RIGHT;
		PlatformKey_B = KEY_DOWN;
		PlatformKey_X = KEY_UP;
		PlatformKey_Y = KEY_LEFT;
		keys_that_are_repeated = KEY_A | KEY_B | KEY_X | KEY_Y;
	}
	else
	{
		PlatformKey_UP = KEY_UP;
		PlatformKey_DOWN = KEY_DOWN;
		PlatformKey_LEFT = KEY_LEFT;
		PlatformKey_RIGHT = KEY_RIGHT;
		PlatformKey_L = KEY_L;
		PlatformKey_R = KEY_R;
		PlatformKey_A = KEY_A;
		PlatformKey_B = KEY_B;
		PlatformKey_X = KEY_X;
		PlatformKey_Y = KEY_Y;
		keys_that_are_repeated = KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT;
	}
}

void PlatformInputUpdate(void) {
    scanKeys();
	PlatformKeysDown = hidKeysDown() | (hidKeysDownRepeat() & keys_that_are_repeated);
    PlatformKeysUp = hidKeysUp();
	PlatformKeysHeld = hidKeysHeld();

    touchPosition touch;
    hidTouchRead(&touch);
    PlatformTouchX = touch.px;
    PlatformTouchY = touch.py;
}

void PlatformInputSetRepeat(int delay, int rate_delay) {
	hidSetRepeatParameters(delay, rate_delay);
}