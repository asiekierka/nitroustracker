/*
 * NitrousTracker - MIDI sync via the mDS Slot-2 cartridge
 *
 * Reads the "STDS" v2 protocol that the HobbyChop/mDS RP2040 cart mirrors
 * into the DS Slot-2 (GBA) SRAM window at 0x0A000000. This gives us, with
 * no WiFi and no interrupts, an incoming 24-PPQ MIDI clock, transport
 * play/stop, live BPM, song position and a 16-entry MIDI event ring.
 *
 * Protocol reference: https://github.com/HobbyChop/mDS  (src/sync_state.h)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef MIDISYNC_H
#define MIDISYNC_H

#include <nds.h>

#ifdef __cplusplus
extern "C" {
#endif

// Callbacks fired by midisync_poll(). Any field may be NULL.
typedef struct {
	void (*on_transport_start)(u16 song_pos_16ths); // external START / CONTINUE
	void (*on_transport_stop)(void);                // external STOP
	void (*on_bpm)(u16 bpm);                        // tempo changed (32..255)
	void (*on_song_pos)(u16 pos_16ths);             // MIDI Song Position Pointer
	void (*on_midi_event)(u8 status, u8 data1, u8 data2); // note/CC/... from the ring
	bool (*is_playing)(void);  // true while the tracker is playing. The poll reads
	                           // the cart ONLY while stopped -- reading the clocked
	                           // cart clicks the audio, so we free-run (no reads)
	                           // during playback. NULL => always read.
} MidiSyncCallbacks;

// Grab the Slot-2 bus and probe for an mDS ("STDS" v2) cart.
// Returns true if a compatible cart is present.
bool midisync_init(void);

// True if midisync_init() found a compatible cart.
bool midisync_present(void);

// Debug: returns the bytes seen by the last probe attempt - the 4 magic bytes,
// the version byte, and the EXMEMCNT register value after bus setup. Lets a
// DEBUG build show why a probe failed (e.g. magic = FF FF FF FF -> open bus).
void midisync_debug_probe(u8 magic[4], u8 *version, u16 *exmemcnt);

// Debug: live snapshot of the running state for a periodic status line.
// transport (0/1), current followed bpm, ring write/read heads, total events
// dispatched, count of frames a torn/unstable read was rejected, and the last
// dispatched event bytes. Any pointer may be NULL.
void midisync_debug_status(u8 *transport, u16 *bpm, u8 *whead, u8 *rhead,
                           u32 *ev_total, u32 *flaky, u8 *ls, u8 *d1, u8 *d2);

// Poll the cart once; call once per frame (e.g. from the VBlank loop).
// Fires the callbacks above for transport edges, tempo/position changes
// and any MIDI events queued in the ring. No-op if no cart is present.
void midisync_poll(const MidiSyncCallbacks *cb);

#ifdef __cplusplus
}
#endif

#endif // MIDISYNC_H
