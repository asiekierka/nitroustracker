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

#include "patternview.h"
#include "../tools.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/* ===================== PUBLIC ===================== */

// Constructor sets base variables
PatternView::PatternView(u8 _x, u8 _y, u8 _width, u8 _height, uint16 **_vram, State *_state)
	:Widget(_x, _y, _width, _height, _vram),
	onMute(0), pattern(0), song(0), state(_state),
	col_lines(RGB15(16,18,24)|BIT(15)),
	col_sublines(RGB15(7,9,17)|BIT(15)),
	col_lines_record(RGB15(31,18,0)|BIT(15)),
	cb_col1(RGB15(9,11,17)|BIT(15)),
	cb_col2(RGB15(16,18,24)|BIT(15)),
	cb_col1_highlight(RGB15(28,15,0)|BIT(15)),
	cb_col2_highlight(RGB15(28,28,0)|BIT(15)),
	col_left_numbers(RGB15(28,15,0)|BIT(15)),
	col_notes(RGB15(9,15,31)|BIT(15)), /* RGB15(3,11,31) */
	col_instr(RGB15(31,11,0)|BIT(15)),
	col_volume(RGB15(0,27,0)|BIT(15)),
	col_effect(RGB15(10,5,31)|BIT(15)),
	col_effect_param(RGB15(15,10,25)|BIT(15)),
	col_notes_dark(RGB15(0,6,26)|BIT(15)),
	col_instr_dark(RGB15(20,6,0)|BIT(15)),
	col_volume_dark(RGB15(0,16,0)|BIT(15)),
	col_effect_dark(RGB15(5,2,25)|BIT(15)),
	col_effect_param_dark(RGB15(7,5,20)|BIT(15)),
	col_bg(RGB15(4,6,15)|BIT(15)),
	cb_sel_highlight(RGB15(31,24,0)|BIT(15)),
	hscrollpos(0), lines_per_beat(8), selection_exists(false), pen_down(false),
	effects_visible(true), cell_width(50)
{
	for(int i=0;i<32;++i)
	{
		mute_channels[i] = false;
		solo_channels[i] = false;
	}
}

// Drawing request
void PatternView::pleaseDraw(void)
{
	updateFromState();

	draw();
}
	
// Event calls
void PatternView::penDown(u8 px, u8 py)
{
	pen_down = true;
	
	// Selection
	if(py > y + MUTE_Y) // To avoid selecing stuff when pressing the mute/solo buttons
	{
		selection_exists = pickCell(px, py, &sel_start_x, &sel_start_y);
		if(selection_exists == true)
		{
			sel_x = sel_start_x;
			sel_y = sel_start_y;
			sel_w = sel_h = 1;
		}
	}
		
	// Mute / Solo buttons
	u8 realx = px - (x+PV_BORDER_WIDTH);
	s32 cellx = realx / getCellWidth() + hscrollpos;
	u8 rel_cell_x = realx % getCellWidth();
	
	// Mute
	if( ( soloChannel() == -1 ) && // No muting when a channel is solo
		( py >= y + MUTE_Y) && ( py <= y + MUTE_Y + MUTE_HEIGHT ) &&
		( rel_cell_x >= MUTE_REL_X ) && ( rel_cell_x <= MUTE_REL_X + MUTE_WIDTH ) )
	{
		mute_channels[cellx] = !mute_channels[cellx];
		
		callMuteCallback();
	}
	
	// Solo
	if( ( py >= y + SOLO_Y) && ( py <= y + SOLO_Y + SOLO_HEIGHT ) &&
		( rel_cell_x >= SOLO_REL_X ) && ( rel_cell_x <= SOLO_REL_X + SOLO_WIDTH ) )
	{
		solo_channels[cellx] = !solo_channels[cellx];
		
		callMuteCallback();
	}
}

void PatternView::penUp(u8 px, u8 py)
{
	pen_down = false;
}

void PatternView::penMove(u8 px, u8 py)
{
	this->px = px;
	this->py = py;
	updateSelection();
}

void PatternView::buttonPress(u16 button)
{

}

void PatternView::updateSelection(void)
{
	if(pen_down == false) return;
	
	if( pickCell(px, py, &sel_end_x, &sel_end_y) == true ) {
		
		if(sel_end_x > sel_start_x) {
			sel_x = sel_start_x;
			sel_w = sel_end_x - sel_start_x + 1;
		} else {
			sel_x = sel_end_x;
			sel_w = sel_start_x - sel_end_x + 1;
		}
		
		if(sel_end_y > sel_start_y) {
			sel_y = sel_start_y;
			sel_h = sel_end_y - sel_start_y + 1;
		} else {
			sel_y = sel_end_y;
			sel_h = sel_start_y - sel_end_y + 1;
		}
	}
}

bool PatternView::getSelection(u16 *sel_x1, u16 *sel_y1, u16 *sel_x2, u16 *sel_y2)
{
	if(selection_exists == false) return false;
	*sel_x1 = sel_x;
	*sel_x2 = sel_x+sel_w-1;
	*sel_y1 = sel_y;
	*sel_y2 = sel_y+sel_h-1;
	return true;
}

// Sets the selection to the given coordinates
void PatternView::setSelection(u16 sel_x1, u16 sel_y1, u16 sel_x2, u16 sel_y2)
{
	sel_x = sel_x1;
	sel_y = sel_y1;
	sel_w = sel_x2 - sel_x1 + 1;
	sel_h = sel_y2 - sel_y1 + 1;
	selection_exists = true;
}

void PatternView::clearSelection(void)
{
	selection_exists = false;
}

void PatternView::setSong(Song *s)
{
	song = s;
	pattern = s->getPattern(song->getPotEntry(state->potpos));
	hscrollpos = 0;
}

void PatternView::registerMuteCallback(void (*onMute_)(bool *channels_muted))
{
	onMute = onMute_;
}

void PatternView::muteAll(void)
{
	for(int chn=0; chn<MAX_CHANNELS; ++chn)
	{
		mute_channels[chn] = true;
		solo_channels[chn] = false;
	}
	callMuteCallback();
}

void PatternView::unmuteAll(void)
{
	for(int chn=0; chn<MAX_CHANNELS; ++chn)
	{
		mute_channels[chn] = false;
		solo_channels[chn] = false;
	}
	callMuteCallback();
}

s16 PatternView::soloChannel(void)
{
	for(u8 c=0;c<MAX_CHANNELS;++c)
	{
		if(solo_channels[c] == true)
			return c;
	}
	return -1;
}

bool PatternView::isMuted(u16 channel)
{
	return mute_channels[channel];
}

void PatternView::unmute(u16 channel)
{
	mute_channels[channel] = false;
	callMuteCallback();
}

void PatternView::toggleEffectsVisibility(bool on)
{
  effects_visible = on;
}

/* ===================== PRIVATE ===================== */

// This is a fullscreen widget so it does not give a damn
// about the specified dimensions, it will just overdraw
// the whole screen.
void PatternView::draw(void)
{
	u32 colcol = col_bg | col_bg << 16;
	s32 sel_screen_x1 = -1, sel_screen_x2 = -1, sel_screen_y1 = -1, sel_screen_y2 = -1;

	dmaFillWords(colcol, *vram, 192*256*2);
	
	// Selection
	if(selection_exists == true) {
		
		// Calculate dimensions in screen coords
		sel_screen_x1 = PV_BORDER_WIDTH + (sel_x - hscrollpos) * getCellWidth();
		sel_screen_x2 = sel_screen_x1 + sel_w * getCellWidth();
		sel_screen_y1 = (sel_y - state->getCursorRow() + getCursorBarPos()) * PV_CELL_HEIGHT;
		sel_screen_y2 = sel_screen_y1 + sel_h * PV_CELL_HEIGHT;
		
		// Does an intersection with the screen exist?
		if	(!(	(sel_screen_x1 >= getEffectiveWidth()) ||
				(sel_screen_x2 <= 0) ||
				(sel_screen_y1 >= height) ||
				(sel_screen_y2 <= 0) ) ) {
		
			// Clamp to screen
			if (sel_screen_x1 < 0) sel_screen_x1 = 0;
			if (sel_screen_x2 > getEffectiveWidth()) sel_screen_x2 = getEffectiveWidth();
			if (sel_screen_y1 < 0) sel_screen_y1 = 0;
			if (sel_screen_y2 > height) sel_screen_y2 = height;
			
			// Draw
			drawFullBox(sel_screen_x1, sel_screen_y1, sel_screen_x2 - sel_screen_x1,
				    sel_screen_y2 - sel_screen_y1 + 1, cb_sel_highlight);
		}
	}
	
	// H-Lines
	u16 linescol;
	if(state->recording == true) {
		linescol = col_lines_record;
	} else {
		linescol = col_lines;
	}
	
	s16 realrow;
	u16 ptnlen = song->getPatternLength(song->getPotEntry(state->potpos));
	for(u16 i=0; i<=getNumVisibleRows(); ++i) {
		realrow = i-getCursorBarPos()+state->getCursorRow();
	
		if((realrow>=0)&&(realrow<=ptnlen)) {
			if(realrow%lines_per_beat==0) {
				drawHLine(0, PV_CELL_HEIGHT*i, getEffectiveWidth(), linescol);
			} else {
				drawHLine(PV_BORDER_WIDTH, PV_CELL_HEIGHT*i,
					 getEffectiveWidth()-PV_BORDER_WIDTH, col_sublines);
			}
		}
	}
	
	// V-Lines
	for(u16 i=0;i<=getNumVisibleChannels();++i) {
		drawVLine(PV_BORDER_WIDTH-1+i*getCellWidth(), 0, width-PV_BORDER_WIDTH, linescol);
	}
	
	// Cursor bar (highlight)
	drawGradient(cb_col1, cb_col2, 0, PV_CURSORBAR_Y, getEffectiveWidth(), PV_CELL_HEIGHT);
	if(selection_exists == true) {
		// Cursor bar (highlighted component)
		if (	(sel_screen_y1 <= PV_CURSORBAR_Y + 1)
			&&	(sel_screen_y2 >= PV_CURSORBAR_Y + 1 + PV_CELL_HEIGHT-1)
			&&	(sel_screen_x1 <= getEffectiveWidth())
			&&	(sel_screen_x2 >= PV_BORDER_WIDTH)) {
				s32 cursor_highlight_x1 = std::max((int) sel_screen_x1, PV_BORDER_WIDTH + 1);
				drawFullBox(cursor_highlight_x1, PV_CURSORBAR_Y+1, sel_screen_x2 - cursor_highlight_x1 - 1, PV_CELL_HEIGHT-1, cb_sel_highlight);
		}
	}

	// Playback box
	int playback_box_y = PV_CURSORBAR_Y + (state->getPlaybackRow()-state->getCursorRow())*PV_CELL_HEIGHT;
	if (playback_box_y >= 0 && playback_box_y < 192) {
		drawBox(0, playback_box_y, getEffectiveWidth(), PV_CELL_HEIGHT+1);
	}

	// Cursor
	drawBox(PV_BORDER_WIDTH-1+(state->channel-hscrollpos)*getCellWidth(), PV_CURSORBAR_Y, getCellWidth()+1, PV_CELL_HEIGHT+1);
	drawGradient(cb_col1_highlight, cb_col2_highlight, PV_BORDER_WIDTH+(state->channel-hscrollpos)*getCellWidth(),
				 PV_CURSORBAR_Y+1, getCellWidth()-1, PV_CELL_HEIGHT-1);
	
	// Numbers on the left
	s16 ip;
	for(ip=state->getCursorRow()-getCursorBarPos();ip<=state->getCursorRow()+getCursorBarPos()+1;++ip) {
		if((ip>=0)&&(ip<ptnlen)) {
			drawHexByte(ip, 1, 2+(ip+getCursorBarPos()-state->getCursorRow())*PV_CHAR_HEIGHT, col_left_numbers);
		}
	}
	
	// Pattern data
	s16 firstrow = state->getCursorRow()-getCursorBarPos();
	int highlight_row = getNumVisibleRows() / 2 - 1;
	
	for(u16 i=0;i<getNumVisibleChannels();++i)
	{
		for(u16 j=0;j<getNumVisibleRows();++j)
		{
			if((firstrow+j>=0)&&(firstrow+j<ptnlen))
			{
				//drawCell(hscrollpos+i, firstrow+j, (i*getCellWidth()+PV_BORDER_WIDTH+2)/PV_CHAR_WIDTH/*i*14+3*/, j);
				if(j == highlight_row)
					drawCell(hscrollpos+i, firstrow+j, i, j, true);
				else
					drawCell(hscrollpos+i, firstrow+j, i, j, false);
			}
		}
	}
	
	// Channel indices
	char numberstr[3];
	for(u16 i=0;i<getNumVisibleChannels();++i)
	{
		drawFullBox(PV_BORDER_WIDTH+i*getCellWidth()+1, 1, 14, 11, col_bg);
		sniprintf(numberstr, sizeof(numberstr), "%-2x", (u8) (hscrollpos+i));
		drawString(numberstr, PV_BORDER_WIDTH+i*getCellWidth()+1, 1, 255, col_lines);
	}
	
	// Mute / Solo buttons
	u16 mute_col1, mute_col2, solo_col1, solo_col2;
	
	for(u8 i=0;i<getNumVisibleChannels();++i)
	{
		u8 chn = hscrollpos+i;
		
		if( soloChannel() == -1 )
		{
			if(mute_channels[chn] == true)
			{
				mute_col1 = cb_col1_highlight;
				mute_col2 = cb_col2_highlight;
			}
			else
			{
				mute_col1 = cb_col1;
				mute_col2 = cb_col2;
			}
			drawGradient(mute_col1, mute_col2, MUTE_X(i), MUTE_Y, MUTE_WIDTH, MUTE_HEIGHT);
			drawString("m", PV_BORDER_WIDTH+i*getCellWidth()+9, 0, 255);
		}
		
		if(solo_channels[chn] == true)
		{
			solo_col1 = cb_col1_highlight;
			solo_col2 = cb_col2_highlight;
		}
		else
		{
			solo_col1 = cb_col1;
			solo_col2 = cb_col2;
		}
		
		drawGradient(solo_col1, solo_col2, SOLO_X(i), SOLO_Y, SOLO_WIDTH, SOLO_HEIGHT);
		drawString("s", PV_BORDER_WIDTH+i*getCellWidth()+20, 0, 255);
	}
}

// This updates interval variables befoe drawing
void PatternView::updateFromState(void)
{
	// Pattern
	pattern = song->getPattern(song->getPotEntry(state->potpos));
	
	// Horizontal scroll position
	if(state->channel<hscrollpos) {
		hscrollpos--;
	}

	if(state->channel==hscrollpos+getNumVisibleChannels()) {
		hscrollpos++;
	}
	
	// Scroll back if number of patterns has decreased and the cursor was at the end
	if((hscrollpos+getNumVisibleChannels() > song->getChannels())&&(hscrollpos>0)) {
		hscrollpos = song->getChannels() - getNumVisibleChannels();
	}
}

bool PatternView::pickCell(u8 px, u8 py, u16 *cx, u16 *cy)
{
	if( (px < x+PV_BORDER_WIDTH) || (px > x+getEffectiveWidth()) ) {
		return false;
	} else {
		u8 realx = px - (x+PV_BORDER_WIDTH);
		u8 realy = py - y - 1;
		s32 cellx = realx / getCellWidth() + hscrollpos;
		s32 celly = realy / PV_CELL_HEIGHT - getCursorBarPos() + state->getCursorRow();
		if((celly < 0) || (celly >= song->getPatternLength(song->getPotEntry(state->potpos)))) {
			return false;
		}
		*cx = cellx;
		*cy = celly;
		
		return true;
	}
}

void PatternView::callMuteCallback(void)
{
	if(onMute == 0)
		return;
	
	bool muted_channels[32];
	
	if( soloChannel() != -1 )
	{
		for(u8 chn=0; chn<32; ++chn)
		{
			muted_channels[chn] = !solo_channels[chn];
		}
	}
	else
	{
		for(u8 chn=0; chn<32; ++chn)
		{
			muted_channels[chn] = mute_channels[chn];
		}
	}
	
	onMute(muted_channels);
}
