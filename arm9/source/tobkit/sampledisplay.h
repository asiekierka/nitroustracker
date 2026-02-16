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

#ifndef _SAMPLEDISPLAY_H_
#define _SAMPLEDISPLAY_H_

#include "tobkit/widget.h"
#include "ntxm/sample.h"

namespace tobkit {

#define LOOP_TRIANGLE_SIZE				8
#define SNAP_TO_ZERO_CROSSING_RADIUS	32 // Search 32 samples to the left and to the right to find a zero crossing

#define SCROLLBAR_WIDTH				9
#define SCROLLBUTTON_HEIGHT			9

#define SAMPLE_MAX_ZOOM					16

#define SCROLLPIXELS				25 // Scroll that many pixels when a scroll button is pressed
#define MIN_SCROLLTHINGY_WIDTH		15
#define DRAW_HEIGHT					(height-SCROLLBUTTON_HEIGHT-3) // height of the visible window

class SampleDisplay: public Widget {
	public:
		// Constructor sets base variables
		SampleDisplay(u8 _x, u8 _y, u8 _width, u8 _height, u16 **_vram, Sample *_smp=0);
		~SampleDisplay(void);

		// Event calls
		void penDown(u8 px, u8 py);
		void penUp(u8 px, u8 py);
		void penMove(u8 px, u8 py);

		// Drawing request
		void pleaseDraw(void);
		void requestRedraw(void);
		void reveal(void);
		
		void setSample(Sample *_smp, u8 smpidx=0, u8 instidx=255);
		void select_all(void);
		void clear_selection(void);

		void calcCursor();
		void setCursorPosPtr(SampleCursor *cursorpos);
		void hideCursor(void);
		void showCursor(void);

		// Return start and end sample of selection
		bool getSelection(u32 *startsample, u32 *endsample);

		// Functions to toggle "active" mode, i.e. the mode in which you can make selections
		void setActive(void);
		void setInactive(void);

		void setDrawMode(bool _on);

		void showLoopPoints(void);
		void hideLoopPoints(void);

		void setSnapToZeroCrossing(bool snap);

	private:
		// Finds a zero corssing in the sample near pos, returns sample when successful, -1 else
		long find_zero_crossing_near(long pos);

		void draw(void);
		void scroll(u32 newscrollpos);
		void calcScrollThingy(void);
		void zoomIn(void);
		void zoomOut(void);
		u32 pixelToSample(s32 pixel);
		s32 sampleToPixel(u32 sample);

		Sample *smp;
		u8 smpidx, instidx;
		u32 selstart, selend;
		bool selection_exists;

		bool pen_is_down;

		bool active;
		bool loop_points_visible;

		bool pen_on_loop_start_point;
		bool pen_on_loop_end_point;
		s16 loop_touch_offset;

		bool pen_on_zoom_in;
		bool pen_on_zoom_out;

		bool pen_on_scroll_left;
		bool pen_on_scroll_right;
		bool pen_on_scrollthingy;
		bool pen_on_scrollbar;

		u8 scrollthingypos, scrollthingywidth, pen_x_on_scrollthingy;
		u8 zoom_level;
		u64 scrollpos;

		bool snap_to_zero_crossings;

		bool draw_mode;
		bool cursor_draw_mode;
		bool do_redraw;

		u8 draw_last_x, draw_last_y;

		SampleCursor *cursorpos;
		u32 cursors_xpos[16] = {0};
};

};

#endif
