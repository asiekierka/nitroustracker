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

#include "platform.h"
#include <fat.h>
#include <nds.h>

#define FRONT_BUFFER 0
#define BACK_BUFFER 1
static u8 active_buffer = FRONT_BUFFER;

static u16 *main_vram_front, *main_vram_back, *sub_vram;
Screen *main_screen, *sub_screen;

bool PlatformInitFilesystem(void)
{
	return fatInitDefault();
}

bool PlatformInit(int argc, char *argv[])
{
	// Hide everything
#ifndef DEBUG
	setBrightness(3, 16);
#endif

	powerOn(POWER_ALL_2D);

	// Adjust screens so that the main screen is the top screen
	lcdMainOnTop();

	// Main screen: Text and double buffer ERB
	videoSetMode(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);

	// Sub screen: Keyboard tiles, Typewriter tiles and ERB
	videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE |
	                DISPLAY_BG2_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D);

	vramSetPrimaryBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000,
	                    VRAM_C_SUB_BG_0x06200000, VRAM_D_SUB_SPRITE);

	// SUB_BG0 for Piano Tiles
	videoBgEnableSub(0);
	int piano_bg = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 1, 0);
	bgSetScroll(piano_bg, 0, 0);
	bgSetPriority(piano_bg, 2);

	// SUB_BG1 for Typewriter Tiles
	videoBgEnableSub(1);
	int typewriter_bg = bgInitSub(1, BgType_Text4bpp, BgSize_T_256x256, 12, 1);
	bgSetPriority(typewriter_bg, 0);

#ifdef DEBUG
	u8 text_priority = 0;
	u8 bg_priority = 1;
#else
	u8 text_priority = 1;
	u8 bg_priority = 0;
#endif

	// Pattern view
	int ptn_bg = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 2, 0);
	bgSetPriority(ptn_bg, bg_priority);

	// Sub screen framebuffer
	int sub_bg = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 2, 0);
	bgSetPriority(sub_bg, 1);

	oamInit(&oamSub, SpriteMapping_1D_32, false);

	// Create a window the same size as the sample display. (so we can occclude loop handles, etc)
	windowEnableSub(WINDOW_0);

	bgWindowEnable(sub_bg, (WINDOW)(WINDOW_0 | WINDOW_OUT));
	bgWindowEnable(piano_bg, (WINDOW)(WINDOW_0 | WINDOW_OUT));
	bgWindowEnable(typewriter_bg, (WINDOW)(WINDOW_0 | WINDOW_OUT));

	windowSetBoundsSub(WINDOW_0, 5, 24, 5 + 131,
	                   23 + 61); // sampledisplay x1,y1,x2,y2
	oamWindowEnable(&oamSub, WINDOW_0);

	// Special effects
#ifdef DEBUG
	REG_BLDCNT = BLEND_ALPHA | BLEND_SRC_BG0 | BLEND_DST_BG2;
	REG_BLDALPHA = 0x040C;
#else
	REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG2 | BLEND_SRC_BG0;
	REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG2 | BLEND_SRC_BG0;
#endif

	// Setup text
	PrintConsole *console = consoleInit(NULL, 0, BgType_Text4bpp,
	                                    BgSize_T_256x256, 4, 0, true, true);
#ifdef DEBUG
	consoleArm7Setup(console, 1024);
#endif
	bgSetPriority(0, text_priority);

	bgUpdate();

	// Initialize VRAM pointers
	main_vram_front = (u16 *)BG_BMP_RAM(2);
	main_vram_back = (u16 *)BG_BMP_RAM(8);
	sub_vram = (u16 *)BG_BMP_RAM_SUB(2);

	main_screen = new Screen(main_vram_back, 256, 192, 256);
	sub_screen = new Screen(sub_vram, 256, 192, 256);

	// Clear tile mem
	dmaFillWords(0, BG_BMP_RAM_SUB(0), 32 * 1024);

	return true;
}

void PlatformExit(void)
{
}

void PlatformFlipMainScreen(void)
{
	// Flip buffers
	active_buffer = !active_buffer;

	if (active_buffer == FRONT_BUFFER) {
		bgSetMapBase(2, 2);
		main_vram_front = (u16 *)BG_BMP_RAM(2);
		main_vram_back = (u16 *)BG_BMP_RAM(8);
	} else {
		bgSetMapBase(2, 8);
		main_vram_front = (u16 *)BG_BMP_RAM(8);
		main_vram_back = (u16 *)BG_BMP_RAM(2);
	}
	main_screen->pixels = main_vram_back;
}

void PlatformDrawSubScreen(void)
{
}

void PlatformClearMainScreen(tobkit_pixel_t color)
{
	main_screen->pixels = main_vram_front;
	main_screen->clear(color);
	main_screen->pixels = main_vram_back;
	main_screen->clear(color);
}

void PlatformClearSubScreen(tobkit_pixel_t color)
{
	sub_screen->clear(color);
}

bool PlatformWaitVBlank(void)
{
	cothread_yield_irq(IRQ_VBLANK);
	return true;
}

void PlatformVideoFadeIn(void)
{
	for (int i = -16; i <= 0; ++i) {
		setBrightness(3, i);
		PlatformWaitVBlank();
	}
}

bool PlatformVideoAreScreensSwapped(void)
{
	return !(REG_POWERCNT & POWER_SWAP_LCDS);
}

bool PlatformVideoSwapScreens(void)
{
	lcdSwap();
	return true;
}

PlatformKeyMask PlatformKey_LEFT = KEY_LEFT, PlatformKey_UP = KEY_UP,
                PlatformKey_RIGHT = KEY_RIGHT, PlatformKey_DOWN = KEY_DOWN;
PlatformKeyMask PlatformKey_A = KEY_A, PlatformKey_B = KEY_B,
                PlatformKey_X = KEY_X, PlatformKey_Y = KEY_Y,
                PlatformKey_L = KEY_L, PlatformKey_R = KEY_R;
PlatformKeyMask PlatformKey_START = KEY_START, PlatformKey_SELECT = KEY_SELECT,
                PlatformKey_TOUCH = KEY_TOUCH;
PlatformKeyMask PlatformKeysHeld, PlatformKeysDown, PlatformKeysUp;
u16 PlatformTouchX, PlatformTouchY;
u8 PlatformTouchScreen = TOUCH_SCREEN_BOTTOM;

static PlatformKeyMask keys_that_are_repeated =
    KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT;

void PlatformSetInputLayout(Handedness handedness)
{
	if (handedness == LEFT_HANDED) {
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
	} else {
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

void PlatformInputUpdate(void)
{
	scanKeys();
	PlatformKeysDown = keysDown() | (keysDownRepeat() & keys_that_are_repeated);
	PlatformKeysUp = keysUp();
	PlatformKeysHeld = keysHeld();

	touchPosition touch;
	touchRead(&touch);
	PlatformTouchX = touch.px;
	PlatformTouchY = touch.py;
}

void PlatformInputSetRepeat(int delay, int rate_delay)
{
	keysSetRepeat(delay, rate_delay);
}
