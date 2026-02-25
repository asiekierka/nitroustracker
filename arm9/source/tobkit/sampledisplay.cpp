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

#include <cmath>
#include <cstdio>

#include "ntxm/ntxmtools.h"
#include "../tools.h"

using namespace tobkit;

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
	gfxSampleCursor = oamAllocateGfx(&oamSub, SpriteSize_16x32, SpriteColorFormat_16Color);

	gfxLoopHandles = oamAllocateGfx(&oamSub, SpriteSize_8x8, SpriteColorFormat_16Color);
	
	*(gfxSampleCursor+64)=1;
	
	oamSub.oamRotationMemory[0].vdy = 0; // max y scale

	for (int c=0;c<16;++c)
	{
		oamSet(&oamSub, c, 256, 23, 0, 0, 
		SpriteSize_16x32,
		   SpriteColorFormat_16Color,
		gfxSampleCursor, 
		   0, true, false, false, false, false);
	}

	const unsigned int loophandleTiles[8] __attribute__((aligned(4)))=
	{
		0x22000000,
		0x33200000,
		0x33320000,
		0x33332000,
		0x33333200,
		0x33333320,
		0x33333332,
		0x33333332
	};

	memcpy(gfxLoopHandles, loophandleTiles, 32);

	// handles
	for (int i=SPR_LOOPHANDLE_1_L;i<=SPR_LOOPHANDLE_2_R;++i)
	{
		oamSet(&oamSub, i,
		   199, 25,
		   0,
		   1,
		SpriteSize_8x8, SpriteColorFormat_16Color,
		gfxLoopHandles,
		   -1,
		   false,
		   false,
		   i % 2 == 1, i > 17, // h-flip every other handle, v-flip the last two handles
		   false);
	}

	// reuse cursors as part of the loop handles
	for (int i=SPR_LOOPLINE_1;i<=SPR_LOOPLINE_2;++i)
	{
		oamSet(&oamSub, i,
		   199, 0,
		   0, 
		   1,
		SpriteSize_16x32, SpriteColorFormat_16Color,
		gfxSampleCursor, 
		   0, 
		   true, 
		   false,
		   false, false,
		   false);
	}
}

SampleDisplay::~SampleDisplay(void)
{
	oamFreeGfx(&oamSub, gfxSampleCursor);
	oamFreeGfx(&oamSub, gfxLoopHandles);
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
				smp->setLoopStartAndLength(zerocrossing, smp->getLoopLength() - (zerocrossing - oldstart));
				draw();
			}
		}
	} else if(pen_on_loop_end_point) {
		if(snap_to_zero_crossings) {
			s32 zerocrossing = find_zero_crossing_near(smp->getLoopStart() + smp->getLoopLength());
			if(zerocrossing != -1) {
				s32 newlength = zerocrossing - smp->getLoopStart();
				u32 newstart = smp->getLoopStart();
				if(newlength < 0) {
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

void SampleDisplay::penMove(u8 px, u8 py)
{
	if((smp==0) || ( (active==false) && (loop_points_visible==false) && (draw_mode == false) ) )
		   return;

	if(pen_on_loop_start_point) {
		s32 olstart = smp->getLoopStart();
		s32 newstart = pixelToSample((s32)px-(s32)x-1-(s32)loop_touch_offset);
		smp->setLoopStartAndLength(newstart, std::max((s32)0, (s32)smp->getLoopLength() - (newstart - olstart)));
	}
	else if(pen_on_loop_end_point) {
		s32 newlength = (s32)pixelToSample((s32)px-(s32)x-1-(s32)loop_touch_offset) - smp->getLoopStart();
		u32 newstart = smp->getLoopStart();
		if(newlength < 0) {
			newstart += newlength;
			newlength = 0;
		}
		smp->setLoopStartAndLength(newstart, newlength);
	}
	
	else if(pen_on_scrollthingy)
	{
		scrollthingypos = ntxm_clamp(px - x - pen_x_on_scrollthingy - SCROLLBUTTON_HEIGHT, 0, width - 2*SCROLLBUTTON_HEIGHT+2 - scrollthingywidth);

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
			int draw_x = ntxm_clamp(px - x - 1, 0, width-2);
			int draw_y = ntxm_clamp(py - y - 1, 0, DRAW_HEIGHT-1);

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

void SampleDisplay::setCursorPosPtr(SampleCursor *cursorpos_)
{
	cursorpos = cursorpos_;
}

void SampleDisplay::calcCursor()
{
	if (smp == 0) return;
	for (int chn = 0; chn < 16; ++chn)
	{
		bool drawCursor = false;

		if (cursorpos[chn].instidx == 255 || !(cursorpos[chn].active)) 
		{
			oamSub.oamMemory[chn].x = 256;
			continue;
		}

		SampleCursor *playingNote = &cursorpos[chn];

		const u8 looptype = smp->getLoop();

		const u64 loopstart = (u64)(smp->getLoopStart()) << 32;
		const u64 looplen = (u64)(smp->getLoopLength()) << 32;
		const u64 nsamps = (u64)(smp->getNSamples()) << 32;
		const u64 loopend = loopstart + looplen;

		u8& looprev = playingNote->looprev;
		u64& playpos = playingNote->playbackpos;

		const u64 step = (((u64)(playingNote->playbackfreq) /* * (u64)n_ticks */) << 32) / 60; // timer

		// are we going backwards?
		if (looprev) {
			// calculate position if we are "bouncing" from the start
			if (step > playpos) {
				playpos = loopstart + step - playpos;
				looprev = false;
			}
			else
				playpos -= step;
		}
		else
			playpos += step;

		// if between frames the cursor jumped past the end of the sample put it in the correct position after the start
		if (looptype == FORWARD_LOOP && playpos >= loopend)
			playpos = loopstart + playpos - loopend;

		else if (looptype == PING_PONG_LOOP) {
			// If we're going backwards but we jumped beyond the first loop handle,
			// calculate where we should be. (loopstart + (loopstart - playpos)), where (loopstart - playpos) is the error
			if (looprev && playpos <= loopstart) {
				playpos = 2 * loopstart - playpos;
				looprev = false;
			}
			else if (!looprev && playpos >= loopend) {
				playpos = 2 * loopend - playpos;
				looprev = true;
			}
		}
		u16 draw_cursor_at=0;
		if (looptype == NO_LOOP && playpos >= nsamps) {
			cursorpos[chn].active=false;
			playpos = 0;
			drawCursor = false;
		}

		if (looptype != NO_LOOP && looplen <= step && playpos > loopstart)
			if (looptype == PING_PONG_LOOP && looprev)
				playpos = loopend - (step % looplen);
			else
				playpos = loopstart + (step % looplen);

		draw_cursor_at = sampleToPixel(playpos >> 32);

		if (draw_cursor_at != 0 && cursorpos[chn].instidx == instidx && cursorpos[chn].smpidx == smpidx)
			drawCursor = true;


		oamSub.oamMemory[chn].x = !drawCursor ? 199 : draw_cursor_at-3;
	}

}


void SampleDisplay::setSample(Sample *_smp, u8 _smpidx, u8 _instidx)
{
	bool changed = false;
	selection_exists = false;
	selstart = selend = 0;
	if(_smp == 0) {
		loop_points_visible = false;
	}

	if (smp != _smp || _smpidx != smpidx || instidx != _instidx)
		changed = true;

	smpidx = _smpidx;
	instidx = _instidx;
	smp = _smp;

	if (changed) draw();
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

void SampleDisplay::setTheme(Theme *theme_, u16 bgcolor_)
{
	*(SPRITE_PALETTE_SUB+1) = theme_->col_sample_cursor;

	// palette index 1
	*(SPRITE_PALETTE_SUB+2+16) = theme_->col_outline;
	*(SPRITE_PALETTE_SUB+3+16) = theme_->col_loop;
	*(SPRITE_PALETTE_SUB+1+16) = theme_->col_loop;

	Widget::setTheme(theme_, bgcolor_);
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


void SampleDisplay::reveal(void)
{
	oamEnable(&oamSub);
	Widget::reveal();
}

void SampleDisplay::occlude(void)
{
	oamDisable(&oamSub);

	Widget::occlude();
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

	// Border
	if(active==false) {
		drawBorder(theme->col_outline);
	} else {
		drawBorder(theme->col_signal);
	}

	// Now comes sample-dependant stuff, so return if we have no sample
	if((smp==0)||(smp->getNSamples()==0)) {
		drawFullBox(1, 1, width - 2, height - 2, theme->col_smp_bg);
		return;
	}

	//
	// Selection
	//
	s32 selleft = 0;
	s32 selright = 0;
	bool draw_selection = selection_exists;

	if(draw_selection) {
		selleft = sampleToPixel(std::min(selstart, selend));
		selright = sampleToPixel(std::max(selstart, selend));

		if (selleft < 1) selleft = 1;
		else if (selleft > (width-1)) draw_selection = false;

		if (selright > width-1) selright = width-1;
		else if (selright < 1) draw_selection = false;
	}

	//
	// Scrollbar
	//

	// Right Button
	if(pen_on_scroll_right) {
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
	if(pen_on_scroll_left) {
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
	if(pen_on_scrollthingy) {
		drawFullBox(SCROLLBUTTON_HEIGHT+scrollthingypos, height-SCROLLBAR_WIDTH+1, scrollthingywidth-2, SCROLLBAR_WIDTH-2, theme->col_scrollbar_active);
	} else {
		drawFullBox(SCROLLBUTTON_HEIGHT+scrollthingypos, height-SCROLLBAR_WIDTH+1, scrollthingywidth-2, SCROLLBAR_WIDTH-2, theme->col_scrollbar_inactive);
	}

	drawBox(SCROLLBUTTON_HEIGHT-1+scrollthingypos, height-SCROLLBAR_WIDTH, scrollthingywidth, SCROLLBAR_WIDTH, theme->col_outline);

	//
	// Sample
	//

	/* u16 colortable[DRAW_HEIGHT+2];
	u16 colortable_selected[DRAW_HEIGHT+2];
	for(s32 i=0; i<DRAW_HEIGHT+2; i++) {
		// colortable[i] = interpolateColor(theme->col_light_ctrl, theme->col_dark_ctrl, i<<4);
		colortable[i] = theme->col_smp_waveform;
		// colortable_selected[i] = ((colortable[i] >> 2) & 0x1CE7) | 0x8000;
		colortable_selected[i] = theme->col_smp_waveform_sel;
	} */

	int32 step = divf32(inttof32(smp->getNSamples() >> zoom_level), inttof32(width-2));
	int32 pos = 0;

	u32 renderwindow = (u32)std::max(1, std::min(100, (int) ceil_f32toint(step)));

	u16 middle = (DRAW_HEIGHT+2)/2;//-1;
	u16 top = (DRAW_HEIGHT+2);

	s32 lastmax=0, lastmin=0;
	if(smp->is16bit() == true) {

		s16 *data;
		s16 *base = (s16*)smp->getData() + pixelToSample(0);

		for(s32 i=1; i<s32(width-1); ++i)
		{
			bool draw_selection_here = (draw_selection && i >= selleft && i <= selright);
			u16 colortable_current = draw_selection_here ? theme->col_smp_waveform_sel : theme->col_smp_waveform;
			u16 bg_current = draw_selection_here ? theme->col_smp_bg_sel : theme->col_smp_bg;
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

			lastmax = maxy;
			lastmin = miny;

			miny += middle;
			maxy += middle;
			if (miny > maxy) {
				miny = middle;
				maxy = middle;
			}

			s16 j;
			for(j=0; j<miny;  ++j) (*vram)[SCREEN_WIDTH*(y+top-j)+x+i] = bg_current;
			for(;    j<=maxy; ++j) (*vram)[SCREEN_WIDTH*(y+top-j)+x+i] = colortable_current; /* [top-j]; */
			for(;    j<top;   ++j) (*vram)[SCREEN_WIDTH*(y+top-j)+x+i] = bg_current;

			pos += step;
		}

	} else {

		s8 *data;
		s8 *base = (s8*)smp->getData() + pixelToSample(0);

		for(s32 i=1; i<s32(width-1); ++i)
		{
			bool draw_selection_here = (draw_selection && i >= selleft && i <= selright);
			u16 colortable_current = draw_selection_here ? theme->col_smp_waveform_sel : theme->col_smp_waveform;
			u16 bg_current = draw_selection_here ? theme->col_smp_bg_sel : theme->col_smp_bg;
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

			lastmax = maxy;
			lastmin = miny;

			miny += middle;
			maxy += middle;
			if (miny > maxy) {
				miny = middle;
				maxy = middle;
			}

			s16 j;
			for(j=0; j<miny;  ++j) (*vram)[SCREEN_WIDTH*(y+top-j)+x+i] = bg_current;
			for(;    j<=maxy; ++j) (*vram)[SCREEN_WIDTH*(y+top-j)+x+i] = colortable_current; /* [top-j]; */
			for(;    j<top;   ++j) (*vram)[SCREEN_WIDTH*(y+top-j)+x+i] = bg_current;

			pos += step;
		}

	}

	//
	// Loop Points
	//

	u32 loop_start_pos = sampleToPixel(smp->getLoopStart());
	u32 loop_end_pos   = sampleToPixel(smp->getLoopStart() + smp->getLoopLength());

	bool draw_loop_end = ( (loop_end_pos >= 0) && (loop_end_pos <= width-2) );
	bool draw_loop_start = (loop_start_pos >= 0) && (loop_start_pos <= width-2);

	if( (loop_points_visible) && (smp->getLoop() != NO_LOOP) && !draw_mode )
	{
		if(draw_loop_start)
		{
			oamSetXY(&oamSub, SPR_LOOPHANDLE_1_L, loop_start_pos-2, DRAW_HEIGHT+18);
			oamSetXY(&oamSub, SPR_LOOPHANDLE_1_R, loop_start_pos-2+8-1, DRAW_HEIGHT+18);
			oamSetXY(&oamSub, SPR_LOOPLINE_1, loop_start_pos-2-1, y);
		}
		else
			oamSetXY(&oamSub, SPR_LOOPLINE_1, width+1, y);

		if(draw_loop_end)
		{
			oamSetXY(&oamSub, SPR_LOOPHANDLE_2_L, loop_end_pos-2, y);
			oamSetXY(&oamSub, SPR_LOOPHANDLE_2_R, loop_end_pos-2+8-1, y);
			oamSetXY(&oamSub, SPR_LOOPLINE_2, loop_end_pos-2-1, y);
		}
		else
			oamSetXY(&oamSub, SPR_LOOPLINE_2, width+1, y);
	}

	oamSetHidden(&oamSub, SPR_LOOPHANDLE_1_L, !draw_loop_start);
	oamSetHidden(&oamSub, SPR_LOOPHANDLE_1_R, !draw_loop_start);
	oamSetHidden(&oamSub, SPR_LOOPHANDLE_2_L, !draw_loop_end);
	oamSetHidden(&oamSub, SPR_LOOPHANDLE_2_R, !draw_loop_end);	

	//
	// Zoom buttons
	//

	if(!draw_mode) {
		// Outlines
		drawHLine(2, 1, 7, theme->col_smp_zoom);
		drawHLine(10, 1, 7, theme->col_smp_zoom);

		drawHLine(2, 9, 7, theme->col_smp_zoom);
		drawHLine(10, 9, 7, theme->col_smp_zoom);

		drawVLine(1, 2, 7, theme->col_smp_zoom);
		drawVLine(9, 2, 7, theme->col_smp_zoom);
		drawVLine(17, 2, 7, theme->col_smp_zoom);

		// +
		if(pen_on_zoom_in) {
			drawFullBox(2, 2, 7, 7, theme->col_smp_zoom);
			drawHLine(3, 5, 5, theme->col_smp_bg);
			drawVLine(5, 3, 5, theme->col_smp_bg);
		} else {
			drawHLine(3, 5, 5, theme->col_smp_zoom);
			drawVLine(5, 3, 5, theme->col_smp_zoom);
		}

		// -
		if(pen_on_zoom_out) {
			drawFullBox(10, 2, 7, 7, theme->col_smp_zoom);
			drawHLine(11, 5, 5, theme->col_smp_bg);
		} else {
			drawHLine(11, 5, 5, theme->col_smp_zoom);
		}
	}
}

void SampleDisplay::scroll(u32 newscrollpos)
{
	u32 window_width = width - 2;
	u64 disp_width = (u64)window_width << zoom_level;
	u32 scroll_width = width - 2*SCROLLBUTTON_HEIGHT+2 - scrollthingywidth;

	scrollpos = ntxm_clamp(newscrollpos, 0, disp_width - window_width);
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
	pixel=std::max(0,std::min(width-2,(int)pixel));
	u64 abspos = scrollpos + pixel;
	return u32( abspos * smp->getNSamples() / ( u64(width-2)<<zoom_level ) );
}

s32 SampleDisplay::sampleToPixel(u32 sample)
{
	return s32( sample * (s64(width-2) << zoom_level) / smp->getNSamples() - scrollpos );
}
