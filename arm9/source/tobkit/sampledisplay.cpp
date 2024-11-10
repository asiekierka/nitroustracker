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

#include <math.h>
#include <stdio.h>

#include "ntxm/ntxmtools.h"
#include "../tools.h"

/* ===================== PUBLIC ===================== */

// Constructor sets base variables
SampleDisplay::SampleDisplay(u8 _x, u8 _y, u8 _width, u8 _height, u16 **_vram, Sample *_smp)
	:Widget(_x, _y, _width, _height, _vram),
	smp(_smp),
	selstart(0), selend(0), selection_exists(false), pen_is_down(false), active(false), loop_points_visible(false),
	pen_on_loop_start_point(false), pen_on_loop_end_point(false),
	pen_on_zoom_in(false), pen_on_zoom_out(false),
	pen_on_scroll_left(false), pen_on_scroll_right(false), pen_on_scrollthingy(false), pen_on_scrollbar(false),
	scrollthingypos(0), scrollthingywidth(width-2*SCROLLBUTTON_HEIGHT+2), pen_x_on_scrollthingy(0), zoom_level(0), scrollpos(0),
	snap_to_zero_crossings(true), draw_mode(false)
{

}

SampleDisplay::~SampleDisplay(void)
{

}

void SampleDisplay::penDown(u8 px, u8 py)
{
	if( (smp==0) || ( (active==false) && (loop_points_visible==false) && (draw_mode == false) ) )
		return;

	if(draw_mode) {
		draw_last_x = px - x;
		draw_last_y = py - y;
	} else {
		// Stylus on a loop point?
		u32 loop_start_pos = sampleToPixel(smp->getLoopStart());//smp->getLoopStart() * (width-2) / smp->getNSamples();
		u32 loop_end_pos   = sampleToPixel(smp->getLoopStart() + smp->getLoopLength());//(smp->getLoopStart() + smp->getLoopLength()) * (width-2) / smp->getNSamples();

		if(isInRect(px-x, py-y, std::max((s32)0, (s32)loop_start_pos - (s32)LOOP_TRIANGLE_SIZE), DRAW_HEIGHT+2-LOOP_TRIANGLE_SIZE,
			loop_start_pos+LOOP_TRIANGLE_SIZE, DRAW_HEIGHT+2))
		{
			pen_on_loop_start_point = true;
			loop_touch_offset = px-x-1 - loop_start_pos;
		}
		else if(isInRect(px-x, py-y, std::max((s32)0, (s32)loop_end_pos - (s32)LOOP_TRIANGLE_SIZE), 0,
			loop_end_pos+LOOP_TRIANGLE_SIZE, LOOP_TRIANGLE_SIZE))
		{
			pen_on_loop_end_point = true;
			loop_touch_offset = px-x-1 - loop_end_pos;
		}

		// Else: Stylus on a zoom button?
		else if(isInRect(px-x, py-y, 1, 1, 18, 10))	{
			if(px-x < 10) {
				pen_on_zoom_in = true;
				zoomIn();
			} else {
				pen_on_zoom_out = true;
				zoomOut();
			}
		}

		// Else: Stylus on the scrollar?
		else if(py - y >= height - SCROLLBAR_WIDTH)	{
			if(px - x <= SCROLLBUTTON_HEIGHT) {
				pen_on_scroll_left = true;
				scroll(scrollpos-SCROLLPIXELS);
			} else if(px - x >= width - SCROLLBUTTON_HEIGHT) {
				pen_on_scroll_right = true;
				scroll(scrollpos+SCROLLPIXELS);
			} else if(px - x < scrollthingypos + SCROLLBUTTON_HEIGHT)	{
				scroll(scrollpos-width+2);
			} else if(px - x > scrollthingypos + SCROLLBUTTON_HEIGHT + scrollthingywidth) {
				scroll(scrollpos+width-2);
			} else {
				pen_on_scrollthingy = true;
				pen_x_on_scrollthingy = px - x - SCROLLBUTTON_HEIGHT-scrollthingypos;
			}
			pen_on_scrollbar = true;
		}

		// Else: Stylus on the sample.
		else if(active) {
			selstart = selend = pixelToSample(px - x);
			selection_exists = false;
			draw();
		}
	}
}

void SampleDisplay::penUp(u8 px, u8 py)
{
	// Swap selstart and selend if they are in the wrong order
	if(selend < selstart) {
		u32 tmp = selstart;
		selstart = selend;
		selend = tmp;
	}

	if( pen_on_zoom_in || pen_on_zoom_out || pen_on_scroll_left || pen_on_scroll_right || pen_on_scrollthingy || pen_on_scrollbar) {
		pen_on_zoom_in = pen_on_zoom_out = pen_on_scroll_left = pen_on_scroll_right = pen_on_scrollthingy = pen_on_scrollbar = false;
		draw();
	} else if(pen_on_loop_start_point) {
		if(snap_to_zero_crossings) {
			s32 oldstart = smp->getLoopStart();
			s32 zerocrossing = find_zero_crossing_near(smp->getLoopStart());
			if(zerocrossing != -1) {
				smp->setLoopLength(smp->getLoopLength() - (zerocrossing - oldstart));
				smp->setLoopStart(zerocrossing);
				DC_FlushAll();
				draw();
			}
		}
	} else if(pen_on_loop_end_point) {
		if(snap_to_zero_crossings) {
			s32 zerocrossing = find_zero_crossing_near(smp->getLoopStart() + smp->getLoopLength());
			if(zerocrossing != -1) {
				smp->setLoopLength( zerocrossing - smp->getLoopStart() );
				DC_FlushAll();
				draw();
			}
		}
	}

	pen_on_loop_start_point = false;
	pen_on_loop_end_point = false;
}

void SampleDisplay::penMove(u8 px, u8 py)
{
	if((smp==0) || ( (active==false) && (loop_points_visible==false) && (draw_mode == false) ) )
		   return;

	if(pen_on_loop_start_point) {
		s32 olstart = smp->getLoopStart();
		s32 newstart = pixelToSample((s32)px-(s32)x-1-(s32)loop_touch_offset);
		smp->setLoopStartAndLength(newstart, std::max((s32)0, (s32)smp->getLoopLength() - (newstart - olstart)));
		DC_FlushAll();
	}
	else if(pen_on_loop_end_point) {
		s32 newlength = (s32)pixelToSample((s32)px-(s32)x-1-(s32)loop_touch_offset) - smp->getLoopStart();
		if(newlength < 0) {
			u32 newstart = smp->getLoopStart() + newlength;
			smp->setLoopStart(newstart);
			newlength = 0;
		}
		smp->setLoopLength(newlength);
		DC_FlushAll();
	}
	else if(pen_on_scrollthingy)
	{
		scrollthingypos = my_clamp(px - x - pen_x_on_scrollthingy - SCROLLBUTTON_HEIGHT, 0, width - 2*SCROLLBUTTON_HEIGHT+2 - scrollthingywidth);

		u32 window_width = width - 2;
		u32 disp_width = window_width << zoom_level;
		u32 scroll_width = width - 2*SCROLLBUTTON_HEIGHT+2 - scrollthingywidth;
		scrollpos = scrollthingypos * (disp_width - window_width) / scroll_width;
	}
	else if( !pen_on_zoom_in && !pen_on_zoom_out && !pen_on_scroll_left && !pen_on_scroll_right && !pen_on_scrollbar)
	{
		if(active and !draw_mode) {
			if(px<x+1)
				selend = pixelToSample(0);
			else if(px-x<width-1)
				selend = pixelToSample(px-x);
			else
				selend = pixelToSample(width-2);

			if(selstart != selend)
				selection_exists = true;
		} else if(draw_mode) {
			int draw_x = my_clamp(px - x - 1, 0, width-2);
			int draw_y = my_clamp(py - y - 1, 0, DRAW_HEIGHT-1);

			u32 sx1 = pixelToSample(draw_last_x);
			u32 sx2 = pixelToSample(draw_x);

			s16 sy1, sy2;
			if(smp->is16bit()) {
				sy1 = 65535 * (DRAW_HEIGHT/2 - draw_last_y) / DRAW_HEIGHT;
				sy2 = 65535 * (DRAW_HEIGHT/2 - draw_y) / DRAW_HEIGHT;
			} else {
				sy1 = 255 * (DRAW_HEIGHT/2 - draw_last_y) / DRAW_HEIGHT;
				sy2 = 255 * (DRAW_HEIGHT/2 - draw_y) / DRAW_HEIGHT;
			}

			smp->drawLine(sx1, sy1, sx2, sy2);
			DC_FlushAll();

			draw_last_x = draw_x;
			draw_last_y = draw_y;
		}
	}

	draw();
}

void SampleDisplay::setSample(Sample *_smp)
{
	smp = _smp;
	selection_exists = false;
	selstart = selend = 0;
	if(_smp == 0) {
		loop_points_visible = false;
	}
	draw();
}

void SampleDisplay::select_all(void)
{
	if(smp==0) return;

	selection_exists = true;
	selstart = 0;
	selend = smp->getNSamples();
	draw();
}

void SampleDisplay::clear_selection(void)
{
	if(smp==0) return;

	selection_exists = false;
	selstart = 0;
	selend = 0;
	draw();
}

bool SampleDisplay::getSelection(u32 *startsample, u32 *endsample)
{
	if(selection_exists == false) return false;

	*startsample = selstart;
	*endsample   = selend;

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
	draw();
}

void SampleDisplay::showLoopPoints(void)
{
	if(loop_points_visible == false) {
		loop_points_visible = true;
		draw();
	}
}

void SampleDisplay::hideLoopPoints(void)
{
	if(loop_points_visible == true)
	{
		loop_points_visible = false;
		draw();
	}
}

void SampleDisplay::setSnapToZeroCrossing(bool snap)
{
	snap_to_zero_crossings = snap;
}

/* ===================== PRIVATE ===================== */

long SampleDisplay::find_zero_crossing_near(long pos)
{
	if( smp->is16bit() )
	{
		s16 *data = (s16*)smp->getData();
		u32 n_samples = smp->getNSamples();

		s16 lastsample_left, lastsample_right;
		lastsample_left = lastsample_right = data[pos];

		for(int i=1; i<SNAP_TO_ZERO_CROSSING_RADIUS; ++i)
		{
			// Seek left
			if( ( pos > i ) &&
				( ( (data[pos - i] >= 0) && (lastsample_left <= 0) ) ||
				  ( (data[pos - i] <= 0) && (lastsample_left >= 0) ) ) )
			{
				return pos-i;
			}
			// Seek right
			else
			if( ( pos + i < (long)n_samples ) &&
			    ( ( (data[pos + i] >= 0) && (lastsample_right <= 0) ) ||
			      ( (data[pos + i] <= 0) && (lastsample_right >= 0) ) ) )
			{
				return pos+i;
			}

			lastsample_left = lastsample_right = data[pos];
		}

		return -1;
	}
	else
	{
		s8 *data = (s8*)smp->getData();
		u32 n_samples = smp->getNSamples();

		s8 lastsample_left, lastsample_right;
		lastsample_left = lastsample_right = data[pos];

		for(int i=1; i<SNAP_TO_ZERO_CROSSING_RADIUS; ++i)
		{
			// Seek left
			if( ( pos > i ) &&
				( ( (data[pos - i] >= 0) && (lastsample_left <= 0) ) ||
				  ( (data[pos - i] <= 0) && (lastsample_left >= 0) ) ) )
			{
				return pos-i;
			}
			// Seek right
			else
			if( ( pos + i < (long)n_samples ) &&
				( ( (data[pos + i] >= 0) && (lastsample_right <= 0) ) ||
				( (data[pos + i] <= 0) && (lastsample_right >= 0) ) ) )
			{
				return pos+i;
			}

			lastsample_left = lastsample_right = data[pos];
		}

		return -1;
	}
}

void SampleDisplay::draw(void)
{
	if(!isExposed())
		return;

	//
	// Border and background
	//
	drawFullBox(0, 0, width, height, theme->col_dark_bg);
	
	if(active==false) {
		drawBorder();
	} else {
		drawBorder(RGB15(31,0,0)|BIT(15));
	}

	// Now comes sample-dependant stuff, so return if we have no sample
	if((smp==0)||(smp->getNSamples()==0)) return;

	//
	// Selection
	//
	s32 selleft = 0;
	s32 selwidth = 0;
	s32 selright = 0;

	if(selection_exists) {
		selleft = sampleToPixel(std::min(selstart, selend));
		selright = sampleToPixel(std::max(selstart, selend));
		bool dontdraw = false;

		if (selleft < 1) selleft = 1;
		else if (selleft > (width-1)) dontdraw = true;

		if (selright > width-1) selright = width-1;
		else if (selright < 1) dontdraw = true;

		selwidth = selright - selleft;
		if(!dontdraw) {
			drawFullBox(selleft, 1, selwidth, DRAW_HEIGHT+1, RGB15(31,31,0)|BIT(15));
		}
	}

	//
	// Scrollbar
	//

	// Right Button
	if(pen_on_scroll_right) {
		drawGradient(theme->col_dark_ctrl, theme->col_light_ctrl, width-9, height-SCROLLBAR_WIDTH+1, 8, 8);
	} else {
		drawGradient(theme->col_light_ctrl, theme->col_dark_ctrl, width-9, height-SCROLLBAR_WIDTH+1, 8, 8);
	}

	// This draws the right-arrow
	s8 j, p;
	for(j=0;j<3;j++) {
		for(p=-j;p<=j;++p) {
			*(*vram+SCREEN_WIDTH*(y+height-SCROLLBAR_WIDTH+4+p)+x+width-j-3) = RGB15(0,0,0) | BIT(15);
		}
	}

	drawBox(width-SCROLLBAR_WIDTH, height-SCROLLBUTTON_HEIGHT, 9, 9);

	// Left Button
	if(pen_on_scroll_left) {
		drawGradient(theme->col_dark_ctrl, theme->col_light_ctrl, 1, height-9, 8, 8);
	} else {
		drawGradient(theme->col_light_ctrl, theme->col_dark_ctrl, 1, height-9, 8, 8);
	}

	// This draws the down-arrow
	for(j=2;j>=0;j--) {
		for(p=-j;p<=j;++p) {
			*(*vram+SCREEN_WIDTH*(y+height-SCROLLBAR_WIDTH+4+p)+x+j+3) = theme->col_icon;
		}
	}

	drawBox(0, height-9, 9, 9);


	drawBox(0, height-SCROLLBAR_WIDTH, width, SCROLLBAR_WIDTH);

	// Clear Scrollbar
	drawGradient(theme->col_medium_bg, theme->col_light_bg, SCROLLBUTTON_HEIGHT, height-SCROLLBAR_WIDTH+1, width-2*SCROLLBUTTON_HEIGHT, SCROLLBAR_WIDTH-2);

	// The scroll thingy
	if(pen_on_scrollthingy) {
		drawFullBox(SCROLLBUTTON_HEIGHT+scrollthingypos, height-SCROLLBAR_WIDTH+1, scrollthingywidth-2, SCROLLBAR_WIDTH-2, theme->col_light_ctrl);
	} else {
		drawFullBox(SCROLLBUTTON_HEIGHT+scrollthingypos, height-SCROLLBAR_WIDTH+1, scrollthingywidth-2, SCROLLBAR_WIDTH-2, theme->col_dark_ctrl);
	}

	drawBox(SCROLLBUTTON_HEIGHT-1+scrollthingypos, height-SCROLLBAR_WIDTH, scrollthingywidth, SCROLLBAR_WIDTH);

	//
	// Sample
	//

	u16 colortable[DRAW_HEIGHT+2];
	u16 colortable_selected[DRAW_HEIGHT+2];
	for(s32 i=0; i<DRAW_HEIGHT+2; i++) {
		colortable[i] = interpolateColor(theme->col_light_ctrl, theme->col_dark_ctrl, i<<4);
		//colortable_selected[i] = ((colortable[i] >> 2) & 0x1CE7) | 0x8000;
		colortable_selected[i] = 0x8000;
	}

	int32 step = divf32(inttof32(smp->getNSamples() >> zoom_level), inttof32(width-2));
	int32 pos = 0;

	u32 renderwindow = (u32)std::max(1, std::min(100, (int) ceil_f32toint(step)));

	u16 middle = (DRAW_HEIGHT+2)/2;//-1;

	s32 lastmax=0, lastmin=0;
	if(smp->is16bit() == true) {

		s16 *data;
		s16 *base = (s16*)smp->getData() + pixelToSample(0);

		for(s32 i=1; i<s32(width-1); ++i)
		{
			u16 *colortable_current = (selection_exists && i >= selleft && i < selright) ? colortable_selected : colortable;
			data = &(base[f32toint(pos)]);

			s32 maxsmp = -32767, minsmp = 32767;

			for(u32 j=0;j<renderwindow;++j) {
				if(*data > maxsmp) maxsmp = *data;
				if(*data < minsmp) minsmp = *data;
				data++;
			}

			s32 maxy = div32((DRAW_HEIGHT+2) * maxsmp, 2 * 32767);
			s32 miny = div32((DRAW_HEIGHT+2) * minsmp, 2 * 32767);

			if(i>1) {
				if(lastmin > maxy) maxy = lastmin;
				if(lastmax < miny) miny = lastmax;
			}

			for(s16 j=miny; j<=maxy; ++j) (*vram)[SCREEN_WIDTH*(y+middle-j)+x+i] = colortable_current[middle-j];

			lastmax = maxy;
			lastmin = miny;

			*(*vram+SCREEN_WIDTH*(y+middle)+x+i) = colortable_current[middle];

			pos += step;
		}

	} else {

		s8 *data;
		s8 *base = (s8*)smp->getData() + pixelToSample(0);

		for(s32 i=1; i<s32(width-1); ++i)
		{
			u16 *colortable_current = (selection_exists && i >= selleft && i < selright) ? colortable_selected : colortable;
			data = &(base[f32toint(pos)]);

			s8 maxsmp = -127, minsmp = 127;

			for(u32 j=0;j<renderwindow;++j) {
				if(*data > maxsmp) maxsmp = *data;
				if(*data < minsmp) minsmp = *data;
				data++;
			}

			s8 maxy = div32((DRAW_HEIGHT+2) * maxsmp, 2 * 127);
			s8 miny = div32((DRAW_HEIGHT+2) * minsmp, 2 * 127);

			if(i>1) {
				if(lastmin > maxy) maxy = lastmin;
				if(lastmax < miny) miny = lastmax;
			}

			for(s16 j=miny; j<=maxy; ++j) (*vram)[SCREEN_WIDTH*(y+middle-j)+x+i] = colortable_current[middle-j];

			lastmax = maxy;
			lastmin = miny;

			*(*vram+SCREEN_WIDTH*(y+middle)+x+i) = colortable_current[middle];

			pos += step;
		}

	}

	//
	// Loop Points
	//
	if( (loop_points_visible) && (smp->getLoop() != NO_LOOP) && !draw_mode )
	{
		s32 loop_start_pos = sampleToPixel(smp->getLoopStart());
		s32 loop_end_pos   = sampleToPixel(smp->getLoopStart() + smp->getLoopLength());

		// Loop Start

		if( (loop_start_pos >= 0) && (loop_start_pos <= width-2) ) {
			// Line
			for(u8 i=1; i<DRAW_HEIGHT+1; ++i)
				*(*vram+SCREEN_WIDTH*(y+i)+x+loop_start_pos) = RGB15(7,25,5)|BIT(15);

			/* unused
			u8 cutoff = 0;
			if(loop_start_pos < 1+LOOP_TRIANGLE_SIZE)
				cutoff = 1+LOOP_TRIANGLE_SIZE - loop_start_pos;
			*/

			// Left Triangle
			if(loop_start_pos > 1 + LOOP_TRIANGLE_SIZE)
			{
				drawHLine(loop_start_pos-2, DRAW_HEIGHT+1-LOOP_TRIANGLE_SIZE, 2, RGB15(0,0,0)|BIT(15));

				for(u8 i=0; i<LOOP_TRIANGLE_SIZE-2; ++i)
				{
					drawHLine(loop_start_pos-i-2, DRAW_HEIGHT+2-LOOP_TRIANGLE_SIZE+i, i+2, RGB15(7,25,5)|BIT(15));
					drawPixel(loop_start_pos-i-3, DRAW_HEIGHT+2-LOOP_TRIANGLE_SIZE+i, RGB15(0,0,0)|BIT(15));
				}

				drawHLine(loop_start_pos-LOOP_TRIANGLE_SIZE+1, DRAW_HEIGHT, LOOP_TRIANGLE_SIZE-1,
					RGB15(7,25,5)|BIT(15));
				drawPixel(loop_start_pos-LOOP_TRIANGLE_SIZE, DRAW_HEIGHT, RGB15(0,0,0)|BIT(15));
			}

			// Right Triangle
			if(loop_start_pos < width - 2 - LOOP_TRIANGLE_SIZE)
			{
				drawHLine(loop_start_pos+1, DRAW_HEIGHT+1-LOOP_TRIANGLE_SIZE, 2, RGB15(0,0,0)|BIT(15));
				for(u8 i=0; i<LOOP_TRIANGLE_SIZE-2; ++i) {
					drawHLine(loop_start_pos+1, DRAW_HEIGHT+2-LOOP_TRIANGLE_SIZE+i, 2+i, RGB15(7,25,5)|BIT(15));
					drawPixel(loop_start_pos+3+i, DRAW_HEIGHT+2-LOOP_TRIANGLE_SIZE+i, RGB15(0,0,0)|BIT(15));
				}
				drawHLine(loop_start_pos+1, DRAW_HEIGHT-LOOP_TRIANGLE_SIZE+LOOP_TRIANGLE_SIZE, LOOP_TRIANGLE_SIZE-1,
					RGB15(7,25,5)|BIT(15));
				drawPixel(loop_start_pos+LOOP_TRIANGLE_SIZE, DRAW_HEIGHT, RGB15(0,0,0)|BIT(15));
			}
		}

		// Loop End

		if( (loop_end_pos >= 0) && (loop_end_pos <= width-2) ) {
			// Line
			for(u8 i=1; i<DRAW_HEIGHT+1; ++i)
				*(*vram+SCREEN_WIDTH*(y+i)+x+loop_end_pos) = RGB15(7,25,5)|BIT(15);

			// Left Triangle
			if(loop_end_pos > 1 + LOOP_TRIANGLE_SIZE)
			{
				drawHLine(loop_end_pos-LOOP_TRIANGLE_SIZE+1, 1, LOOP_TRIANGLE_SIZE-1,
					RGB15(7,25,5)|BIT(15));
				drawPixel(loop_end_pos-LOOP_TRIANGLE_SIZE, 1, RGB15(0,0,0)|BIT(15));

				for(u8 i=0; i<LOOP_TRIANGLE_SIZE-2; ++i)
				{
					drawHLine(loop_end_pos-LOOP_TRIANGLE_SIZE+i+1, 2+i, LOOP_TRIANGLE_SIZE-i-1, RGB15(7,25,5)|BIT(15));
					drawPixel(loop_end_pos-1-LOOP_TRIANGLE_SIZE+i+1, 2+i, RGB15(0,0,0)|BIT(15));
				}

				drawHLine(loop_end_pos-2, LOOP_TRIANGLE_SIZE, 2, RGB15(0,0,0)|BIT(15));
			}

			// Right Triangle
			if(loop_end_pos < width-1-LOOP_TRIANGLE_SIZE)
			{
				drawHLine(loop_end_pos+1, 1, LOOP_TRIANGLE_SIZE-1, RGB15(7,25,5)|BIT(15));
				drawPixel(loop_end_pos+LOOP_TRIANGLE_SIZE, 1, RGB15(0,0,0)|BIT(15));

				for(u8 i=0; i<LOOP_TRIANGLE_SIZE-2; ++i)
				{
					drawHLine(loop_end_pos+1, 2+i, LOOP_TRIANGLE_SIZE-i-1, RGB15(7,25,5)|BIT(15));
					drawPixel(loop_end_pos+LOOP_TRIANGLE_SIZE-i, 2+i, RGB15(0,0,0)|BIT(15));
				}

				drawHLine(loop_end_pos+1, LOOP_TRIANGLE_SIZE, 2, RGB15(0,0,0)|BIT(15));
			}
		}
	}

	//
	// Zoom buttons
	//

	if(!draw_mode) {
		// Outlines
		drawHLine(2, 1, 7, theme->col_light_bg);
		drawHLine(10, 1, 7, theme->col_light_bg);

		drawHLine(2, 9, 7, theme->col_light_bg);
		drawHLine(10, 9, 7, theme->col_light_bg);

		drawVLine(1, 2, 7, theme->col_light_bg);
		drawVLine(9, 2, 7, theme->col_light_bg);
		drawVLine(17, 2, 7, theme->col_light_bg);

		// +
		if(pen_on_zoom_in) {
			drawFullBox(2, 2, 7, 7, theme->col_light_bg);
			drawHLine(3, 5, 5, theme->col_dark_bg);
			drawVLine(5, 3, 5, theme->col_dark_bg);
		} else {
			drawHLine(3, 5, 5, theme->col_light_bg);
			drawVLine(5, 3, 5, theme->col_light_bg);
		}

		// -
		if(pen_on_zoom_out) {
			drawFullBox(10, 2, 7, 7, theme->col_light_bg);
			drawHLine(11, 5, 5, theme->col_dark_bg);
		} else {
			drawHLine(11, 5, 5, theme->col_light_bg);
		}
	}
}

void SampleDisplay::scroll(u32 newscrollpos)
{
	u32 window_width = width - 2;
	u64 disp_width = (u64)window_width << zoom_level;
	u32 scroll_width = width - 2*SCROLLBUTTON_HEIGHT+2 - scrollthingywidth;

	scrollpos = my_clamp(newscrollpos, 0, disp_width - window_width);
	scrollthingypos = scrollpos * scroll_width / (disp_width - window_width);

	calcScrollThingy();
	draw();
}

// Calculate height and position of the scroll thingy
void SampleDisplay::calcScrollThingy(void)
{
	u16 sch = width - 2 * SCROLLBUTTON_HEIGHT + 2;
	scrollthingywidth = sch >> zoom_level;
	if(scrollthingywidth < MIN_SCROLLTHINGY_WIDTH) scrollthingywidth = MIN_SCROLLTHINGY_WIDTH;
}

void SampleDisplay::zoomIn(void)
{
	if(zoom_level == SAMPLE_MAX_ZOOM) return;

	zoom_level++;
	calcScrollThingy();
	scrollpos = scrollpos * 2 + (width - 2) / 2;
	scroll(scrollpos);
}

void SampleDisplay::zoomOut(void)
{
	if(zoom_level == 0) return;

	zoom_level--;
	calcScrollThingy();
	s32 scrollpostmp = ((s32)scrollpos * 2 - (width - 2)) / 4;
	scrollpos = std::max(0, (int)scrollpostmp);
	scroll(scrollpos);

	/*
	zoom_level--;
	scrollthingypos = my_clamp(scrollthingypos - scrollthingywidth / 2, 0, width - 2*SCROLLBUTTON_HEIGHT);
	calcScrollThingy();
	scrollthingypos = my_clamp(scrollthingypos, 0, width - 2*SCROLLBUTTON_HEIGHT+2 - scrollthingywidth);

	scrollpos = scrollthingypos;
	for(u8 i=0; i < zoom_level; ++i) scrollpos *= 2;

	draw();
	*/
}

u32 SampleDisplay::pixelToSample(s32 pixel)
{
	pixel=std::max(0,std::min(width-2,(int)pixel));
	u64 abspos = scrollpos + pixel;
	return u32( abspos * smp->getNSamples() / ( u64(width-2)<<zoom_level ) );
}

s32 SampleDisplay::sampleToPixel(u32 sample)
{
	return s32( sample * (s64(width-2) << zoom_level) / smp->getNSamples() - scrollpos );
}
