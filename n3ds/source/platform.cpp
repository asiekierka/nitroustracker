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
#include <3ds.h>

bool PlatformVideoInit(void);
void PlatformVideoExit(void);

bool PlatformInitFilesystem(void)
{
	return true;
}

bool PlatformInit(int argc, char *argv[])
{
	romfsInit();
	if (!PlatformVideoInit())
		return false;
	return true;
}

void PlatformExit(void)
{
	PlatformVideoExit();
	romfsExit();
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
	PlatformKeysDown =
	    hidKeysDown() | (hidKeysDownRepeat() & keys_that_are_repeated);
	PlatformKeysUp = hidKeysUp();
	PlatformKeysHeld = hidKeysHeld();

	touchPosition touch;
	hidTouchRead(&touch);
	PlatformTouchX = touch.px;
	PlatformTouchY = touch.py;
}

void PlatformInputSetRepeat(int delay, int rate_delay)
{
	hidSetRepeatParameters(delay, rate_delay);
}
