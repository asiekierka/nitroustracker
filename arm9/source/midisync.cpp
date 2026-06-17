/*
 * NitrousTracker - MIDI sync via the mDS Slot-2 cartridge
 *
 * See midisync.h. The byte layout and access discipline mirror the sDS
 * reference DS driver (synth-cart/ds-rom/source/synth_cart.c) and the cart
 * firmware's sync_state_t (HobbyChop/mDS src/sync_state.h): the "STDS"
 * protocol at version 2 (clock + MIDI event ring).
 *
 * Slot-2 notes (why this isn't just "read 0x0A000000"):
 *  - The GBA SRAM bus is byte-wide and the RP2040 emulates it with PIO, so
 *    every field is read one byte at a time (LDRB) with *generous* SRAM
 *    timing (18 cycles) so the cart can settle the data lines.
 *  - On BlocksDS the GBA-cart region is already in the ARM9 MPU (region 3,
 *    0x08000000/128MB), so reads don't fault -- but we still must own the
 *    slot (clear EXMEMCNT_CART_ARM7) and not touch slot-2 before that.
 *  - The cart is single-producer on the ring; we are single-consumer with
 *    our own read cursor. Values are gated against impossible bit patterns
 *    so an empty slot / open bus can't be mistaken for live data.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "midisync.h"
#include <string.h>

// Reading the clocked slot-2 cart perturbs the bus and crackles the DS audio,
// and the crackle scales with how OFTEN and how MUCH we read (a write-only test
// proved the EXMEMCNT write itself is silent). So we poll at a reduced rate and
// read the bare minimum each time.
#define POLL_DIV       4   // run the cart read once every POLL_DIV frames (~15Hz)
#define BPM_READ_DIV   8   // read the 4-byte BPM word once every N active polls

// --- STDS v2 mirrored struct, located at the start of the Slot-2 window ---
#define MDS_SRAM_BASE   0x0A000000u
#define STDS_VERSION    2
#define MIDI_RING_LEN   16

// Field offsets within the mirrored sync_state_t.
#define OFF_MAGIC        0x00 // char[4] "STDS" (legacy "SDS!" also accepted)
#define OFF_VERSION      0x04 // u8
#define OFF_FLAGS        0x05 // u8
#define OFF_TICK_COUNTER 0x08 // u32  24-PPQ MIDI clock (monotonic)
#define OFF_BPM_Q8       0x0C // u32  BPM in Q24.8 fixed point
#define OFF_SONG_POS     0x14 // u16  MIDI Song Position Pointer (16th notes)
#define OFF_WRITE_HEAD   0x16 // u8   producer index into the ring
#define OFF_READ_HEAD    0x17 // u8   (cart's own consumer index; we keep ours)
#define OFF_RING         0x18 // midi_event_t[16]; each event is 4 bytes:
                              //   status, data1, data2, tick_lo

#define FLAG_CLOCK_RUNNING  (1u << 0)
#define FLAG_TRANSPORT_PLAY (1u << 1)
#define FLAGS_VALID_MASK    0x03 // only bits 0-1 are defined; others => garbage

#define BPM_COOLDOWN_FRAMES 20   // min frames between tempo updates (jitter throttle)

static volatile u8 *const sram = (volatile u8*)MDS_SRAM_BASE;

// Byte-wise little-endian reads. `sram` is volatile, so each access is a
// distinct LDRB the compiler can't fold into a single (replicated) word load.
static inline u8  rd8 (u32 off) { return sram[off]; }
static inline u16 rd16(u32 off) { return (u16)(sram[off] | (sram[off + 1] << 8)); }
static inline u32 rd32(u32 off) {
	return (u32)sram[off]
	     | ((u32)sram[off + 1] << 8)
	     | ((u32)sram[off + 2] << 16)
	     | ((u32)sram[off + 3] << 24);
}

static bool bus_ready = false; // init ran
static bool probed     = false; // first-poll probe attempted
static bool present     = false; // a compatible cart was found
static u8   tp_play     = 0;   // committed transport state (debounced)
static u8   tp_cand     = 0;   // consecutive clean reads of the opposite state
static u16  last_bpm     = 0;
static u8   bpm_cooldown  = 0;   // frames until the next tempo update is allowed
static u8   read_head    = 0;   // our own consumer index into the event ring

#define TP_DEBOUNCE 3          // clean reads a transport change must persist

// Diagnostics captured during the probe (for the DEBUG console readout).
static u8   dbg_magic[4] = {0,0,0,0};
static u8   dbg_version  = 0;
static u16  dbg_exmemcnt = 0;
// Live diagnostics (updated each poll) for the per-second status line.
static u8   dbg_whead = 0, dbg_rhead = 0;
static u32  dbg_ev_total = 0;        // events dispatched since boot
static u32  dbg_flaky   = 0;         // frames the snapshot read back unstable
static u32  dbg_reassert = 0;        // frames EXMEMCNT was clobbered -> re-asserted
static u8   dbg_ls = 0, dbg_d1 = 0, dbg_d2 = 0; // last dispatched event bytes

bool midisync_present(void) { return present; }

void midisync_debug_probe(u8 magic[4], u8 *version, u16 *exmemcnt)
{
	if(magic) { magic[0]=dbg_magic[0]; magic[1]=dbg_magic[1];
	            magic[2]=dbg_magic[2]; magic[3]=dbg_magic[3]; }
	if(version)  *version  = dbg_version;
	if(exmemcnt) *exmemcnt = dbg_exmemcnt;
}

// Own the Slot-2 (GBA) bus on the ARM9 (clear CART_ARM7 / bit 7) and set the
// generous SRAM timing the cart needs to be read reliably (the DS default is
// too fast -> garbled reads). Something reverts this between frames, so it has
// to be re-asserted before reading.
static inline void slot2_bus_setup(void)
{
	REG_EXMEMCNT = (REG_EXMEMCNT & ~0xFF)
	             | EXMEMCNT_ROM_TIME1_10_CYCLES
	             | EXMEMCNT_ROM_TIME2_6_CYCLES
	             | EXMEMCNT_SRAM_TIME_18_CYCLES;
}

bool midisync_init(void)
{
	slot2_bus_setup();
	bus_ready = true;
	probed    = false;
	present   = false;
	return true; // detection result is only known after the first poll
}

// Read the magic and verify a compatible cart. Retried a few times because the
// cart's PIO may need a moment to settle after power-on.
static bool probe_cart(void)
{
	slot2_bus_setup();
	dbg_exmemcnt = REG_EXMEMCNT;

	for(int attempt = 0; attempt < 16; ++attempt) {
		char m0 = (char)rd8(OFF_MAGIC + 0);
		char m1 = (char)rd8(OFF_MAGIC + 1);
		char m2 = (char)rd8(OFF_MAGIC + 2);
		char m3 = (char)rd8(OFF_MAGIC + 3);
		dbg_magic[0]=(u8)m0; dbg_magic[1]=(u8)m1;
		dbg_magic[2]=(u8)m2; dbg_magic[3]=(u8)m3;
		dbg_version = rd8(OFF_VERSION);
		// "STDS" v2 = the unified clock + MIDI-ring cart; tolerate a legacy
		// "SDS!" build too (matches the sDS reference driver).
		bool is_stds = (m0 == 'S' && m1 == 'T' && m2 == 'D' && m3 == 'S');
		bool is_sds  = (m0 == 'S' && m1 == 'D' && m2 == 'S' && m3 == '!');
		if(is_stds || is_sds) {
			// Start consuming from wherever the cart is now, so we don't
			// replay a ring full of pre-boot events as a note storm.
			read_head  = rd8(OFF_WRITE_HEAD) % MIDI_RING_LEN;
			// Seed the committed transport state from the current (clean) flags
			// so we don't fire a spurious START/STOP on the first poll.
			{
				u8 f = rd8(OFF_FLAGS);
				tp_play = (!(f & ~FLAGS_VALID_MASK) && (f & FLAG_TRANSPORT_PLAY)) ? 1 : 0;
				tp_cand = 0;
			}
			last_bpm   = 0;
			return true;
		}
		swiDelay(2000); // ~30us; let the PIO settle, then retry
	}
	return false;
}

void midisync_poll(const MidiSyncCallbacks *cb)
{
	if(!bus_ready || cb == NULL)
		return;

	// First poll (after the GUI is up and a frame has passed): probe once.
	if(!probed) {
		probed  = true;
		present = probe_cart();
	}
	if(!present)
		return;

	// NOTE-INPUT MODE (clock sync is intentionally not handled here for now).
	// Drain the cart's MIDI event ring every frame and dispatch note on/off so
	// incoming MIDI notes play and can be placed into the pattern, like the
	// other MIDI input options. (Reading is clean here: a MIDI keyboard sends
	// notes, not clock, and reading only crackled the audio while the cart was
	// being CLOCKED.)
	slot2_bus_setup();                   // SRAM_18 needed for reliable reads
	dbg_exmemcnt = REG_EXMEMCNT;

	u8 wh_a = rd8(OFF_WRITE_HEAD);
	u8 wh_b = rd8(OFF_WRITE_HEAD);
	if(wh_a != wh_b) {                   // torn read while the cart was updating
		dbg_flaky++;
		return;
	}
	u8 write_head = wh_a % MIDI_RING_LEN;
	dbg_whead = write_head;

	if(cb->on_midi_event && read_head != write_head) {
		// Copy the pending entries out, then dispatch them.
		u8 evbuf[MIDI_RING_LEN * 3];
		u8 n = 0, rh = read_head;
		while(rh != write_head && n < MIDI_RING_LEN) {
			u32 o = OFF_RING + (u32)rh * 4;
			evbuf[n * 3 + 0] = rd8(o + 0);
			evbuf[n * 3 + 1] = rd8(o + 1);
			evbuf[n * 3 + 2] = rd8(o + 2);   // o+3 = tick_lo, unused
			rh = (rh + 1) % MIDI_RING_LEN;
			++n;
		}
		read_head = rh;

		for(u8 i = 0; i < n; ++i) {
			u8 status = evbuf[i * 3 + 0];
			u8 data1  = evbuf[i * 3 + 1];
			u8 data2  = evbuf[i * 3 + 2];
			// Only dispatch real channel-voice note messages with a valid note
			// byte; drop MIDI clock / real-time / torn bytes (e.g. 0xF8).
			u8 hi = status & 0xF0;
			if((hi == 0x80 || hi == 0x90) && data1 < 128) {
				dbg_ev_total++; dbg_ls = status; dbg_d1 = data1; dbg_d2 = data2;
				cb->on_midi_event(status, data1, data2);
			}
		}
	}
	dbg_rhead = read_head;
}

void midisync_debug_status(u8 *transport, u16 *bpm, u8 *whead, u8 *rhead,
                           u32 *ev_total, u32 *flaky, u8 *ls, u8 *d1, u8 *d2)
{
	if(transport) *transport = tp_play;
	if(bpm)       *bpm       = last_bpm;
	if(whead)     *whead     = dbg_whead;
	if(rhead)     *rhead     = dbg_rhead;
	if(ev_total)  *ev_total  = dbg_ev_total;
	if(flaky)     *flaky     = dbg_reassert;  // frames EXMEMCNT was found clobbered
	if(ls)        *ls        = dbg_ls;
	if(d1)        *d1        = dbg_d1;
	if(d2)        *d2        = dbg_d2;
}
