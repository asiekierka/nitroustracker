/*
 * NitroTracker - An FT2-style tracker for the Nintendo DS
 *
 *                                by Tobias Weyand (0xtob)
 *
 * http://nitrotracker.tobw.net
 * http://code.google.com/p/nitrotracker
 */

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

#include "envelope_editor.h"

#include <cstdio>
#include <cstring>
#include <cmath>

#include "ntxm/ntxmtools.h"
#include "../tools.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

// Constructor sets base variables
EnvelopeEditor::EnvelopeEditor(u8 _x, u8 _y, u8 _width, u8 _height, u16 **_vram, u16 _max_x, u16 _max_y, u16 _max_points)
	:Widget(_x, _y, _width, _height, _vram), onPointsChange(0), onDrawFinish(0),
	pen_is_down(false), pen_on_point(false), points_max_x(_max_x), points_max_y(_max_y), n_points(0),
	max_points(_max_points), active_point(0), sustain(false), sustain_point_index(0), zoom_level(0), buttonstate(0), scrollthingypos(0),
	scrollthingyheight(width-2*SCROLLBUTTON_HEIGHT+2), pen_x_on_scrollthingy(0), scrollpos(0), draw_mode(false)
{
	points_x = (u16*)calloc(1, _max_points * sizeof(u16));
	points_y = (u16*)calloc(1, _max_points * sizeof(u16));
}

EnvelopeEditor::~EnvelopeEditor(void)
{
	free(points_x);
	free(points_y);
}

// Event calls
void EnvelopeEditor::penDown(u8 px, u8 py)
{
	if(!enabled)
		return;

	s16 penX = px - x;
	s16 penY = py - y;

	if(!draw_mode)
	{
		pen_on_point = false;

		if( penY < height - SCROLLBAR_WIDTH )
		{
			for(int p = 0; p < n_points; ++p)
			{
				s16 pointX, pointY;
				realToDisp(points_x[p], points_y[p], &pointX, &pointY);

				if(	    (penX >= pointX + POINT_X_OFFSET )
					&& (penX <= pointX + POINT_X_OFFSET + POINT_WIDTH )
					&& (penY >= pointY + POINT_Y_OFFSET )
					&& (penY <= pointY + POINT_Y_OFFSET + POINT_HEIGHT )
				)
				{
					active_point = p;
					pen_on_point = true;
				}
			}
		}
		else if(penY >= height - SCROLLBAR_WIDTH)
		{
			if(penX <= SCROLLBUTTON_HEIGHT)
			{
				buttonstate = SCROLLLEFT;

				scroll(-SCROLLPIXELS);
			}
			else if(penX >= width - SCROLLBUTTON_HEIGHT)
			{
				buttonstate = SCROLLRIGHT;

				scroll(SCROLLPIXELS);
			}
			else if(penX < scrollthingypos + SCROLLBUTTON_HEIGHT)
			{
				scroll(-width+2);
			}
			else if(penX > scrollthingypos + SCROLLBUTTON_HEIGHT + scrollthingyheight)
			{
				scroll(width-2);
			}
			else
			{
				buttonstate = SCROLLTHINGY;
				pen_x_on_scrollthingy = penX-SCROLLBUTTON_HEIGHT-scrollthingypos;
			}
		}
	}
	else
	{
		// Draw mode
		penY = my_clamp(penY, MIN_Y, MAX_Y);
		penX = my_clamp(penX, MIN_X, MAX_X);

		s16 realX, realY;
		dispToReal(penX, penY, &realX, &realY);

		points_x[0] = realX;
		points_y[0] = realY;
		n_points++;
	}

	draw();
}

void EnvelopeEditor::penUp(u8 px, u8 py)
{
	if(draw_mode == true)
	{
		draw_mode = false;

		// Create a point at x=0 if necessary
		if(points_x[0] > 0)
		{
			// If the 1st point is close, just stick it to the left
			if(points_x[0] < 30)
			{
				points_x[0] = 0;
			}
			// Else make a new 1st point
			else
			{
				for(u8 i=n_points; i>0; --i)
				{
					points_x[i] = points_x[i-1];
					points_y[i] = points_y[i-1];
				}
				points_x[0] = 0;
				points_y[0] = points_y[1];
				n_points++;
			}
		}

		if(onPointsChange != 0)
			onPointsChange();

		if(onDrawFinish != 0)
			onDrawFinish();
	}

	buttonstate = 0;
	draw();
}

void EnvelopeEditor::penMove(u8 px, u8 py)
{
	s16 penX = px - x;
	s16 penY = py - y;

	if(!draw_mode)
	{
		if( pen_on_point == true )
		{
			s16 pointX, pointY;

			penY = my_clamp(penY, MIN_Y, MAX_Y);
			penX = my_clamp(penX, MIN_X, MAX_X);

			dispToReal(penX, penY, &pointX, &pointY);

			if(	   ( active_point != 0 )
				&& ( (active_point == n_points - 1) || (pointX < points_x[active_point+1]) )
				&& ( pointX > points_x[active_point-1] )
			)
			{
				points_x[active_point] = my_clamp(pointX, 0, points_max_x);
			}

			points_y[active_point] = my_clamp(pointY, 0, points_max_y);

			if(onPointsChange != 0)
				onPointsChange();
		}
		else if(buttonstate == SCROLLTHINGY)
		{
			scrollthingypos = my_clamp(penX - pen_x_on_scrollthingy - SCROLLBUTTON_HEIGHT, 0, width - 2*SCROLLBUTTON_HEIGHT+2 - scrollthingyheight);

			u32 window_width = width - 2;
			u32 disp_width = window_width << zoom_level;
			u32 scroll_width = width - 2*SCROLLBUTTON_HEIGHT+2 - scrollthingyheight;
			scrollpos = scrollthingypos * (disp_width - window_width) / scroll_width;
		}

	}
	else // Draw Mode
	{
		s16 realX, realY;
		dispToReal(penX, penY, &realX, &realY);
		realX = my_clamp(realX, points_x[n_points-1]+1, points_max_x);
		realY = my_clamp(realY, 0, points_max_y);

		// If there's only one point yet, check if we are FIRST_DIST points away from it
		if(n_points == 1)
		{
			int sqdist = (realX - points_x[0]) * (realX - points_x[0])
					+ (realY - points_y[0]) * (realY - points_y[0]);

			if(sqdist >= DRAW_MIN_POINT_DIST * DRAW_MIN_POINT_DIST)
			{
				// Add a point
				points_x[n_points] = realX;
				points_y[n_points] = realY;
				n_points++;
			}

		} else {

			// Check if the angle between the current and the last line is greater than NEW_POINT_ANGLE
			s32 last_dx =  points_x[n_points-1] - points_x[n_points-2];
			s32 last_dy =  points_y[n_points-1] - points_y[n_points-2];

			s32 dx = realX - points_x[n_points-1];
			s32 dy = realY - points_y[n_points-1];

			int32 len = sqrtf32(inttof32(dx*dx + dy*dy)); /* 20.12 */

			if( (f32toint(len) > DRAW_MIN_POINT_DIST) || (realY == points_max_y) || (realY == 0) )
			{
				int32 last_len = sqrtf32(inttof32(last_dx*last_dx + last_dy*last_dy)); /* 20.12 */
				s16 angle = angleToDegrees(acosLerp(divf32(inttof32(last_dx*dx + last_dy*dy), mulf32(last_len, len))));

				if (angle < DRAW_NEW_POINT_ANGLE)
				{
					// Take the point with you
					points_x[n_points-1] = realX;
					points_y[n_points-1] = realY;
				}
				else if( (realY != points_max_y) && (realY != 0) && (n_points < MAX_POINTS - 1) )
				{
					// Add a point
					points_x[n_points] = realX;
					points_y[n_points] = realY;
					n_points++;
				}
			}

		}

	}

	draw();
}

// Drawing request
void EnvelopeEditor::pleaseDraw(void)
{
	draw();
}

void EnvelopeEditor::registerPointsChangeCallback(void (*_onPointsChange)(void))
{
	onPointsChange = _onPointsChange;
}

void EnvelopeEditor::registerDrawFinishCallback(void (*_onDrawFinish)(void))
{
	onDrawFinish = _onDrawFinish;
}

void EnvelopeEditor::setPoints(u16 *xs, u16 *ys, u16 n) // Sets the points to the given values
{
	memcpy(points_x, xs, n*sizeof(u16));
	memcpy(points_y, ys, n*sizeof(u16));
	n_points = n;

	active_point = 0;

	buttonstate = 0;

	if(onPointsChange != 0)
		onPointsChange();
}

u16 EnvelopeEditor::getPoints(u16 **xs, u16 **ys) // Returns the number of points and arrays to their x and y coords
{
	*xs = points_x;
	*ys = points_y;

	return n_points;
}

u16 EnvelopeEditor::getActivePoint(void)
{
  return active_point;
}

void EnvelopeEditor::addPoint(void)
{
	if(n_points == max_points)
		return;

	n_points++;

	for(int p = n_points - 1; p > active_point; --p)
	{
		points_x[p] = points_x[p-1];
		points_y[p] = points_y[p-1];
	}

	if( n_points == 1 ) // First point added
	{
		points_x[active_point] = 0;
		points_y[active_point] = points_max_y;
	}
	// Add the point between this point and the next one
	else if(active_point < n_points - 2)
	{
		points_x[active_point+1] = ( points_x[active_point] +
				points_x[active_point+2] ) / 2;
		points_y[active_point+1] = ( points_y[active_point] +
				points_y[active_point+2] ) / 2;
	}
	else // Add a point after the last point
	{
		points_x[active_point+1] = my_clamp(points_x[active_point] + ((50) >> zoom_level), 0, points_max_x);
		points_y[active_point+1] = points_y[active_point];
	}

	if(onPointsChange != 0)
		onPointsChange();

	if(n_points > 1)
		active_point++;

	draw();
}

void EnvelopeEditor::delPoint(void)
{
	if( ((n_points > 1) && (active_point != 0)) || (n_points == 1) )
	{
		for(int p = active_point; p < n_points - 1; ++p)
		{
			points_x[p] = points_x[p+1];
			points_y[p] = points_y[p+1];
		}
		n_points--;

		if(active_point >= n_points)
			active_point = n_points - 1;

		if(onPointsChange != 0)
			onPointsChange();

		draw();
	}
}

void EnvelopeEditor::toggleSustain(bool is_enabled)
{
  sustain = is_enabled;  
}

void EnvelopeEditor::setEditorSustainParams(bool sus, u8 sus_point)
{
  sustain = sus;
  sustain_point_index = sus_point;
}

void EnvelopeEditor::clear(void)
{
	if (n_points != 0)
	{
		// Delete all points
		n_points = 0;

		if(onPointsChange != 0)
			onPointsChange();
	}
}

void EnvelopeEditor::zoomIn(void)
{
	if(zoom_level < ENVELOPE_MAX_ZOOM)
	{
		zoom_level++;
		calcScrollThingy();
		scrollthingypos += scrollthingyheight / 2;

		scrollpos = scrollthingypos;
		for(u8 i = 0; i < zoom_level; ++i)
			scrollpos *= 2;

		draw();
	}
}

void EnvelopeEditor::zoomOut(void)
{
	if(zoom_level > 0)
	{
		zoom_level--;
		scrollthingypos = my_clamp(scrollthingypos - scrollthingyheight / 2, 0, width - 2*SCROLLBUTTON_HEIGHT);
		calcScrollThingy();
		scrollthingypos = my_clamp(scrollthingypos, 0, width - 2*SCROLLBUTTON_HEIGHT+2 - scrollthingyheight);

		scrollpos = scrollthingypos;
		for(u8 i=0; i < zoom_level; ++i)
			scrollpos *= 2;

		draw();
	}
}

void EnvelopeEditor::setZoomAndPos(int _zoom, int _pos)
{
	zoom_level = _zoom;
	calcScrollThingy();
	scrollthingypos = _pos;
	scrollpos = scrollthingypos;
	for(u8 i=0; i < zoom_level; ++i)
			scrollpos *= 2;
}

void EnvelopeEditor::startDrawMode(void)
{
	clear();

	// Init Draw mode
	draw_mode = true;

	draw();
}

/* ===================== PRIVATE ===================== */

void EnvelopeEditor::draw(void)
{
	if(!isExposed())
		return;

	//
	// Border and background
	//
	drawFullBox(1, 1, width - 2, height - 2, theme->col_env_bg);
	drawBorder(theme->col_outline);

	if( (draw_mode == true) && (n_points == 0) )
	{
		drawString("draw the envelope", 10, 20, theme->col_lighter_bg, 255);
		drawString("with the stylus", 20, 30, theme->col_lighter_bg, 255);
	}

	//
	// Points
	//

	s16 last_point_x = 0, last_point_y = 0, point_x = 0, point_y = 0;

	if(n_points > 0)
	{
		realToDisp(points_x[0], points_y[0], &last_point_x, &last_point_y);

		if( (last_point_x >= MIN_X) && (last_point_x <= MAX_X) )
			drawPoint(last_point_x, last_point_y, 0 == active_point);
	}

	for(u8 idx=1; idx < n_points; ++idx )
	{
		realToDisp(points_x[idx], points_y[idx], &point_x, &point_y);

		// If at least one of the points is inside
		if( ! ( ( (point_x < 1) && ( last_point_x < 1 ) ) || ( (point_x > width-1) && (last_point_x > width-1) ) ) )
		{
			u8 line_y1, line_y2, line_x1, line_x2;

			// If the left point is outside
			if(last_point_x  < 0)
			{
				// Clip the left point
				line_y1 = ( last_point_y*point_x - last_point_x*point_y ) / ( point_x - last_point_x );
				line_x1 = 0;
			} else {
				line_y1 = last_point_y;
				line_x1 = last_point_x;
			}

			// If the right point is outside
			if(point_x > width-1)
			{
				// Clip the right point
				line_y2 = (last_point_y*point_x + (width-1)*point_y - last_point_x*point_y -
						width*last_point_y) / (point_x - last_point_x);
				line_x2 = width-1;
			} else {
				line_y2 = point_y;
				line_x2 = point_x;
			}


			// Draw the line
			if (sustain == true)
			{
			  if (idx == (sustain_point_index + 1))
			  {
			    drawBresLine(last_point_x, last_point_y, last_point_x, MAX_Y, theme->col_env_sustain);
			  }
			}
			drawBresLine(line_x1, line_y1, line_x2, line_y2, theme->col_env_line);

			// Display unclipped points
			if( (last_point_x >= MIN_X) && (last_point_x <= MAX_X) )
				drawPoint(last_point_x, last_point_y, idx - 1 == active_point);

		}
		last_point_x = point_x;
		last_point_y = point_y;
	}

	if( (n_points > 0) && (point_x >= MIN_X) && (point_x <= MAX_X) )
		drawPoint(point_x, point_y, n_points - 1 == active_point);


	//
	// Scrollbar
	//

	// Right Button
	if(buttonstate == SCROLLRIGHT) {
		drawGradient(theme->col_scrollbar_arr_bg1, theme->col_scrollbar_arr_bg2, width-9, height-SCROLLBAR_WIDTH+1, 8, 8);
	} else {
		drawGradient(theme->col_scrollbar_arr_bg2, theme->col_scrollbar_arr_bg1, width-9, height-SCROLLBAR_WIDTH+1, 8, 8);
	}

	// This draws the right-arrow
	s8 j, p;
	for(j=0;j<3;j++) {
		for(p=-j;p<=j;++p) {
			*(*vram+SCREEN_WIDTH*(y+height-SCROLLBAR_WIDTH+4+p)+x+width-j-3) = theme->col_icon_bt;
		}
	}

	drawBox(width-SCROLLBAR_WIDTH, height-SCROLLBUTTON_HEIGHT, 9, 9, theme->col_outline);

	// Left Button
	if(buttonstate==SCROLLLEFT) {
		drawGradient(theme->col_scrollbar_arr_bg1, theme->col_scrollbar_arr_bg2, 1, height-9, 8, 8);
	} else {
		drawGradient(theme->col_scrollbar_arr_bg2, theme->col_scrollbar_arr_bg1, 1, height-9, 8, 8);
	}

	// This draws the down-arrow
	for(j=2;j>=0;j--) {
		for(p=-j;p<=j;++p) {
			*(*vram+SCREEN_WIDTH*(y+height-SCROLLBAR_WIDTH+4+p)+x+j+3) = theme->col_icon_bt;
		}
	}

	drawBox(0, height-9, 9, 9, theme->col_outline);


	drawBox(0, height-SCROLLBAR_WIDTH, width, SCROLLBAR_WIDTH, theme->col_outline);

	// Clear Scrollbar
	drawGradient(theme->col_scrollbar_bg1, theme->col_scrollbar_bg2, SCROLLBUTTON_HEIGHT, height-SCROLLBAR_WIDTH+1, width-2*SCROLLBUTTON_HEIGHT, SCROLLBAR_WIDTH-2);

	// The scroll thingy
	if(buttonstate==SCROLLTHINGY) {
		drawFullBox(SCROLLBUTTON_HEIGHT+scrollthingypos, height-SCROLLBAR_WIDTH+1, scrollthingyheight-2, SCROLLBAR_WIDTH-2, theme->col_scrollbar_active);
	} else {
		drawFullBox(SCROLLBUTTON_HEIGHT+scrollthingypos, height-SCROLLBAR_WIDTH+1, scrollthingyheight-2, SCROLLBAR_WIDTH-2, theme->col_scrollbar_inactive);
	}

	drawBox(SCROLLBUTTON_HEIGHT-1+scrollthingypos, height-SCROLLBAR_WIDTH, scrollthingyheight, SCROLLBAR_WIDTH, theme->col_outline);
}

// Calculate height and position of the scroll thingy
void EnvelopeEditor::calcScrollThingy(void)
{
	u16 sch = width - 2 * SCROLLBUTTON_HEIGHT + 2;

	scrollthingyheight = sch >> zoom_level;
}

void EnvelopeEditor::drawPoint(u8 x, u8 y, bool active)
{
	drawFullBox(x + POINT_X_OFFSET, y + POINT_Y_OFFSET, POINT_WIDTH, POINT_HEIGHT, theme->col_env_pt);

	if(!active)
		drawBox(x + POINT_X_OFFSET, y + POINT_Y_OFFSET, POINT_WIDTH, POINT_HEIGHT, theme->col_env_pt_border);
	else
		drawBox(x + POINT_X_OFFSET, y + POINT_Y_OFFSET, POINT_WIDTH, POINT_HEIGHT, theme->col_env_pt_border_active);
}

void EnvelopeEditor::scroll(s32 difference)
{
	u32 window_width = width - 2;
	u32 disp_width = window_width << zoom_level;
	u32 scroll_width = width - 2*SCROLLBUTTON_HEIGHT+2 - scrollthingyheight;

	scrollpos = my_clamp(scrollpos + difference, 0, disp_width - window_width);
	scrollthingypos = scrollpos * scroll_width / (disp_width - window_width);

	calcScrollThingy();
	draw();
}

void EnvelopeEditor::dispToReal(s16 dispX, s16 dispY, s16 *realX, s16 *realY) // Calculate actual point from screen coords
{
	u32 dispwidth = MAX_X - MIN_X;
	dispwidth <<= zoom_level;
	u32 dispheight = MAX_Y - MIN_Y;

	s32 xpos = dispX - MIN_X + scrollpos;
	s32 ypos = dispheight - (dispY - MIN_Y);

	if(xpos < 0) xpos = 0;
	if(ypos < 0) ypos = 0;

	*realX = xpos * points_max_x / dispwidth;
	*realY = ypos * points_max_y / dispheight;
}

void EnvelopeEditor::realToDisp(s16 realX, s16 realY, s16 *dispX, s16 *dispY) // Calculate screen coords from real coords
{
	u32 dispwidth = MAX_X - MIN_X;
	dispwidth <<= zoom_level;
	u32 dispheight = MAX_Y - MIN_Y;

	s32 xpos = realX * dispwidth / points_max_x;
	s32 ypos = realY * dispheight / points_max_y;

	*dispX = xpos - scrollpos + MIN_X;
	*dispY = dispheight - ypos + MIN_Y;
}
