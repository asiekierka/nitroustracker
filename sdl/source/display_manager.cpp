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
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <cstdio>
#include "display_manager.h"

#ifndef SDL_SCALEMODE_PIXELART
#define SDL_SCALEMODE_PIXELART SDL_SCALEMODE_NEAREST
#endif

Screen *main_screen, *sub_screen;

static bool IsDualScreenShapePlatform() {
	// Try to detect dual-screen Android handhelds.
	if (!strcmp("Android", SDL_GetPlatform())) {
		int count = 0;
		SDL_GetDisplays(&count);
		if (count == 2) {
			return true;
		}
	}
	return false;
}

DisplayManager::DisplayManager(int width, int height, float _scale, bool multiWindow) {
	bool autoArrange = !width && !height;

	scale = _scale;
	if (scale <= 0.01f) {
		scale = 2.0f;
	}
	if (!width || !height) {
		width = 256;
		height = 192;
	}

	int widthTop = width;
	int heightTop = height;
	int widthBottom = width;
	int heightBottom = height;

	if (autoArrange) {
		// Auto-arrange windows
		if (IsDualScreenShapePlatform()) {
			// TODO
		}
	}

	main_screen = new Screen(nullptr, widthTop, heightTop, widthTop);
    sub_screen = new Screen(nullptr, widthBottom, heightBottom, widthBottom);

	if (multiWindow) {
		windowTop = SDL_CreateWindow("NitrousTracker A", (int) (scale * widthTop), (int) (scale * heightTop), 0);
		windowBottom = SDL_CreateWindow("NitrousTracker B", (int) (scale * widthBottom), (int) (scale * heightBottom), 0);
	} else {
		windowTop = SDL_CreateWindow("NitrousTracker", (int) (scale * getMaxWidth()), (int) (scale * (heightTop + heightBottom)), 0);
		windowBottom = windowTop;
	}

	// TODO: NULL checks...
	rendererTop = SDL_CreateRenderer(windowTop, nullptr);
	rendererBottom = multiWindow ? SDL_CreateRenderer(windowBottom, nullptr) : rendererTop;

	textureTop = SDL_CreateTexture(rendererTop, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, widthTop, heightTop);
	textureBottom = SDL_CreateTexture(rendererBottom, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, widthBottom, heightBottom);

	SDL_SetTextureScaleMode(textureTop, SDL_SCALEMODE_PIXELART);
	SDL_SetTextureScaleMode(textureBottom, SDL_SCALEMODE_PIXELART);

	lockScreens();
}

DisplayManager::~DisplayManager() {
	SDL_DestroyTexture(textureBottom);
	SDL_DestroyTexture(textureTop);
	if (isMultiWindow())
		SDL_DestroyRenderer(rendererBottom);
	SDL_DestroyRenderer(rendererTop);
	if (isMultiWindow())
		SDL_DestroyWindow(windowBottom);
	SDL_DestroyWindow(windowTop);
}

void DisplayManager::draw() {
	SDL_FRect src, dest;
	Screen *screen;

	// TODO: textureTop and textureBottom swapping doesn't work in multi-window mode

	unlockScreens();
	SDL_SetRenderDrawColor(rendererTop, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(rendererTop);

	screen = screensSwapped ? sub_screen : main_screen;
	src = {0, 0, (float)screen->getWidth(), (float)screen->getHeight()};
	dest = {0, 0, scale * screen->getWidth(), scale * screen->getHeight()};
	if (!isMultiWindow()) {
		dest.x += scale * ((getMaxWidth() - screen->getWidth()) / 2.0f);
	}
	SDL_RenderTexture(rendererTop, textureTop, &src, &dest);

	if (isMultiWindow()) {
		SDL_RenderPresent(rendererTop);
		SDL_SetRenderDrawColor(rendererBottom, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(rendererBottom);
	}

	int secondWindowYOffset = screen->getHeight();
	screen = !screensSwapped ? sub_screen : main_screen;
	src = {0, 0, (float)screen->getWidth(), (float)screen->getHeight()};
	dest = {0, 0, scale * screen->getWidth(), scale * screen->getHeight()};
	if (!isMultiWindow()) {
		dest.x += scale * ((getMaxWidth() - screen->getWidth()) / 2.0f);
		dest.y += scale * secondWindowYOffset;
	}
	SDL_RenderTexture(rendererBottom, textureBottom, &src, &dest);

	SDL_RenderPresent(rendererBottom);
	lockScreens();
}

bool DisplayManager::swapScreens() {
	unlockScreens();
	screensSwapped = !screensSwapped;
	lockScreens();
	return true;
}

void DisplayManager::convertTouchCoords(SDL_WindowID windowId, float x, float y) {
	bool isBottomScreen;

	x /= scale;
	y /= scale;

	if (isMultiWindow()) {
		isBottomScreen = windowId == SDL_GetWindowID(windowBottom);
	} else {
		isBottomScreen = y >= main_screen->getHeight();
		if (isBottomScreen)
			y -= main_screen->getHeight();
	}

	Screen *screen = isBottomScreen ? sub_screen : main_screen;
	if (!isMultiWindow()) {
		x -= (getMaxWidth() - screen->getWidth()) / 2.0f;
	}

	if (x >= 0.0f && y >= 0.0f && x < screen->getWidth() && y < screen->getHeight()) {
		PlatformTouchScreen = isBottomScreen ? TOUCH_SCREEN_BOTTOM : TOUCH_SCREEN_TOP;
		PlatformTouchX = x;
		PlatformTouchY = y;
	}
}

void DisplayManager::lockScreens(void) {
	SDL_Rect rectMain = {0, 0, main_screen->getWidth(), main_screen->getHeight()};
	SDL_Rect rectSub = {0, 0, sub_screen->getWidth(), sub_screen->getHeight()};
	int pitchMain, pitchSub;

	SDL_Texture *textureMain = screensSwapped ? textureBottom : textureTop;
	SDL_Texture *textureSub = !screensSwapped ? textureBottom : textureTop;

	SDL_LockTexture(textureMain, &rectMain, (void**) &main_screen->pixels, &pitchMain);
	SDL_LockTexture(textureSub, &rectSub, (void**) &sub_screen->pixels, &pitchSub);

	main_screen->setSize(textureMain->w, textureMain->h, pitchMain >> 1);
	sub_screen->setSize(textureSub->w, textureSub->h, pitchSub >> 1);
}

void DisplayManager::unlockScreens(void) {
	SDL_UnlockTexture(textureTop);
	SDL_UnlockTexture(textureBottom);
}
