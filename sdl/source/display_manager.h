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

#ifndef SDL_DISPLAY_MANAGER_H_
#define SDL_DISPLAY_MANAGER_H_

#include "platform.h"
#include <SDL3/SDL_video.h>

extern Screen *main_screen, *sub_screen;

class DisplayManager {
public:
	DisplayManager(int width, int height, float scale, bool multiWindow);
	~DisplayManager();

	void draw();
	bool swapScreens();
	void convertTouchCoords(SDL_WindowID windowId, float x, float y);

	inline float getScale() const { return scale; }
	inline bool getScreensSwapped() const { return screensSwapped; }
	inline bool isMultiWindow() const { return windowTop != windowBottom; }

private:
	bool screensSwapped = false;
	float scale = 1.0f;
	SDL_Window *windowTop = NULL;
	SDL_Window *windowBottom = NULL;
	SDL_Renderer *rendererTop = NULL;
	SDL_Renderer *rendererBottom = NULL;
	SDL_Texture *textureTop = NULL;
	SDL_Texture *textureBottom = NULL;

	inline int getMaxWidth() const { return main_screen->getWidth() > sub_screen->getWidth() ? main_screen->getWidth() : sub_screen->getWidth(); }
	inline int getMaxHeight() const { return main_screen->getHeight() > sub_screen->getHeight() ? main_screen->getHeight() : sub_screen->getHeight(); }
	void lockScreens();
	void unlockScreens();
};

#endif /* SDL_DISPLAY_MANAGER_H_ */
