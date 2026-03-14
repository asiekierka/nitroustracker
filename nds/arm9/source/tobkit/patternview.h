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

#ifndef PATTERNVIEW_H
#define PATTERNVIEW_H

#include "tobkit/widget.h"
#include "ntxm/song.h"

#include "../state.h"

namespace tobkit {

#define PV_BORDER_WIDTH	10
#define PV_CURSORBAR_Y	88
#define PV_CELL_HEIGHT	8
#define PV_CHAR_WIDTH	4
#define PV_CHAR_HEIGHT	8

#define MUTE_REL_X	9
#define MUTE_X(i)		(PV_BORDER_WIDTH+(i)*getCellWidth()+MUTE_REL_X)
#define MUTE_Y		1
#define MUTE_WIDTH	10
#define MUTE_HEIGHT	9

#define SOLO_REL_X	20
#define SOLO_X(i)		(PV_BORDER_WIDTH+(i)*getCellWidth()+SOLO_REL_X)
#define SOLO_Y		MUTE_Y
#define SOLO_WIDTH	MUTE_WIDTH
#define SOLO_HEIGHT	MUTE_HEIGHT

#define C0	0
#define C1	1
#define C2	2
#define C3	3
#define C4	4
#define C5	5
#define C6	6
#define C7	7
#define C8	8
#define C9	9
#define A	10
#define B	11
#define C	12
#define D	13
#define E	14
#define F	15
#define G	16
#define H	17

#define L	21
#define M	22
#define P	25
#define R	27
#define S	28
#define U	30
#define V 	31

#define DOT	36
#define MINUS	37
#define SHARP	38
#define SPACE	39

#define VSLIDEUP 		41
#define VSLIDEDOWN 		MINUS
#define FVSLIDEDOWN 	43
#define FVSLIDEUP 		42
#define SETPANPOS		P
#define PANSLIDELEFT 	44
#define PANSLIDERIGHT 	45
#define NOTEPORTA		M
#define SETVIBRATOSPD	S
#define SETVIBRATO		V

const u8 notes_chars[] =   {12, 12, 13, 13, 14, 15, 15, 16, 16, 10, 10, 17};
const u8 notes_signs[] =   {0 , 1 , 0 , 1 , 0 , 0 , 1 , 0 , 1 , 0 , 1 ,  0};

// TODO: Define width/height of cells
// Make displayed info configurable (vol, effect)
// Make a getCellWidth function that determines cellwidth depending on the selected display options
// make an (inline) function pickcell(x,y, &cx, &cy) that gets the cell for a given touch position
// add possibility to select a rectangle of cells (draw selected cells in a special color)
// add cut+copy+paste buttons and functionality

class PatternView: public Widget {
	public:
		// Constructor sets base variables
		PatternView(u8 _x, u8 _y, u8 _width, u8 _height, uint16 **_vram, State *_state);
		
		// Drawing request
		void pleaseDraw(void);
		
		// Event calls
		void penDown(u8 px, u8 py);
		void penUp(u8 px, u8 py);
		void penMove(u8 px, u8 py);
		void buttonPress(u16 button);
		
		void updateSelection(void);
		
		// Fills the parameters with the selection coordinates. Returns true if no selection exists.
		bool getSelection(u16 *sel_x1, u16 *sel_y1, u16 *sel_x2, u16 *sel_y2);
		
		// Sets the selection to the given coordinates
		void setSelection(u16 sel_x1, u16 sel_y1, u16 sel_x2, u16 sel_y2);
		
		void clearSelection(void);
		
		void setSong(Song *s);
		void setLinesPerBeat(u16 lpb);
		
		void registerMuteCallback(void (*onMute_)(bool *channels_muted));
		
		void muteAll(void);
		void unmuteAll(void);
		
		// Returns the solo-ed channel, or -1 if no channel is solo
		s16 soloChannel(void);
		
		bool isMuted(u16 channel);
		void unmute(u16 channel);
		void toggleEffectsVisibility(bool on);

		void setTheme(Theme *theme_, u16 bgcolor_) { 
			theme = theme_; 
			bgcolor = bgcolor_;
			col_notes = theme_->col_pv_notes;
			col_instr = theme_->col_pv_instr;
			col_volume = theme_->col_pv_volume;
			col_effect = theme_->col_pv_effect;
			col_effect_param = theme_->col_pv_effect_param;
			col_notes_dark = theme_->col_pv_notes_dark;
			col_instr_dark = theme_->col_pv_instr_dark;
			col_volume_dark = theme_->col_pv_volume_dark;
			col_effect_dark = theme_->col_pv_effect_dark;
			col_effect_param_dark = theme_->col_pv_effect_param_dark;
		}
		void recalcHscroll(void);
		
	private:
		void draw(void);
		

		inline void drawHexByte(u8 byte, u8 cx, u8 cy, u16 col)
		{
			//drawSmallChar(byte/0x10, cx  , cy, col);
			//drawSmallChar(byte%0x10, cx+1, cy, col);
			drawSmallChar(byte/0x10, cx               , cy, col);
			drawSmallChar(byte%0x10, cx+PV_CHAR_WIDTH, cy, col);
		}

		inline void drawCell(u8 cellx, u8 celly, u8 px, u8 py, bool dark)
		{
			u16 notecol = dark?col_notes_dark:col_notes;
			u16 instrcol = dark?col_instr_dark:col_instr;
			u16 volumecol = dark?col_volume_dark:col_volume;
			u16 effectcol = dark?col_effect_dark:col_effect;
			u16 effectparamcol = dark?col_effect_param_dark:col_effect_param;
			/*
			typedef struct {
				u8 note;
				u8 instrument;
				u8 volume;
				u8 effect;
				u8 effect_param;
			} Cell;
			*/
			
			Cell *cell = &(pattern[cellx][celly]);

			u8 realx = PV_BORDER_WIDTH+1+px*getCellWidth();
			u8 realy = 2+py*PV_CELL_HEIGHT;

			// Check for empty note or stop-note
			if(cell->note == STOP_NOTE) {
				drawSmallChar(DOT,   realx                , realy, notecol);
				drawSmallChar(MINUS, realx+1*PV_CHAR_WIDTH, realy, notecol);
				drawSmallChar(DOT,   realx+2*PV_CHAR_WIDTH, realy, notecol);
			} else if(cell->note != EMPTY_NOTE) {
				// Note
				drawSmallChar(notes_chars[cell->note%12], realx, realy, notecol);
				if(notes_signs[cell->note%12]) {
					drawSmallChar(SHARP, realx+1*PV_CHAR_WIDTH, realy, notecol);
				} else {
					drawSmallChar(MINUS, realx+1*PV_CHAR_WIDTH, realy, notecol);
				}
				drawSmallChar(cell->note/12, realx+2*PV_CHAR_WIDTH, realy, notecol);
			}

			// Instrument
			if(cell->instrument != NO_INSTRUMENT)
				drawHexByte(cell->instrument+1, realx+3*PV_CHAR_WIDTH+1, realy, instrcol); // Adding one because FT2 indices start with 1
			
			if (cell->volume != NO_VOLUME)
			{
				drawHexByte(cell->volume, realx + 5 * PV_CHAR_WIDTH + 2, realy, volumecol);
			}

			// volume effect column slightly buggy and needs 
			// song.cpp adjustment in libntxm, lets leave that commented for now
			
			// else {
			// 	if (cell->effect2 != 0xff && cell->volume == NO_VOLUME)
			// 	{
			// 		char eff;
			// 		u8 vol = cell->volume_raw;
					
			// 		if ((vol >= 0x60) && (vol <= 0x6F)) // Volume slide down
			// 			eff = VSLIDEDOWN;
			// 		else if ((vol >= 0x70) && (vol <= 0x7F)) // Volume slide up
			// 			eff = VSLIDEUP;
			// 		else if ((vol >= 0x80) && (vol <= 0x8F)) // Fine volume slide down
			// 			eff = FVSLIDEDOWN;
			// 		else if ((vol >= 0x90) && (vol <= 0x9F)) // Fine volume slide up
			// 			eff = FVSLIDEUP;
			// 		else if ((vol >= 0xA0) && (vol <= 0xAF)) // Set vibrato speed (calls vibrato)
			// 			eff = SETVIBRATOSPD;
			// 		else if ((vol >= 0xB0) && (vol <= 0xBF)) // Vibrato
			// 			eff = SETVIBRATO;
			// 		else if ((vol >= 0xC0) && (vol <= 0xCF)) // Set panning
			// 			eff = SETPANPOS;
			// 		else if ((vol >= 0xD0) && (vol <= 0xDF)) // Panning slide left
			// 			eff = PANSLIDELEFT;
			// 		else if ((vol <= 0x1f)) // Panning slide right
			// 			eff = PANSLIDERIGHT;
			// 		else if (vol >= 0xF0) // Tone porta
			// 			eff = NOTEPORTA;
			// 		else {
			// 			eff = 0;
			// 		}
			// 		if (eff != 0) {
			// 			drawSmallChar(eff, realx + 5 * PV_CHAR_WIDTH + 2, realy, effectcol);
			// 			drawSmallChar(cell->effect2_param & 0x0F, realx + 6 * PV_CHAR_WIDTH + 2, realy, effectparamcol);
			// 		}
			// 	}
			// }
			
			if(effects_visible) {
				// Effect and effect parameter
				if (cell->effect != 0xff)
					drawSmallChar(cell->effect, realx+7*PV_CHAR_WIDTH+3, realy, effectcol);
						
				if (cell->effect_param != 0x00)
					drawHexByte(cell->effect_param, realx+8*PV_CHAR_WIDTH+3, realy, effectparamcol);
			}
		}
		
		void updateFromState(void);
	
		inline u8 getCellWidth(void)
		{
			if (effects_visible) {
				cell_width = 45;
			} else {
				cell_width = 32;
			}
			return cell_width;
		}

		inline u8 getEffectiveWidth(void)
		{
			return PV_BORDER_WIDTH + getNumVisibleChannels() * getCellWidth();
		}

		inline u8 getNumVisibleChannels(void)
		{
			u8 cw = (width-PV_BORDER_WIDTH) / getCellWidth();
			if(cw < song->getChannels()) {
				return cw;
			} else {
				return song->getChannels();
			}
		}

		inline u8 getNumVisibleRows(void)
		{
			return height / PV_CELL_HEIGHT;
		}

		inline u8 getCursorBarPos(void)
		{
			return getNumVisibleRows()/2-1;
		}

		
		void callMuteCallback(void);
		
		void (*onMute)(bool *channels_muted);
		
		bool pickCell(u8 px, u8 py, u16 *cx, u16 *cy);
		
		Cell **pattern;
		Song *song;
		State *state;

		u16 col_notes, col_instr, col_volume, col_effect, col_effect_param,
			col_notes_dark, col_instr_dark, col_volume_dark, col_effect_dark, col_effect_param_dark;
		
		u16 hscrollpos, lines_per_beat;
		
		bool selection_exists, pen_down;
		bool effects_visible;
		
		u8 cell_width;
		u8 px, py;
		u16 sel_start_x, sel_end_x, sel_start_y, sel_end_y;
		u16 sel_x, sel_y, sel_w, sel_h;
		
		bool solo_channels[32];
		bool mute_channels[32];
};

};

#endif
