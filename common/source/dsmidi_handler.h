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

#ifndef NT_DSMIDI_HANDLER_H_
#define NT_DSMIDI_HANDLER_H_

#include "ntxm/song.h"

class DSMIDIHandler
{
public:
	DSMIDIHandler() {}
	~DSMIDIHandler() {}

	bool connect();
	void disconnect();
	void tick();
	void stop();
	void noteStroke(bool on, int channel, int note);
	void rowUpdate(Song *song, int row, int potpos);

	bool dsmi_connected = false;
	bool dsmi_send = true;
	bool dsmi_recv = true;

private:
	u8 dsmw_lastnotes[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	u8 dsmw_lastchannels[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};

#endif
