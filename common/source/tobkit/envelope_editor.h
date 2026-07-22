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

#ifndef ENVELOPE_EDITOR
#define ENVELOPE_EDITOR

#include "ntxm/instrument.h"
#include "tobkit/widget.h"

namespace tobkit
{

#define MAX_POINTS 12

#define POINT_WIDTH 7
#define POINT_HEIGHT 7
#define POINT_X_OFFSET (-POINT_WIDTH / 2)
#define POINT_Y_OFFSET (-POINT_HEIGHT / 2)

#define SCROLLBAR_WIDTH 9
#define SCROLLBUTTON_HEIGHT 9

#define SCROLLLEFT 1
#define SCROLLRIGHT 2
#define SCROLLTHINGY 3

#define MIN_X (-POINT_X_OFFSET + 1)
#define MAX_X (width + POINT_X_OFFSET - 2)
#define MIN_Y (-POINT_Y_OFFSET + 1)
#define MAX_Y (height + POINT_Y_OFFSET - 1 - SCROLLBAR_WIDTH)

#define ENVELOPE_MAX_ZOOM 3
#define ZOOM_BUTTONS_MARGIN (2 * SCROLLBUTTON_HEIGHT)

#define SCROLLPIXELS                                                           \
	25 // Scroll that many pixels when a scroll button is pressed
#define MIN_SCROLLTHINGY_WIDTH 15

#define DRAW_MIN_POINT_DIST 10 //15
#define DRAW_NEW_POINT_ANGLE 20

class EnvelopeEditor : public Widget
{
public:
	// Constructor sets base variables
	EnvelopeEditor(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen,
	               u16 _max_x, u16 _max_y, u16 _max_points);
	~EnvelopeEditor(void);

	// Event calls
	void penDown(u16 px, u16 py);
	void penUp(u16 px, u16 py);
	void penMove(u16 px, u16 py);

	// Drawing request
	void pleaseDraw(void);

	void registerPointsChangeCallback(void (*_onPointsChange)(void));
	void registerDrawFinishCallback(void (*_onDrawFinish)(void));

	void setPoints(u16 *xs, u16 *ys,
	               u16 n); // Sets the points to the given values
	u16 getPoints(
	    u16 **xs,
	    u16 **
	        ys); // Returns the number of points and arrays to their x and y coords
	u16 getActivePoint(void);

	void addPoint(int shift = 0);
	void delPoint(void);
	void setEditorSustainParams(bool sus, u8 suspoint);
	void setEditorLoopParams(bool enabled, u8 start_point, u8 end_point);
	void clear(void);

	void setZoomAndPos(int _zoom, int _pos);

	void startDrawMode(void);

private:
	void draw(void);

	void zoomIn(void);
	void zoomOut(void);

	void calcScrollThingy(void);
	void drawPoint(u16 x, u16 y, bool active);

	void scroll(s32 newscrollpos);

	void calcPoint(u8 idx, s16 *x,
	               s16 *y); // Calculate point position from coords
	void dispToReal(s16 dispX, s16 dispY, s16 *realX,
	                s16 *realY); // Calculate actual point from screen corrds
	void realToDisp(s16 realX, s16 realY, s16 *dispX,
	                s16 *dispY); // Calculate screen coords from real coords

	void (*onPointsChange)(void);
	void (*onDrawFinish)(void);

	bool pen_is_down;
	bool pen_on_point;

	bool pen_on_zoom_in;
	bool pen_on_zoom_out;

	u16 *points_x;
	u16 *points_y;

	u16 points_max_x;
	u16 points_max_y;

	u16 n_points;
	u16 max_points;
	u16 active_point;
	bool sustain;
	u8 sustain_point_index;
	bool loop;
	u8 loop_start_index;
	u8 loop_end_index;

	u8 zoom_level;
	u8 buttonstate;
	u8 scrollthingypos, scrollthingyheight, pen_x_on_scrollthingy;
	u32 scrollpos;

	bool draw_mode;
};

}; // namespace tobkit

#endif
