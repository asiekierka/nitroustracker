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
#include "display_manager.h"
#include <SDL3/SDL.h>
#include <cstdlib>
#include <unistd.h>

static DisplayManager *display;

bool PlatformInitFilesystem(void)
{
	return true;
}

bool PlatformInit(int argc, char *argv[])
{
	int width = 0;
	int height = 0;
	float scale = 0.0f;
	bool multiWindow = false;

	int c;
	while ((c = getopt(argc, argv, "H:MS:W:")) >= 0) {
		switch (c) {
		case 'W': width = atoi(optarg); break;
		case 'H': height = atoi(optarg); break;
		case 'S': scale = atof(optarg); break;
		case 'M': multiWindow = true; break;
		}
	}

	SDL_SetAppMetadata("NitrousTracker", VERSION, "pl.asie.nitroustracker");

	if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
		return false;
	}

	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

	display = new DisplayManager(width, height, scale, multiWindow);

	return true;
}

void PlatformExit(void)
{
	delete display;

	SDL_Quit();
}

void PlatformFlipMainScreen(void)
{
}

void PlatformClearMainScreen(tobkit_pixel_t color)
{
	main_screen->clear(color);
}

void PlatformClearSubScreen(tobkit_pixel_t color)
{
	sub_screen->clear(color);
}

bool PlatformWaitVBlank(void)
{
	display->draw();

	PlatformKeysDown = 0;
	PlatformKeysUp = 0;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// TODO: multi-window simultaneous touches are not handled
		switch (event.type) {
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			PlatformKeysDown |= PlatformKey_TOUCH;
			display->convertTouchCoords(event.button.windowID, event.button.x,
			                            event.button.y);
			break;
		case SDL_EVENT_MOUSE_MOTION:
			display->convertTouchCoords(event.motion.windowID, event.motion.x,
			                            event.motion.y);
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			PlatformKeysUp |= PlatformKey_TOUCH;
			PlatformTouchX = 0;
			PlatformTouchY = 0;
			break;
		case SDL_EVENT_QUIT: return false;
		}
	}

	PlatformKeysHeld |= PlatformKeysDown;
	PlatformKeysHeld &= ~PlatformKeysUp;

	return true;
}

void PlatformVideoFadeIn(void)
{
}

bool PlatformVideoAreScreensSwapped(void)
{
	return display->getScreensSwapped();
}

bool PlatformVideoSwapScreens(void)
{
	return display->swapScreens();
}

PlatformKeyMask PlatformKey_LEFT = KEY_LEFT, PlatformKey_UP = KEY_UP,
                PlatformKey_RIGHT = KEY_RIGHT, PlatformKey_DOWN = KEY_DOWN;
PlatformKeyMask PlatformKey_A = KEY_A, PlatformKey_B = KEY_B,
                PlatformKey_X = KEY_X, PlatformKey_Y = KEY_Y,
                PlatformKey_L = KEY_L, PlatformKey_R = KEY_R;
PlatformKeyMask PlatformKey_START = KEY_START, PlatformKey_SELECT = KEY_SELECT,
                PlatformKey_TOUCH = KEY_TOUCH;
PlatformKeyMask PlatformKeysHeld = 0, PlatformKeysDown = 0, PlatformKeysUp = 0;
u16 PlatformTouchX, PlatformTouchY;
u8 PlatformTouchScreen;

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
}

void PlatformInputSetRepeat(int delay, int rate_delay)
{
}
