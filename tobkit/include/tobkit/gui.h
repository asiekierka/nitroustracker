/*====================================================================
Copyright 2006 Tobias Weyand

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

/*******************
DS Tracker GUI Class

Contains the widgets
Gets pen/key events
Calls touched widget
********************/

#ifndef GUI_H
#define GUI_H

#include <nds.h>
#include <vector>
#include <map>
#include "widget.h"
#include "theme.h"

namespace tobkit {

static constexpr u32 MAIN_SCREEN = 0;
static constexpr u32 SUB_SCREEN = 1;

class GUI {
	public:
		GUI();

		// Sets the theme - mandatory!
		void setTheme(Theme *theme_, u16 bgcolor_);

		// Adds a widget and specifies which button it listens to
		// Touches on widget's area are redirected to the widget
		void registerWidget(Widget *w, u16 listeningButtons=0, u8 screen = SUB_SCREEN);

		// Removes a widget from the GUI
		void unregisterWidget(Widget *w);

		// Registers a widget that is in top of all other widgets and has input
		// priority, like a popup-window or something.
		void registerOverlayWidget(Widget *w, u16 listeningButtons, u8 screen = SUB_SCREEN);

		// Remove the overlay widget
		void unregisterOverlayWidget(u8 screen = SUB_SCREEN);

		// Is there an overlay widget?
		bool hasOverlayWidget(u8 screen = SUB_SCREEN);

		// Event handler for when an overlay widget is about to draw. (so we can clean up widgets underneath)
		void setOnOverlayChanged(void (*_onOverlayChanged)(u8, bool));

		// Event calls
		void penDown(u8 x, u8 y);
		void penUp(u8 x, u8 y); // Remove the coordinates here!
		void penMove(u8 x, u8 y);
		void buttonPress(u16 buttons);
		void buttonRelease(u16 buttons);

		// Draw requests
		void draw(void);
		void drawMainScreen(void);
		void drawSubScreen(void);

		// Screen switch
		void switchScreens(void);
		u8 getActiveScreen(void);

		// Show/Hide all elements
		void showAll(void);
		void hideAll(void);

		void occludeAll(void);
		void revealAll(void);

		inline std::vector<Widget*> getWidgets(u8 screen = SUB_SCREEN) {
			return screen == SUB_SCREEN ? widgets_sub : widgets_main;
		};

	private:
		std::vector<Widget*> widgets_main, widgets_sub;
		std::vector<Widget*> shortcuts;
		Widget *activeWidget;
		u8 activeScreen;
		Widget *overlayWidgetMain, *overlayWidgetSub;
		void (*onOverlayChanged)(u8, bool);
		u16 overlayShortcuts;
		Theme *theme;
		u16 bgcolor;

		// Find the widget that got hit
		Widget *getWidgetAt(u8 x, u8 y);
		Widget *getWidgetForButtons(u16 buttons);
};

};

#endif
