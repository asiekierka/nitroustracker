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
static bool screensSwapped;

#define GFX_SCREEN_MAIN (screensSwapped ? GFX_BOTTOM : GFX_TOP)
#define GFX_SCREEN_SUB (!screensSwapped ? GFX_BOTTOM : GFX_TOP)
#define GFX_EYE_MAIN (screensSwapped ? GFX_RIGHT : GFX_LEFT)
#define GFX_EYE_SUB (!screensSwapped ? GFX_RIGHT : GFX_LEFT)

bool PlatformInitFilesystem(void) {
    return true;
}

bool PlatformInit(int argc, char *argv[]) {
    gfxInit(GSP_RGB5_A1_OES, GSP_RGB5_A1_OES, false);
    gfxSetDoubleBuffering(GFX_TOP, true);
    gfxSetDoubleBuffering(GFX_BOTTOM, false);

    u16 *fb_main = (u16*) gfxGetFramebuffer(GFX_SCREEN_MAIN, GFX_EYE_MAIN, NULL, NULL);
    u16 *fb_sub = (u16*) gfxGetFramebuffer(GFX_SCREEN_SUB, GFX_EYE_SUB, NULL, NULL);

	main_screen = new Screen(fb_main, 400, 240, 240);
	sub_screen = new Screen(fb_sub, 320, 240, 240);
	screensSwapped = false;

	return true;
}

void PlatformExit(void) {
	gfxExit();
}

void PlatformFlipMainScreen(void) {
	gfxFlushBuffers();
	gfxScreenSwapBuffers(GFX_SCREEN_MAIN, false);
	main_screen->pixels = (u16*) gfxGetFramebuffer(GFX_SCREEN_MAIN, GFX_EYE_MAIN, NULL, NULL);
}

void PlatformClearMainScreen(tobkit_pixel_t color) {
	main_screen->clear(color);
	PlatformFlipMainScreen();
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
    return screensSwapped;
}

bool PlatformVideoSwapScreens(void) {
	screensSwapped = !screensSwapped;

    gfxSetDoubleBuffering(GFX_TOP, !screensSwapped);
    gfxSetDoubleBuffering(GFX_BOTTOM, screensSwapped);

    u16 *fb_main = (u16*) gfxGetFramebuffer(GFX_SCREEN_MAIN, GFX_EYE_MAIN, NULL, NULL);
    u16 *fb_sub = (u16*) gfxGetFramebuffer(GFX_SCREEN_SUB, GFX_EYE_SUB, NULL, NULL);

	main_screen->pixels = fb_main;
	sub_screen->pixels = fb_sub;

	if (screensSwapped) {
		// Draw black bars
		for (int i = 0; i < 40*240; i++) {
		    sub_screen->pixels[i] = RGB5A1_ALPHA_BIT;
		}
		for (int i = 0; i < 40*240; i++) {
		    sub_screen->pixels[i + (360 * 240)] = RGB5A1_ALPHA_BIT;
		}
		sub_screen->pixels += (40 * 240);
		main_screen->setSize(320, 240, 240);
		sub_screen->setSize(320, 240, 240);
	} else {
		main_screen->setSize(400, 240, 240);
		sub_screen->setSize(320, 240, 240);
	}

	return true;
}

PlatformKeyMask PlatformKey_LEFT = KEY_LEFT, PlatformKey_UP = KEY_UP, PlatformKey_RIGHT = KEY_RIGHT, PlatformKey_DOWN = KEY_DOWN;
PlatformKeyMask PlatformKey_A = KEY_A, PlatformKey_B = KEY_B, PlatformKey_X = KEY_X, PlatformKey_Y = KEY_Y, PlatformKey_L = KEY_L, PlatformKey_R = KEY_R;
PlatformKeyMask PlatformKey_START = KEY_START, PlatformKey_SELECT = KEY_SELECT, PlatformKey_TOUCH = KEY_TOUCH;
PlatformKeyMask PlatformKeysHeld, PlatformKeysDown, PlatformKeysUp;
u16 PlatformTouchX, PlatformTouchY;
u8 PlatformTouchScreen = TOUCH_SCREEN_BOTTOM;

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
