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

#ifndef LABEL_H
#define LABEL_H

#include "widget.h"

namespace tobkit
{

class Label : public Widget
{
public:
	Label(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen,
	      bool _has_border = true, bool _albino = false, bool _no_bg = false,
	      bool _right_aligned = false);
	~Label(void);

	// Callback registration
	void registerPushCallback(void (*onPush_)(void));

	// Drawing request
	void pleaseDraw(void);

	// Event calls
	void penDown(u16 x, u16 y);

	void setCaption(const char *caption);
	char *getCaption(void);

private:
	void draw(void);

	void (*onPush)(void);
	char *caption;

	bool has_border;
	bool is_albino;
	bool no_bg;
	bool right_aligned;
};

}; // namespace tobkit

#endif
