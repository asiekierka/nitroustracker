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

#include "dsmidi_handler.h"

#ifdef MIDI

#include <nds.h>
#include <libdsmi.h>
#include <dswifi9.h>
#include "ntxm/fifocommand.h"

bool DSMIDIHandler::connect() { 
    if (dsmi_connected) return true;
    dsmi_connected = (dsmi_connect() > 0);
    return dsmi_connected;
}

void DSMIDIHandler::disconnect() {
    if (!dsmi_connected) return;
    dsmi_disconnect();
}

void DSMIDIHandler::tick() {
    if (!dsmi_connected) return;
    
    // Pull events
	u8 message, data1, data2;

	while(dsmi_read(&message, &data1, &data2)) {
		if(dsmi_recv) {
			// debugprintf("got sth. %x %x %x\n", message, data1, data2);

			u8 type = message & 0xF0;
			//debugprintf("Type is %x\n", type);
			switch(type) {
				case NOTE_ON: {
					u8 inst = message & 0x0F;
					u8 note = data1;
					u8 volume = data2;
					u16 tag = (((u16)inst + 1) << 8) | note;
					// debugprintf("on %d %d\n", inst, note);
					CommandPlayNoteAuto(inst, note, volume, tag);
					break;
				}

				case NOTE_OFF: {
					u8 inst = message & 0x0F;
					u8 note = data1;
					u16 tag = (((u16)inst + 1) << 8) | note;
					CommandStopNoteAuto(tag);
					break;
				}
			}
		}
	}

    // Tick libdsmi
    dsmi_task();
}

void DSMIDIHandler::stop() {
    if(!(dsmi_connected && dsmi_send)) return;

    for(u8 chn=0; chn<16; ++chn) {
        dsmi_write(MIDI_CC | chn, 120, 0);
        dsmi_write(NOTE_OFF | chn, dsmw_lastnotes[chn], 0);
    }
}

void DSMIDIHandler::noteStroke(bool on, int channel, int note) {
	if(!(dsmi_connected && dsmi_send)) return;

    dsmi_write((on ? NOTE_ON : NOTE_OFF) | channel, note, 127);
}

void DSMIDIHandler::rowUpdate(Song *song, int row, int potpos) {
	if(!(dsmi_connected && dsmi_send)) return;

    Cell ** pattern = song->getPattern(song->getPotEntry(potpos));

    Cell *curr_cell;

    for(u8 chn=0; chn < song->getChannels(); ++chn)
    {
        if(song->channelMuted(chn))
            continue;

        curr_cell = &(pattern[chn][row]);

        if(curr_cell->note == 254) // Note off
        {
            //debugprintf("off c %u n %u\n", chn, curr_cell->note);
            dsmi_write(NOTE_OFF | dsmw_lastchannels[chn], dsmw_lastnotes[chn], 0);
            dsmw_lastnotes[chn] = curr_cell->note;
        }
        else if(curr_cell->note < 254) // Note on
        {
            // Turn the last note off
            if(dsmw_lastnotes[chn] < 254) {
                //debugprintf("off c %u n %u\n", chn, curr_cell->note);
                dsmi_write(NOTE_OFF | dsmw_lastchannels[chn], dsmw_lastnotes[chn], 0);
            }
            //debugprintf("on c %u n %u v %u\n", chn, curr_cell->note, curr_cell->volume / 2);
            u8 midichannel = curr_cell->instrument & 0xF;
            dsmi_write(NOTE_ON | midichannel, curr_cell->note, curr_cell->volume / 2);

            dsmw_lastchannels[chn] = midichannel;
            dsmw_lastnotes[chn] = curr_cell->note;
        }
    }
}
#else

bool DSMIDIHandler::connect() { return false; }
void DSMIDIHandler::disconnect() { }
void DSMIDIHandler::tick() { }
void DSMIDIHandler::stop() { }
void DSMIDIHandler::noteStroke(bool on, int channel, int note) { }
void DSMIDIHandler::rowUpdate(Song *song, int row, int potpos) { }

#endif
