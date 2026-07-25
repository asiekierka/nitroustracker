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

// The current state of several things.
// In contrast to Settings, the State is not saved.

#ifndef STATE_H
#define STATE_H

#include "ntxm/song.h"

#define STATE_FILENAME_LEN 255

class State
{
public:
	State(void);
	~State(void);
	void reset(void);
	void resetSong(void); // resets only song-specific settings

	inline u16 getPlaybackRow() { return this->row; }
	inline void setPlaybackRow(u16 row)
	{
		this->row = row;
		if (!scroll_lock) {
			this->cursor_row = row;
		}
	}
	inline u16 getCursorRow() { return this->cursor_row; }
	inline void setCursorRow(u16 row) { this->cursor_row = row; }

	char *song_filename;
	char *sample_filename;
	char *inst_filename;

	Sample *preview_sample;

	s16 queued_potpos;
	u8 potpos;
	u8 channel;

	u8 add;
	u8 basenote;
	u8 instrument; // Current instrument
	u8 sample;
	bool recording;
	bool playing;
	bool pause;
	bool unsaved_changes;

	bool map_samples;
	bool scroll_lock;

private:
	u16 row;
	u16 cursor_row;
};

#endif
