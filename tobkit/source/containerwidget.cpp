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

#include "tobkit/containerwidget.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

ContainerWidget::ContainerWidget(u16 _x, u16 _y, u16 _width, u16 _height,
                                 Screen *_screen, bool _visible)
    : Widget(_x, _y, _width, _height, _screen, _visible)
{
}

// Event calls
void ContainerWidget::penDown(u16 px, u16 py)
{
	currentGui()->penDown(px, py);
}

void ContainerWidget::penUp(u16 px, u16 py)
{
	currentGui()->penUp(px, py);
}

void ContainerWidget::penMove(u16 px, u16 py)
{
	currentGui()->penMove(px, py);
}

void ContainerWidget::buttonPress(u16 buttons)
{
	currentGui()->buttonPress(buttons);
}

// Drawing request
void ContainerWidget::show(void)
{
	Widget::show();
	currentGui()->showAll();
}

void ContainerWidget::hide(void)
{
	Widget::hide();
	currentGui()->hideAll();
}

void ContainerWidget::occlude(void)
{
	Widget::occlude();
	currentGui()->occludeAll();
}

void ContainerWidget::reveal(void)
{
	Widget::reveal();
	currentGui()->revealAll();
}

void ContainerWidget::draw(void)
{
	currentGui()->draw();
}
