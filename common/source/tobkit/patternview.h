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

#include "fxkeyboard.h"
#include "ntxm/song.h"
#include "tobkit/widget.h"

#include "state.h"

namespace tobkit
{

#define PV_BORDER_WIDTH 10
#define PV_CELL_HEIGHT 8
#define PV_CHAR_WIDTH 4
#define PV_CHAR_HEIGHT 8
#define PV_CELL_NOTE_X (1)
#define PV_CELL_NOTE_WIDTH (PV_CHAR_WIDTH * 3)
#define PV_CELL_INST_X (PV_CELL_NOTE_X + PV_CELL_NOTE_WIDTH + 1)
#define PV_CELL_INST_WIDTH (PV_CHAR_WIDTH * 2)
#define PV_CELL_VOL_X (PV_CELL_INST_X + PV_CELL_INST_WIDTH + 1)
#define PV_CELL_VOL_WIDTH (PV_CHAR_WIDTH * 2)
#define PV_CELL_FX_X (PV_CELL_VOL_X + PV_CELL_VOL_WIDTH + 1)
#define PV_CELL_FX_WIDTH (PV_CHAR_WIDTH * 3)
#define PV_CELL_WIDTH PV_CELL_FX_X
#define PV_CELL_WIDTH_FX (PV_CELL_FX_X + PV_CELL_FX_WIDTH + 1)

#define MUTE_REL_X 9
#define MUTE_X(i) (PV_BORDER_WIDTH + (i) * getCellWidth() + MUTE_REL_X)
#define MUTE_Y 1
#define MUTE_WIDTH 10
#define MUTE_HEIGHT 9

#define SOLO_REL_X 20
#define SOLO_X(i) (PV_BORDER_WIDTH + (i) * getCellWidth() + SOLO_REL_X)
#define SOLO_Y MUTE_Y
#define SOLO_WIDTH MUTE_WIDTH
#define SOLO_HEIGHT MUTE_HEIGHT

#define DOT GLYPH_3X5('$')
#define MINUS GLYPH_3X5('-')
#define SHARP GLYPH_3X5('#')
#define SPACE GLYPH_3X5(' ')

#define VSLIDEUP GLYPH_3X5('+')
#define VSLIDEDOWN MINUS
#define FVSLIDEDOWN GLYPH_3X5(';')
#define FVSLIDEUP GLYPH_3X5('&')
#define SETPANPOS GLYPH_3X5('P')
#define PANSLIDELEFT GLYPH_3X5('<')
#define PANSLIDERIGHT GLYPH_3X5('>')
#define NOTEPORTA GLYPH_3X5('M')
#define SETVIBRATOSPD GLYPH_3X5('S')
#define SETVIBRATO GLYPH_3X5('V')

const u8 notes_chars[] = {GLYPH_3X5('C'), GLYPH_3X5('C'), GLYPH_3X5('D'), GLYPH_3X5('D'), GLYPH_3X5('E'), GLYPH_3X5('F'), GLYPH_3X5('F'), GLYPH_3X5('G'), GLYPH_3X5('G'), GLYPH_3X5('A'), GLYPH_3X5('A'), GLYPH_3X5('H')};
const u8 notes_signs[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
const u8 hex_chars[] = {
	GLYPH_3X5('0'), GLYPH_3X5('1'), GLYPH_3X5('2'), GLYPH_3X5('3'),
	GLYPH_3X5('4'), GLYPH_3X5('5'), GLYPH_3X5('6'), GLYPH_3X5('7'),
	GLYPH_3X5('8'), GLYPH_3X5('9'), GLYPH_3X5('A'), GLYPH_3X5('B'),
	GLYPH_3X5('C'), GLYPH_3X5('D'), GLYPH_3X5('E'), GLYPH_3X5('F'),
	GLYPH_3X5('G'), GLYPH_3X5('H'), GLYPH_3X5('I'), GLYPH_3X5('J'),
	GLYPH_3X5('K'), GLYPH_3X5('L'), GLYPH_3X5('M'), GLYPH_3X5('N'),
	GLYPH_3X5('O'), GLYPH_3X5('P'), GLYPH_3X5('Q'), GLYPH_3X5('R'),
	GLYPH_3X5('S'), GLYPH_3X5('T'), GLYPH_3X5('U'), GLYPH_3X5('V'),
	GLYPH_3X5('W'), GLYPH_3X5('X'), GLYPH_3X5('Y'), GLYPH_3X5('Z')
};

#define PV_COMPONENT_NOTE 0
#define PV_COMPONENT_INSTRUMENT 1
#define PV_COMPONENT_VOLUME 2
#define PV_COMPONENT_EFFECT 3
#define PV_COMPONENT_EFFECT_PARAM 4

// TODO: Define width/height of cells
// Make displayed info configurable (vol, effect)
// Make a getCellWidth function that determines cellwidth depending on the selected display options
// make an (inline) function pickcell(x,y, &cx, &cy) that gets the cell for a given touch position
// add possibility to select a rectangle of cells (draw selected cells in a special color)
// add cut+copy+paste buttons and functionality

class PatternView : public Widget
{
public:
	// Constructor sets base variables
	PatternView(u16 _x, u16 _y, u16 _width, u16 _height, Screen *_screen,
	            State *_state);

	// Drawing request
	void pleaseDraw(void);

	void setSize(u16 _width, u16 _height);

	// Event calls
	void penDown(u16 px, u16 py);
	void penUp(u16 px, u16 py);
	void penMove(u16 px, u16 py);
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

	void setTheme(Theme *theme_, u16 bgcolor_)
	{
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

	inline bool isPerComponentNav() { return componentnav; }
	inline void setPerComponentNav(bool value) { componentnav = value; }

	inline int getComponentNavOffset() { return componentpos; }
	inline void setComponentNavOffset(int value) { componentpos = value; }

	inline int getMaxComponentNavOffset()
	{
		return effects_visible ? PV_COMPONENT_EFFECT_PARAM
		                       : PV_COMPONENT_VOLUME;
	}

private:
	void draw(void);

	inline void drawHexByte(u8 byte, u16 cx, u16 cy, u16 col)
	{
		drawSmallChar(hex_chars[byte >> 4], cx, cy, col);
		drawSmallChar(hex_chars[byte & 0xF], cx + PV_CHAR_WIDTH, cy, col);
	}

	inline void drawCell(u16 cellx, u16 celly, u16 px, u16 py, u8 dark)
	{
		u16 notecol = (dark & 0x1) ? col_notes_dark : col_notes;
		u16 instrcol = (dark & 0x2) ? col_instr_dark : col_instr;
		u16 volumecol = (dark & 0x4) ? col_volume_dark : col_volume;
		u16 effectcol = (dark & 0x8) ? col_effect_dark : col_effect;
		u16 effectparamcol =
		    (dark & 0x10) ? col_effect_param_dark : col_effect_param;
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

		u16 realx = PV_BORDER_WIDTH + px * getCellWidth();
		u16 realy = 2 + py * PV_CELL_HEIGHT;

		// Check for empty note or stop-note
		if (cell->note == STOP_NOTE) {
			drawSmallChar(DOT, realx + PV_CELL_NOTE_X, realy, notecol);
			drawSmallChar(MINUS, realx + PV_CELL_NOTE_X + PV_CHAR_WIDTH, realy,
			              notecol);
			drawSmallChar(DOT, realx + PV_CELL_NOTE_X + 2 * PV_CHAR_WIDTH,
			              realy, notecol);
		} else if (cell->note != EMPTY_NOTE) {
			// Note
			drawSmallChar(notes_chars[cell->note % 12], realx + PV_CELL_NOTE_X,
			              realy, notecol);
			if (notes_signs[cell->note % 12]) {
				drawSmallChar(SHARP, realx + PV_CELL_NOTE_X + PV_CHAR_WIDTH,
				              realy, notecol);
			} else {
				drawSmallChar(MINUS, realx + PV_CELL_NOTE_X + PV_CHAR_WIDTH,
				              realy, notecol);
			}
			drawSmallChar(GLYPH_3X5('0' + (cell->note / 12)),
			              realx + PV_CELL_NOTE_X + 2 * PV_CHAR_WIDTH, realy,
			              notecol);
		}

		// Instrument
		if (cell->instrument != NO_INSTRUMENT)
			drawHexByte(
			    cell->instrument + 1, realx + PV_CELL_INST_X, realy,
			    instrcol); // Adding one because FT2 indices start with 1

		u8 vol = cell->volume;
		if (vol >= 0x10 && vol <= 0x5F) {
			drawHexByte(vol - 0x10, realx + PV_CELL_VOL_X, realy, volumecol);
		} else if (vol >= 0x60) {
			u8 eff = 0;
			if ((vol >= 0x60) && (vol <= 0x6F)) // Volume slide down
				eff = VSLIDEDOWN;
			else if ((vol >= 0x70) && (vol <= 0x7F)) // Volume slide up
				eff = VSLIDEUP;
			else if ((vol >= 0x80) && (vol <= 0x8F)) // Fine volume slide down
				eff = FVSLIDEDOWN;
			else if ((vol >= 0x90) && (vol <= 0x9F)) // Fine volume slide up
				eff = FVSLIDEUP;
			else if ((vol >= 0xA0) &&
			         (vol <= 0xAF)) // Set vibrato speed (calls vibrato)
				eff = SETVIBRATOSPD;
			else if ((vol >= 0xB0) && (vol <= 0xBF)) // Vibrato
				eff = SETVIBRATO;
			else if ((vol >= 0xC0) && (vol <= 0xCF)) // Set panning
				eff = SETPANPOS;
			else if ((vol >= 0xD0) && (vol <= 0xDF)) // Panning slide left
				eff = PANSLIDELEFT;
			else if ((vol >= 0xE0) && (vol <= 0xEF)) // Panning slide right
				eff = PANSLIDERIGHT;
			else if (vol >= 0xF0) // Tone porta
				eff = NOTEPORTA;

			if (eff != 0) {
				drawSmallChar(hex_chars[eff], realx + PV_CELL_VOL_X, realy, effectcol);
				drawSmallChar(hex_chars[vol & 0x0F], realx + PV_CELL_VOL_X + PV_CHAR_WIDTH,
				              realy, volumecol);
			}
		}

		if (effects_visible) {
			// Effect and effect parameter
			if (cell->effect != NO_EFFECT)
				drawSmallChar(hex_chars[cell->effect], realx + PV_CELL_FX_X, realy,
				              effectcol);

			if (cell->effect_param != 0x00 || cell->effect != NO_EFFECT)
				drawHexByte(cell->effect_param,
				            realx + PV_CELL_FX_X + PV_CHAR_WIDTH, realy,
				            effectparamcol);
		}
	}

	void updateFromState(void);

	inline u16 getCellWidth(void)
	{
		if (effects_visible)
			return PV_CELL_WIDTH_FX;
		else
			return PV_CELL_WIDTH;
	}

	inline u16 getEffectiveWidth(void)
	{
		return PV_BORDER_WIDTH + getNumVisibleChannels() * getCellWidth();
	}

	inline u16 getNumVisibleChannels(void)
	{
		u16 cw = (width - PV_BORDER_WIDTH) / getCellWidth();
		if (cw < song->getChannels()) {
			return cw;
		} else {
			return song->getChannels();
		}
	}

	inline u16 getNumVisibleRows(void) { return height / PV_CELL_HEIGHT; }

	inline u16 getCursorBarPos(void) { return getNumVisibleRows() / 2 - 1; }

	void callMuteCallback(void);

	void (*onMute)(bool *channels_muted);

	bool pickCell(u16 px, u16 py, u16 *cx, u16 *cy);

	Cell **pattern;
	Song *song;
	State *state;

	u16 col_notes, col_instr, col_volume, col_effect, col_effect_param,
	    col_notes_dark, col_instr_dark, col_volume_dark, col_effect_dark,
	    col_effect_param_dark;

	u8 hscrollpos;
	u16 lines_per_beat;

	bool selection_exists, pen_down;
	bool effects_visible;

	u16 cell_width;
	u16 px, py;
	u16 sel_start_x, sel_end_x, sel_start_y, sel_end_y;
	u16 sel_x, sel_y, sel_w, sel_h;

	bool solo_channels[MAX_CHANNELS];
	bool mute_channels[MAX_CHANNELS];

	u8 componentpos;
	bool componentnav;
};

}; // namespace tobkit

#endif
