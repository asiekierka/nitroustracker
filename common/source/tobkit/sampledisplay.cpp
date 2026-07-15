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

#include "sampledisplay.h"

#define sqrtf32 unused_sqrtf32
#include <cmath>
#undef sqrtf32
#include <cstdio>

#ifdef NT_PLATFORM_NDS
#include "loophandle.h"
#else
#include "libnds/math.h"
#endif
#include "ntxm/ntxmtools.h"
#include "tools.h"

#define ZOOM_BUTTON_SIZE 11
#define ZOOM_BUTTON_CENTER (((ZOOM_BUTTON_SIZE) + 1) >> 1)

using namespace tobkit;

/* ===================== PUBLIC ===================== */

// Constructor sets base variables
SampleDisplay::SampleDisplay(u16 _x, u16 _y, u16 _width, u16 _height,
                             Screen *_screen, Sample *_smp)
    : Widget(_x, _y, _width, _height, _screen), smp(_smp), selstart(0),
      selend(0), selection_exists(false), pen_is_down(false), active(false),
      loop_points_visible(false), pen_on_loop_start_point(false),
      pen_on_loop_end_point(false), pen_on_zoom_in(false),
      pen_on_zoom_out(false), pen_on_scroll_left(false),
      pen_on_scroll_right(false), pen_on_scrollthingy(false),
      pen_on_scrollbar(false), scrollthingypos(0),
      scrollthingywidth(width - 2 * SCROLLBUTTON_HEIGHT - ZOOM_BUTTONS_MARGIN +
                        2),
      pen_x_on_scrollthingy(0), zoom_level(0), scrollpos(0),
      offset_guide_pos(-1), snap_to_zero_crossings(true), draw_mode(false)
{
#ifdef NT_PLATFORM_NDS
	gfxLine =
	    oamAllocateGfx(&oamSub, SpriteSize_16x32, SpriteColorFormat_16Color);
	gfxLoopHandle =
	    oamAllocateGfx(&oamSub, SpriteSize_8x8, SpriteColorFormat_16Color);

	// draw a single pixel then use max y-scale to make a line
	*(gfxLine + 64) = 1;
	oamSub.oamRotationMemory[0].vdy = 0;

	for (int i = SPR_LOOPHANDLE_1_L; i <= SPR_LOOPHANDLE_2_R; ++i) {
		bool top = i > 17;
		bool right = i % 2 == 1;

		oamSet(&oamSub, i, x + width + 8, top ? y : SAMPLE_DRAW_HEIGHT + 18,
		       0, // priority 0
		       1, // pallette index 1
		       SpriteSize_8x8, SpriteColorFormat_16Color, gfxLoopHandle, -1,
		       false, false, right,
		       top, // h-flip every other handle, v-flip the last two handles
		       false);
	}

	memcpy(gfxLoopHandle, loophandleTiles, loophandleTilesLen);

	for (int i = SPR_LOOPLINE_1; i <= SPR_LOOPLINE_2; ++i) {
		oamSet(&oamSub, i, 199, y, 0, 0, SpriteSize_16x32,
		       SpriteColorFormat_16Color, gfxLine, 0, true, false, false, false,
		       false);
	}
#endif
}

SampleDisplay::~SampleDisplay(void)
{
#ifdef NT_PLATFORM_NDS
	oamFreeGfx(&oamSub, gfxLine);
	oamFreeGfx(&oamSub, gfxLoopHandle);
#endif
}

void SampleDisplay::penDown(u16 px, u16 py)
{
	if ((smp == 0) || ((active == false) && (loop_points_visible == false) &&
	                   (draw_mode == false)))
		return;

	// Stylus on the scroll bar?
	if (py - y >= height - SCROLLBAR_WIDTH) {
		if (px - x <= SCROLLBUTTON_HEIGHT) {
			pen_on_scroll_left = true;
			scroll(scrollpos - SCROLLPIXELS);
		} else if (px - x >= width - SCROLLBUTTON_HEIGHT) {
			pen_on_zoom_out = true;
			zoomOut();
		} else if (px - x >= width - 2 * SCROLLBUTTON_HEIGHT) {
			pen_on_zoom_in = true;
			zoomIn();
		} else if (px - x >= width - 3 * SCROLLBUTTON_HEIGHT) {
			pen_on_scroll_right = true;
			scroll(scrollpos + SCROLLPIXELS);
		} else if (px - x < scrollthingypos + SCROLLBUTTON_HEIGHT) {
			scroll(scrollpos - width + 2);
		} else if (px - x >
		           scrollthingypos + SCROLLBUTTON_HEIGHT + scrollthingywidth) {
			scroll(scrollpos + width - 2);
		} else {
			pen_on_scrollthingy = true;
			pen_x_on_scrollthingy =
			    px - x - SCROLLBUTTON_HEIGHT - scrollthingypos;
		}
		pen_on_scrollbar = true;
	} else if (draw_mode) {
		draw_last_x = px - x;
		draw_last_y = py - y;
	} else {
		// Stylus on a loop point?
		u32 loop_start_pos = sampleToPixel(
		    smp->getLoopStart()); //smp->getLoopStart() * (width-2) / smp->getNSamples();
		u32 loop_end_pos = sampleToPixel(
		    smp->getLoopStart() +
		    smp->getLoopLength()); //(smp->getLoopStart() + smp->getLoopLength()) * (width-2) / smp->getNSamples();

		if (isInRect(
		        px - x - 1, py - y - 1,
		        std::max((s32)0, (s32)loop_start_pos - (s32)LOOP_TRIANGLE_SIZE),
		        SAMPLE_DRAW_HEIGHT - LOOP_TRIANGLE_SIZE,
		        loop_start_pos + LOOP_TRIANGLE_SIZE, SAMPLE_DRAW_HEIGHT)) {
			pen_on_loop_start_point = true;
			loop_touch_offset = px - x - 1 - loop_start_pos;
		} else if (isInRect(px - x, py - y,
		                    std::max((s32)0, (s32)loop_end_pos -
		                                         (s32)LOOP_TRIANGLE_SIZE),
		                    0, loop_end_pos + LOOP_TRIANGLE_SIZE,
		                    LOOP_TRIANGLE_SIZE)) {
			pen_on_loop_end_point = true;
			loop_touch_offset = px - x - 1 - loop_end_pos;
		}

		// Else: Stylus on the sample.
		else if (active) {
			selstart = selend = pixelToSample(px - x - 1);
			selection_exists = false;
			draw();
		}
	}
}

void SampleDisplay::penUp(u16 px, u16 py)
{
	// Swap selstart and selend if they are in the wrong order
	if (selend < selstart) {
		u32 tmp = selstart;
		selstart = selend;
		selend = tmp;
	}

	if (pen_on_zoom_in || pen_on_zoom_out || pen_on_scroll_left ||
	    pen_on_scroll_right || pen_on_scrollthingy || pen_on_scrollbar) {
		pen_on_zoom_in = pen_on_zoom_out = pen_on_scroll_left =
		    pen_on_scroll_right = pen_on_scrollthingy = pen_on_scrollbar =
		        false;
		draw();
	} else if (pen_on_loop_start_point) {
		if (snap_to_zero_crossings) {
			s32 oldstart = smp->getLoopStart();
			s32 zerocrossing = find_zero_crossing_near(smp->getLoopStart());
			if (zerocrossing != -1) {
				smp->setLoopStartAndLength(zerocrossing,
				                           smp->getLoopLength() -
				                               (zerocrossing - oldstart));
				draw();
			}
		}
	} else if (pen_on_loop_end_point) {
		if (snap_to_zero_crossings) {
			s32 zerocrossing = find_zero_crossing_near(smp->getLoopStart() +
			                                           smp->getLoopLength());
			if (zerocrossing != -1) {
				s32 newlength = zerocrossing - smp->getLoopStart();
				u32 newstart = smp->getLoopStart();
				if (newlength < 0) {
					newstart += newlength;
					newlength = 0;
				}
				smp->setLoopStartAndLength(newstart, newlength);
				draw();
			}
		}
	}

	pen_on_loop_start_point = false;
	pen_on_loop_end_point = false;
}

void SampleDisplay::penMove(u16 px, u16 py)
{
	if ((smp == 0) || ((active == false) && (loop_points_visible == false) &&
	                   (draw_mode == false)))
		return;

	bool redraw = !(pen_on_loop_start_point || pen_on_loop_end_point);

	if (pen_on_loop_start_point) {
		s32 olstart = smp->getLoopStart();
		s32 newstart =
		    pixelToSample((s32)px - (s32)x - 1 - (s32)loop_touch_offset);
		smp->setLoopStartAndLength(
		    newstart,
		    std::max((s32)0, (s32)smp->getLoopLength() - (newstart - olstart)));
		drawLoopHandles();
	} else if (pen_on_loop_end_point) {
		s32 newlength =
		    (s32)pixelToSample((s32)px - (s32)x - 1 - (s32)loop_touch_offset) -
		    smp->getLoopStart();
		u32 newstart = smp->getLoopStart();
		if (newlength < 0) {
			newstart += newlength;
			newlength = 0;
		}
		smp->setLoopStartAndLength(newstart, newlength);
		drawLoopHandles();
	} else if (pen_on_scrollthingy) {
		scrollthingypos =
		    ntxm_clamp(px - x - pen_x_on_scrollthingy - SCROLLBUTTON_HEIGHT, 0,
		               width - 2 * SCROLLBUTTON_HEIGHT - ZOOM_BUTTONS_MARGIN +
		                   2 - scrollthingywidth);

		u32 window_width = width - 2;
		u32 disp_width = window_width << zoom_level;
		u32 scroll_width = width - 2 * SCROLLBUTTON_HEIGHT -
		                   ZOOM_BUTTONS_MARGIN + 2 - scrollthingywidth;
		scrollpos =
		    scrollthingypos * (disp_width - window_width) / scroll_width;
	} else if (!pen_on_zoom_in && !pen_on_zoom_out && !pen_on_scroll_left &&
	           !pen_on_scroll_right && !pen_on_scrollbar) {
		if (active and !draw_mode) {
			if (px < x + 1)
				selend = pixelToSample(0);
			else if (px - x < width - 1)
				selend = pixelToSample(px - x - 1);
			else
				selend = pixelToSample(width - 2);

			if (selstart != selend)
				selection_exists = true;
		} else if (draw_mode) {
			int draw_x = ntxm_clamp(px - x - 1, 0, width - 2);
			int draw_y = ntxm_clamp(py - y - 1, 0, SAMPLE_DRAW_HEIGHT - 1);

			u32 sx1 = pixelToSample(draw_last_x);
			u32 sx2 = pixelToSample(draw_x);

			s16 sy1, sy2;
			if (smp->is16bit()) {
				sy1 = 65535 * (SAMPLE_DRAW_HEIGHT / 2 - draw_last_y) / SAMPLE_DRAW_HEIGHT;
				sy2 = 65535 * (SAMPLE_DRAW_HEIGHT / 2 - draw_y) / SAMPLE_DRAW_HEIGHT;
			} else {
				sy1 = 255 * (SAMPLE_DRAW_HEIGHT / 2 - draw_last_y) / SAMPLE_DRAW_HEIGHT;
				sy2 = 255 * (SAMPLE_DRAW_HEIGHT / 2 - draw_y) / SAMPLE_DRAW_HEIGHT;
			}

			smp->drawLine(sx1, sy1, sx2, sy2);
			ntxm_flush_dcache();

			draw_last_x = draw_x;
			draw_last_y = draw_y;
		}
	}

	if (redraw)
		draw();
}

void SampleDisplay::setSample(Sample *_smp)
{
	if (_smp != NULL && smp != _smp) {
		zoom_level = scrollpos = scrollthingypos = 0;
		calcScrollThingy();
	}

	smp = _smp;
	selection_exists = false;
	selstart = selend = 0;

	if (_smp == NULL) {
		loop_points_visible = false;
#ifdef NT_PLATFORM_NDS
		oamDisable(&oamSub);
#endif
	}
#ifdef NT_PLATFORM_NDS
	else if (isExposed())
		oamEnable(&oamSub);
#endif

	draw();
}

void SampleDisplay::select_all(void)
{
	if (smp == 0)
		return;

	selection_exists = true;
	selstart = 0;
	selend = smp->getNSamples();
	draw();
}

void SampleDisplay::clear_selection(void)
{
	if (smp == 0)
		return;

	selection_exists = false;
	selstart = 0;
	selend = 0;
	draw();
}

bool SampleDisplay::getSelection(u32 *startsample, u32 *endsample)
{
	if (selection_exists == false)
		return false;

	*startsample = selstart;
	*endsample = selend;

	return true;
}

// Drawing request
void SampleDisplay::pleaseDraw(void)
{
	draw();
}

// Functions to toggle "active" mode, i.e. the mode in which you can make selections
void SampleDisplay::setActive(void)
{
	active = true;
	draw();
}

void SampleDisplay::setInactive(void)
{
	active = false;
	selection_exists = false;
	selstart = selend = 0;
	draw();
}

void SampleDisplay::setDrawMode(bool _on)
{
	draw_mode = _on;

#ifdef NT_PLATFORM_NDS
	if (draw_mode)
		oamDisable(&oamSub);
	else
		oamEnable(&oamSub);
#endif
}

void SampleDisplay::showLoopPoints(void)
{
	if (loop_points_visible == false) {
		loop_points_visible = true;
		drawLoopHandles();
	}
}

void SampleDisplay::hideLoopPoints(void)
{
	if (loop_points_visible == true) {
		loop_points_visible = false;
		drawLoopHandles();
	}
}

void SampleDisplay::setOffsetGuide(s32 newpos)
{
	offset_guide_pos = newpos;
}

void SampleDisplay::setSnapToZeroCrossing(bool snap)
{
	snap_to_zero_crossings = snap;
}

void SampleDisplay::reveal(void)
{
#ifdef NT_PLATFORM_NDS
	if (smp)
		oamEnable(&oamSub);
#endif
	Widget::reveal();
}

void SampleDisplay::occlude(void)
{
#ifdef NT_PLATFORM_NDS
	oamDisable(&oamSub);
#endif
	Widget::occlude();
}

void SampleDisplay::setTheme(Theme *theme_, u16 bgcolor_)
{
#ifdef NT_PLATFORM_NDS
	*(SPRITE_PALETTE_SUB + 1) = theme_->col_loop;
	*(SPRITE_PALETTE_SUB + 1 + 16) = theme_->col_outline;
	*(SPRITE_PALETTE_SUB + 2 + 16) = theme_->col_loop;
	*(SPRITE_PALETTE_SUB + 1 + 32) = theme_->col_smp_zoom;
#endif

	Widget::setTheme(theme_, bgcolor_);
}

/* ===================== PRIVATE ===================== */

long SampleDisplay::find_zero_crossing_near(long pos)
{
	pos = ntxm_clamp(pos, 0, smp->getNSamples() - 1);

	int radius_div = (width - 2) << zoom_level;
	// 4 pixels in each direction
	int radius = div32((smp->getNSamples() + radius_div - 1) * 4, radius_div);

	if (smp->is16bit()) {
		s16 *data = (s16 *)smp->getData();
		u32 n_samples = smp->getNSamples();

		s16 lastsample_left, lastsample_right;
		lastsample_left = lastsample_right = data[pos];

		for (int i = 1; i < radius; ++i) {
			// Seek left
			if ((pos > i) &&
			    (((data[pos - i] >= 0) && (lastsample_left <= 0)) ||
			     ((data[pos - i] <= 0) && (lastsample_left >= 0)))) {
				return pos - i;
			}
			// Seek right
			else if ((pos + i < (long)n_samples) &&
			         (((data[pos + i] >= 0) && (lastsample_right <= 0)) ||
			          ((data[pos + i] <= 0) && (lastsample_right >= 0)))) {
				return pos + i;
			}

			lastsample_left = lastsample_right = data[pos];
		}

		return -1;
	} else {
		s8 *data = (s8 *)smp->getData();
		u32 n_samples = smp->getNSamples();

		s8 lastsample_left, lastsample_right;
		lastsample_left = lastsample_right = data[pos];

		for (int i = 1; i < radius; ++i) {
			// Seek left
			if ((pos > i) &&
			    (((data[pos - i] >= 0) && (lastsample_left <= 0)) ||
			     ((data[pos - i] <= 0) && (lastsample_left >= 0)))) {
				return pos - i;
			}
			// Seek right
			else if ((pos + i < (long)n_samples) &&
			         (((data[pos + i] >= 0) && (lastsample_right <= 0)) ||
			          ((data[pos + i] <= 0) && (lastsample_right >= 0)))) {
				return pos + i;
			}

			lastsample_left = lastsample_right = data[pos];
		}

		return -1;
	}
}

void SampleDisplay::drawLoopHandles(void)
{
#ifdef NT_PLATFORM_NDS
	u16 loop_start_pos = smp == 0 ? 0 : sampleToPixel(smp->getLoopStart());
	u16 loop_end_pos =
	    smp == 0 ? 0
	             : sampleToPixel(smp->getLoopStart() + smp->getLoopLength());

	oamSub.oamMemory[SPR_LOOPHANDLE_1_L].x = loop_start_pos + x - 6;
	oamSub.oamMemory[SPR_LOOPHANDLE_1_R].x = loop_start_pos + x - 6 + 8 - 1;

	oamSub.oamMemory[SPR_LOOPHANDLE_2_L].x = loop_end_pos + x - 6;
	oamSub.oamMemory[SPR_LOOPHANDLE_2_R].x = loop_end_pos + x - 6 + 8 - 1;

	bool draw_loop_end = loop_points_visible && (loop_end_pos <= width + 8);
	bool draw_loop_start = loop_points_visible && (loop_start_pos <= width + 8);

	oamSetHidden(&oamSub, SPR_LOOPHANDLE_1_L, !draw_loop_start);
	oamSetHidden(&oamSub, SPR_LOOPHANDLE_1_R, !draw_loop_start);
	oamSub.oamMemory[SPR_LOOPLINE_1].x =
	    draw_loop_start ? loop_start_pos + x - 6 - 1 : x + width + 1;

	oamSetHidden(&oamSub, SPR_LOOPHANDLE_2_L, !draw_loop_end);
	oamSetHidden(&oamSub, SPR_LOOPHANDLE_2_R, !draw_loop_end);
	oamSub.oamMemory[SPR_LOOPLINE_2].x =
	    draw_loop_end ? loop_end_pos + x - 6 - 1 : x + width + 1;
#else
	draw();
#endif
}

void SampleDisplay::draw(void)
{
	if (!isExposed())
		return;

	// Border
	if (active == false) {
		drawBorder(theme->col_outline);
	} else {
		drawBorder(theme->col_signal);
	}

#ifdef NT_PLATFORM_NDS
	drawLoopHandles(); // Hides loop points if no sample/loop
#endif

	// Now comes sample-dependant stuff, so return if we have no sample
	if ((smp == 0) || (smp->getNSamples() == 0)) {
		drawFullBox(1, 1, width - 2, height - 2, theme->col_smp_bg);
		return;
	}

	//
	// Selection
	//
	s32 selleft = -1;
	s32 selright = -1;
	bool draw_selection = selection_exists;

	if (draw_selection) {
		selleft = sampleToPixel(std::min(selstart, selend));
		selright = sampleToPixel(std::max(selstart, selend));

		if (selleft < 0)
			selleft = 0;
		else if (selleft > (width - 2))
			draw_selection = false;

		if (selright > width - 2)
			selright = width - 2;
		else if (selright < 0)
			draw_selection = false;
	}

	//
	// Scrollbar
	//

	// Right Button
	if (pen_on_scroll_right) {
		drawGradient(theme->col_scrollbar_arr_bg1, theme->col_scrollbar_arr_bg2,
		             width - 8 - ZOOM_BUTTONS_MARGIN,
		             height - SCROLLBAR_WIDTH + 1, 8, 8);
	} else {
		drawGradient(theme->col_scrollbar_arr_bg2, theme->col_scrollbar_arr_bg1,
		             width - 8 - ZOOM_BUTTONS_MARGIN,
		             height - SCROLLBAR_WIDTH + 1, 8, 8);
	}

	// This draws the right-arrow
	s8 j, p;
	for (j = 0; j < 3; j++) {
		for (p = -j; p <= j; ++p) {
			drawPixel(width - ZOOM_BUTTONS_MARGIN - j - 4,
			          height - SCROLLBAR_WIDTH + 4 + p, theme->col_icon_bt);
		}
	}

	drawBox(width - SCROLLBAR_WIDTH - ZOOM_BUTTONS_MARGIN,
	        height - SCROLLBUTTON_HEIGHT, 9, 9, theme->col_outline);

	// Left Button
	if (pen_on_scroll_left) {
		drawGradient(theme->col_scrollbar_arr_bg1, theme->col_scrollbar_arr_bg2,
		             1, height - 9, 8, 8);
	} else {
		drawGradient(theme->col_scrollbar_arr_bg2, theme->col_scrollbar_arr_bg1,
		             1, height - 9, 8, 8);
	}

	// This draws the left-arrow
	for (j = 2; j >= 0; j--) {
		for (p = -j; p <= j; ++p) {
			drawPixel(j + 3, height - SCROLLBAR_WIDTH + 4 + p,
			          theme->col_icon_bt);
		}
	}

	drawBox(0, height - 9, 9, 9, theme->col_outline);

	//
	// Zoom buttons on scrollbar
	//

	// Zoom in
	if (pen_on_zoom_in) {
		drawGradient(theme->col_scrollbar_arr_bg1, theme->col_scrollbar_arr_bg2,
		             width - 2 * SCROLLBUTTON_HEIGHT, height - 9, 9, 8);
	} else {
		drawGradient(theme->col_scrollbar_arr_bg2, theme->col_scrollbar_arr_bg1,
		             width - 2 * SCROLLBUTTON_HEIGHT, height - 9, 9, 8);
	}

	drawBox(width - 2 * SCROLLBUTTON_HEIGHT - 1, height - 9, 10, 9,
	        theme->col_outline);

	// + icon
	drawHLine(width - 2 * SCROLLBUTTON_HEIGHT + 1, height - 9 + 4, 6,
	          theme->col_icon_bt);
	drawVLine(width - 2 * SCROLLBUTTON_HEIGHT + 3, height - 9 + 2, 5,
	          theme->col_icon_bt);
	drawVLine(width - 2 * SCROLLBUTTON_HEIGHT + 4, height - 9 + 2, 5,
	          theme->col_icon_bt);

	//Zoom out
	if (pen_on_zoom_out) {
		drawGradient(theme->col_scrollbar_arr_bg1, theme->col_scrollbar_arr_bg2,
		             width - 1 * SCROLLBUTTON_HEIGHT, height - 9, 9, 8);
	} else {
		drawGradient(theme->col_scrollbar_arr_bg2, theme->col_scrollbar_arr_bg1,
		             width - 1 * SCROLLBUTTON_HEIGHT, height - 9, 9, 8);
	}

	drawBox(width - 1 * SCROLLBUTTON_HEIGHT - 1, height - 9, 10, 9,
	        theme->col_outline);

	// - icon
	drawHLine(width - 1 * SCROLLBUTTON_HEIGHT + 1, height - 9 + 4, 6,
	          theme->col_icon_bt);

	drawBox(0, height - SCROLLBAR_WIDTH, width, SCROLLBAR_WIDTH,
	        theme->col_outline);

	// Clear Scrollbar
	drawGradient(theme->col_scrollbar_bg1, theme->col_scrollbar_bg2,
	             SCROLLBUTTON_HEIGHT, height - SCROLLBAR_WIDTH + 1,
	             width - 2 * SCROLLBUTTON_HEIGHT - ZOOM_BUTTONS_MARGIN,
	             SCROLLBAR_WIDTH - 2);

	// The scroll thingy
	if (pen_on_scrollthingy) {
		drawFullBox(SCROLLBUTTON_HEIGHT + scrollthingypos,
		            height - SCROLLBAR_WIDTH + 1, scrollthingywidth - 2,
		            SCROLLBAR_WIDTH - 2, theme->col_scrollbar_active);
	} else {
		drawFullBox(SCROLLBUTTON_HEIGHT + scrollthingypos,
		            height - SCROLLBAR_WIDTH + 1, scrollthingywidth - 2,
		            SCROLLBAR_WIDTH - 2, theme->col_scrollbar_inactive);
	}

	drawBox(SCROLLBUTTON_HEIGHT - 1 + scrollthingypos, height - SCROLLBAR_WIDTH,
	        scrollthingywidth, SCROLLBAR_WIDTH, theme->col_outline);

	//
	// Sample
	//

	/* u16 colortable[SAMPLE_DRAW_HEIGHT+2];
	u16 colortable_selected[SAMPLE_DRAW_HEIGHT+2];
	for(s32 i=0; i<SAMPLE_DRAW_HEIGHT+2; i++) {
		// colortable[i] = interpolateColor(theme->col_light_ctrl, theme->col_dark_ctrl, i<<4);
		colortable[i] = theme->col_smp_waveform;
		// colortable_selected[i] = ((colortable[i] >> 2) & 0x1CE7) | 0x8000;
		colortable_selected[i] = theme->col_smp_waveform_sel;
	} */

	s32 step =
	    divf32(inttof32(smp->getNSamples() >> zoom_level), inttof32(width - 2));
	s32 pos = 0;

	u32 renderwindow =
	    (u32)std::max(1, std::min(100, (int)ceil_f32toint(step)));

	u16 bottom = 0;
	u16 middle = (SAMPLE_DRAW_HEIGHT / 2);
	u16 top = SAMPLE_DRAW_HEIGHT - 1;

	s32 lastmax = 0, lastmin = 0;

	u32 offsetpos = sampleToPixel(offset_guide_pos);

	if (smp->is16bit() == true) {

		s16 *data;
		s16 *base = (s16 *)smp->getData() + pixelToSample(0);

		for (s32 i = 0; i < s32(width - 2);) {
			bool draw_selection_here =
			    (draw_selection && i >= selleft && i < selright);
			bool draw_offset_line_here = offsetpos == i;

			u16 wave_col = draw_selection_here ? theme->col_smp_waveform_sel
			                                   : theme->col_smp_waveform;
			u16 bg_col =
			    draw_selection_here ? theme->col_smp_bg_sel : theme->col_smp_bg;

			u16 colortable_current =
			    draw_offset_line_here ? theme->col_smp_offset_guide : wave_col;
			u16 bg_current =
			    draw_offset_line_here ? theme->col_smp_offset_guide : bg_col;

			data = &(base[f32toint(pos)]);

			s32 maxsmp = -32767, minsmp = 32767;

			for (u32 j = 0; j < renderwindow; ++j) {
				if (*data > maxsmp)
					maxsmp = *data;
				if (*data < minsmp)
					minsmp = *data;
				data++;
			}

			s32 maxy = div32(SAMPLE_DRAW_HEIGHT * maxsmp, 2 * 32767);
			s32 miny = div32(SAMPLE_DRAW_HEIGHT * minsmp, 2 * 32767);

			if (i > 0) {
				if (lastmin > maxy)
					maxy = lastmin;
				if (lastmax < miny)
					miny = lastmax;
			}

			lastmax = maxy;
			lastmin = miny;

			miny += middle;
			maxy += middle;
			if (miny > maxy) {
				miny = middle;
				maxy = middle;
			} else {
				if (maxy > top) maxy = top;
				if (miny < bottom) miny = bottom;
			}

			i++;

			s16 j;
			for (j = 0; j < miny; ++j)
				drawPixel(i, 1 + top - j, bg_current);
			for (; j <= maxy; ++j)
				drawPixel(i, 1 + top - j, colortable_current); /* [top-j]; */
			for (; j <= top; ++j)
				drawPixel(i, 1 + top - j, bg_current);

			pos += step;
		}

	} else {

		s8 *data;
		s8 *base = (s8 *)smp->getData() + pixelToSample(0);

		for (s32 i = 0; i < s32(width - 2);) {
			bool draw_selection_here =
			    (draw_selection && i >= selleft && i < selright);
			bool draw_offset_line_here = offsetpos == i;

			u16 wave_col = draw_selection_here ? theme->col_smp_waveform_sel
			                                   : theme->col_smp_waveform;
			u16 bg_col =
			    draw_selection_here ? theme->col_smp_bg_sel : theme->col_smp_bg;

			u16 colortable_current =
			    draw_offset_line_here ? theme->col_smp_offset_guide : wave_col;
			u16 bg_current =
			    draw_offset_line_here ? theme->col_smp_offset_guide : bg_col;
			data = &(base[f32toint(pos)]);

			s8 maxsmp = -127, minsmp = 127;

			for (u32 j = 0; j < renderwindow; ++j) {
				if (*data > maxsmp)
					maxsmp = *data;
				if (*data < minsmp)
					minsmp = *data;
				data++;
			}

			s32 maxy = div32(SAMPLE_DRAW_HEIGHT * maxsmp, 2 * 127);
			s32 miny = div32(SAMPLE_DRAW_HEIGHT * minsmp, 2 * 127);

			if (i > 0) {
				if (lastmin > maxy)
					maxy = lastmin;
				if (lastmax < miny)
					miny = lastmax;
			}

			lastmax = maxy;
			lastmin = miny;

			miny += middle;
			maxy += middle;
			if (miny > maxy) {
				miny = middle;
				maxy = middle;
			} else {
				if (maxy > top) maxy = top;
				if (miny < bottom) miny = bottom;
			}

			i++;

			s16 j;
			for (j = 0; j < miny; ++j)
				drawPixel(i, 1 + top - j, bg_current);
			for (; j <= maxy; ++j)
				drawPixel(i, 1 + top - j, colortable_current); /* [top-j]; */
			for (; j <= top; ++j)
				drawPixel(i, 1 + top - j, bg_current);

			pos += step;
		}
	}

#ifndef NT_PLATFORM_NDS
	//
	// Loop Points
	//
	if ((loop_points_visible) && (smp->getLoop() != NO_LOOP) && !draw_mode) {
		s32 loop_start_pos = sampleToPixel(smp->getLoopStart());
		s32 loop_end_pos =
		    sampleToPixel(smp->getLoopStart() + smp->getLoopLength());

		// Loop Start

		if ((loop_start_pos >= 0) && (loop_start_pos <= width - 2)) {
			loop_start_pos++;

			// Line
			drawVLine(loop_start_pos, 1, SAMPLE_DRAW_HEIGHT, theme->col_loop);

			// Left Triangle
			if (loop_start_pos > 1 + LOOP_TRIANGLE_SIZE) {
				drawHLine(loop_start_pos - 2,
				          SAMPLE_DRAW_HEIGHT + 1 - LOOP_TRIANGLE_SIZE, 2,
				          theme->col_outline);

				for (u8 i = 0; i < LOOP_TRIANGLE_SIZE - 2; ++i) {
					drawHLine(loop_start_pos - i - 2,
					          SAMPLE_DRAW_HEIGHT + 2 - LOOP_TRIANGLE_SIZE + i, i + 2,
					          theme->col_loop);
					drawPixel(loop_start_pos - i - 3,
					          SAMPLE_DRAW_HEIGHT + 2 - LOOP_TRIANGLE_SIZE + i,
					          theme->col_outline);
				}

				drawHLine(loop_start_pos - LOOP_TRIANGLE_SIZE + 1, SAMPLE_DRAW_HEIGHT,
				          LOOP_TRIANGLE_SIZE - 1, theme->col_loop);
				drawPixel(loop_start_pos - LOOP_TRIANGLE_SIZE, SAMPLE_DRAW_HEIGHT,
				          theme->col_outline);
			}

			// Right Triangle
			if (loop_start_pos < width - 2 - LOOP_TRIANGLE_SIZE) {
				drawHLine(loop_start_pos + 1,
				          SAMPLE_DRAW_HEIGHT + 1 - LOOP_TRIANGLE_SIZE, 2,
				          theme->col_outline);
				for (u8 i = 0; i < LOOP_TRIANGLE_SIZE - 2; ++i) {
					drawHLine(loop_start_pos + 1,
					          SAMPLE_DRAW_HEIGHT + 2 - LOOP_TRIANGLE_SIZE + i, 2 + i,
					          theme->col_loop);
					drawPixel(loop_start_pos + 3 + i,
					          SAMPLE_DRAW_HEIGHT + 2 - LOOP_TRIANGLE_SIZE + i,
					          theme->col_outline);
				}
				drawHLine(loop_start_pos + 1,
				          SAMPLE_DRAW_HEIGHT - LOOP_TRIANGLE_SIZE + LOOP_TRIANGLE_SIZE,
				          LOOP_TRIANGLE_SIZE - 1, theme->col_loop);
				drawPixel(loop_start_pos + LOOP_TRIANGLE_SIZE, SAMPLE_DRAW_HEIGHT,
				          theme->col_outline);
			}
		}

		// Loop End

		if ((loop_end_pos >= 0) && (loop_end_pos <= width - 2)) {
			loop_end_pos++;

			// Line
			drawVLine(loop_end_pos, 1, SAMPLE_DRAW_HEIGHT, theme->col_loop);

			// Left Triangle
			if (loop_end_pos > 1 + LOOP_TRIANGLE_SIZE) {
				drawHLine(loop_end_pos - LOOP_TRIANGLE_SIZE + 1, 1,
				          LOOP_TRIANGLE_SIZE - 1, theme->col_loop);
				drawPixel(loop_end_pos - LOOP_TRIANGLE_SIZE, 1,
				          theme->col_outline);

				for (u8 i = 0; i < LOOP_TRIANGLE_SIZE - 2; ++i) {
					drawHLine(loop_end_pos - LOOP_TRIANGLE_SIZE + i + 1, 2 + i,
					          LOOP_TRIANGLE_SIZE - i - 1, theme->col_loop);
					drawPixel(loop_end_pos - 1 - LOOP_TRIANGLE_SIZE + i + 1,
					          2 + i, theme->col_outline);
				}

				drawHLine(loop_end_pos - 2, LOOP_TRIANGLE_SIZE, 2,
				          theme->col_outline);
			}

			// Right Triangle
			if (loop_end_pos < width - 1 - LOOP_TRIANGLE_SIZE) {
				drawHLine(loop_end_pos + 1, 1, LOOP_TRIANGLE_SIZE - 1,
				          theme->col_loop);
				drawPixel(loop_end_pos + LOOP_TRIANGLE_SIZE, 1,
				          theme->col_outline);

				for (u8 i = 0; i < LOOP_TRIANGLE_SIZE - 2; ++i) {
					drawHLine(loop_end_pos + 1, 2 + i,
					          LOOP_TRIANGLE_SIZE - i - 1, theme->col_loop);
					drawPixel(loop_end_pos + LOOP_TRIANGLE_SIZE - i, 2 + i,
					          theme->col_outline);
				}

				drawHLine(loop_end_pos + 1, LOOP_TRIANGLE_SIZE, 2,
				          theme->col_outline);
			}
		}
	}
#endif
}

void SampleDisplay::scroll(u32 newscrollpos)
{
	u32 window_width = width - 2;
	u64 disp_width = (u64)window_width << zoom_level;
	u32 scroll_width = width - 2 * SCROLLBUTTON_HEIGHT - ZOOM_BUTTONS_MARGIN +
	                   2 - scrollthingywidth;

	scrollpos = ntxm_clamp(newscrollpos, 0, disp_width - window_width);

	if (disp_width - window_width == 0)
		scrollthingypos = 0;
	else
		scrollthingypos =
		    scrollpos * scroll_width / (disp_width - window_width);

	calcScrollThingy();
	draw();
}

// Calculate height and position of the scroll thingy
void SampleDisplay::calcScrollThingy(void)
{
	u16 sch = width - 2 * SCROLLBUTTON_HEIGHT - ZOOM_BUTTONS_MARGIN + 2;
	scrollthingywidth = sch >> zoom_level;
	if (scrollthingywidth < MIN_SCROLLTHINGY_WIDTH)
		scrollthingywidth = MIN_SCROLLTHINGY_WIDTH;
}

void SampleDisplay::zoomIn(void)
{
	if (zoom_level == SAMPLE_MAX_ZOOM)
		return;

	zoom_level++;
	calcScrollThingy();
	scrollpos = scrollpos * 2 + (width - 2) / 2;
	scroll(scrollpos);
}

void SampleDisplay::zoomOut(void)
{
	if (zoom_level == 0)
		return;

	zoom_level--;
	calcScrollThingy();
	s32 scrollpostmp = ((s32)scrollpos * 2 - (width - 2)) / 4;
	scrollpos = std::max(0, (int)scrollpostmp);
	scroll(scrollpos);

	/*
	zoom_level--;
	scrollthingypos = ntxm_clamp(scrollthingypos - scrollthingywidth / 2, 0, width - 2*SCROLLBUTTON_HEIGHT);
	calcScrollThingy();
	scrollthingypos = ntxm_clamp(scrollthingypos, 0, width - 2*SCROLLBUTTON_HEIGHT+2 - scrollthingywidth);

	scrollpos = scrollthingypos;
	for(u8 i=0; i < zoom_level; ++i) scrollpos *= 2;

	draw();
	*/
}

u32 SampleDisplay::pixelToSample(s32 pixel)
{
	pixel = ntxm_clamp(pixel, 0, width - 2);
	u64 abspos = scrollpos + pixel;
	return u32(abspos * smp->getNSamples() / (u64(width - 2) << zoom_level));
}

s32 SampleDisplay::sampleToPixel(u32 sample)
{
	if (sample >= smp->getNSamples())
		return width - 2;

	return s32(sample * (s64(width - 2) << zoom_level) / smp->getNSamples() -
	           scrollpos);
}
