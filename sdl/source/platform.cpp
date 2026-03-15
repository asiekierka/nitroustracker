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

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include "platform.h"

Screen *main_screen, *sub_screen;
static bool screensSwapped;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *textureTop = NULL, *textureBottom = NULL;

bool PlatformInitFilesystem(void) {
    return true;
}

#ifndef SDL_SCALEMODE_PIXELART
#define SDL_SCALEMODE_PIXELART SDL_SCALEMODE_NEAREST
#endif

static void lock_screens(void) {
	SDL_Rect rectTop = {0, 0, 256, 192};
	SDL_Rect rectBottom = {0, 0, 256, 192};
	int pitchTop, pitchBottom;

	SDL_LockTexture(textureTop, &rectTop, (void**) &main_screen->pixels, &pitchTop);
	SDL_LockTexture(textureBottom, &rectBottom, (void**) &sub_screen->pixels, &pitchBottom);

	main_screen->setSize(textureTop->w, textureTop->h, pitchTop >> 1);
	sub_screen->setSize(textureBottom->w, textureBottom->h, pitchBottom >> 1);
}

static void unlock_screens(void) {
	SDL_UnlockTexture(textureTop);
	SDL_UnlockTexture(textureBottom);
}

bool PlatformInit(void) {
	SDL_SetAppMetadata("NitrousTracker", VERSION, "pl.asie.nitroustracker");

	if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
		return false;
	}

	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

	window = SDL_CreateWindow("NitrousTracker", 256, 384, 0);
	if (window == NULL) {
		return false;
	}

	renderer = SDL_CreateRenderer(window, NULL);
	if (renderer == NULL) {
		return false;
	}

	textureTop = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, 256, 192);
	if (textureTop == NULL) {
		return false;
	}

	textureBottom = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, 256, 192);
	if (textureBottom == NULL) {
		return false;
	}

	SDL_SetTextureScaleMode(textureTop, SDL_SCALEMODE_PIXELART);
	SDL_SetTextureScaleMode(textureBottom, SDL_SCALEMODE_PIXELART);

	main_screen = new Screen(NULL, 256, 192, 256);
	sub_screen = new Screen(NULL, 256, 192, 256);

	lock_screens();

	return true;
}

void PlatformExit(void) {
	SDL_DestroyTexture(textureBottom);
	SDL_DestroyTexture(textureTop);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void PlatformFlipMainScreen(void) {
}

void PlatformClearMainScreen(tobkit_pixel_t color) {
	main_screen->clear(color);
}

void PlatformClearSubScreen(tobkit_pixel_t color) {
	sub_screen->clear(color);
}

void update_touch_coords(float x, float y) {
	if (y >= 192 && y < 384 && x >= 0 && x < 256) {
		PlatformTouchX = x;
		PlatformTouchY = y - 192;
	} else {
		PlatformTouchX = 0;
		PlatformTouchY = 0;
	}
}

bool PlatformWaitVBlank(void) {
	SDL_FRect src, dest;

	unlock_screens();
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	src = {0, 0, 256, 192};
	dest = {0, 0, 256, 192};
	SDL_RenderTexture(renderer, textureTop, &src, &dest);

	src = {0, 0, 256, 192};
	dest = {0, 192, 256, 192};
	SDL_RenderTexture(renderer, textureBottom, &src, &dest);

	SDL_RenderPresent(renderer);
	lock_screens();

	PlatformKeysDown = 0;
	PlatformKeysUp = 0;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				PlatformKeysDown |= PlatformKey_TOUCH;
				update_touch_coords(event.button.x, event.button.y);
				break;
			case SDL_EVENT_MOUSE_MOTION:
				update_touch_coords(event.motion.x, event.motion.y);
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				PlatformKeysUp |= PlatformKey_TOUCH;
				update_touch_coords(-1, -1);
				break;
			case SDL_EVENT_QUIT:
				return false;
		}
	}

	PlatformKeysHeld |= PlatformKeysDown;
	PlatformKeysHeld &= ~PlatformKeysUp;

	return true;
}

void PlatformVideoFadeIn(void) {

}

bool PlatformVideoAreScreensSwapped(void) {
    return screensSwapped;
}

bool PlatformVideoSwapScreens(void) {
	return false;
}

PlatformKeyMask PlatformKey_LEFT = KEY_LEFT, PlatformKey_UP = KEY_UP, PlatformKey_RIGHT = KEY_RIGHT, PlatformKey_DOWN = KEY_DOWN;
PlatformKeyMask PlatformKey_A = KEY_A, PlatformKey_B = KEY_B, PlatformKey_X = KEY_X, PlatformKey_Y = KEY_Y, PlatformKey_L = KEY_L, PlatformKey_R = KEY_R;
PlatformKeyMask PlatformKey_START = KEY_START, PlatformKey_SELECT = KEY_SELECT, PlatformKey_TOUCH = KEY_TOUCH;
PlatformKeyMask PlatformKeysHeld = 0, PlatformKeysDown = 0, PlatformKeysUp = 0;
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
}

void PlatformInputSetRepeat(int delay, int rate_delay) {
}