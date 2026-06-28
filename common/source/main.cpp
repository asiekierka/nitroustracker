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

// #define SHOW_ALL_SETTINGS

#if defined(NT_PLATFORM_NDS)
#include <nds.h>
#include <fat.h>
#define GURU // Show guru meditations
#define ENABLE_EFFECT_MENU
#define ENABLE_PIANO_PAK
#elif defined(NT_PLATFORM_3DS)
#include <3ds.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <tobkit/tobkit.h>

// Special tracker widgets
#include "tobkit/numbersliderrelnote.h"
#include "tobkit/patternview.h"
#include "tobkit/normalizebox.h"
#include "tobkit/themeselectorbox.h"
#include "tobkit/envelope_editor.h"
#include "tobkit/fxkeyboard.h"
#include "tobkit/recordbox.h"
#include "tobkit/sampledisplay.h"
#include "tobkit/digitbox.h"
using namespace tobkit;

#include <ntxm/fifocommand.h>
#include <ntxm/mod_transport.h>
#include <ntxm/song.h>
#include <ntxm/xm_transport.h>
#include <ntxm/wav.h>
#include <ntxm/instrument.h>
#include <ntxm/sample.h>
#include <ntxm/ntxmtools.h>

#include "dsmidi_handler.h"
#include "state.h"
#include "settings.h"
#include "tools.h"
#include "platform.h"

#include "icon_disk_raw.h"
#include "icon_disk_unsaved_raw.h"
#include "icon_song_raw.h"
#include "icon_sample_raw.h"
#include "icon_wrench_raw.h"
#include "icon_trumpet_raw.h"

#include "icon_flp_raw.h"
#include "icon_fx_raw.h"
#include "icon_copy_raw.h"
#include "icon_cut_raw.h"
#include "icon_paste_raw.h"
#include "icon_pause_raw.h"
#include "icon_play_raw.h"
#include "icon_record_raw.h"
#include "icon_stop_raw.h"

#include "icon_undo_raw.h"
#include "icon_redo_raw.h"

#include "icon_new_folder_raw.h"

#include "nitrotracker_logo_raw.h"

#include "sampleedit_control_icon_raw.h"
#include "sampleedit_chip_icon_raw.h"
#include "sampleedit_wave_icon_raw.h"
#include "sampleedit_loop_icon_raw.h"
#include "sampleedit_fadein_raw.h"
#include "sampleedit_fadeout_raw.h"
#include "sampleedit_all_raw.h"
#include "sampleedit_none_raw.h"
#include "sampleedit_del_raw.h"
#include "sampleedit_trim_raw.h"
#include "sampleedit_reverse_raw.h"
#include "sampleedit_record_raw.h"
#include "sampleedit_normalize_raw.h"
#include "sampleedit_draw_raw.h"
#include "sampleedit_draw_small_raw.h"

#include "cell_array.h"
#include "action.h"

#define REPEAT_FREQ	10 /* Hz */
#define REPEAT_START_DELAY 15 /* frames */

#define FILETYPE_SONG	0
#define FILETYPE_SAMPLE	1
#define FILETYPE_INST	2

u8 frame = 0;

char *launch_path = NULL;

bool typewriter_active = false;
bool exit_requested = false;
volatile bool redraw_main_requested = false;

GUI *gui;

// <Misc GUI>
	Button *buttonrenameinst, *buttonrenamesample, *buttontest, *buttonstopnote, *buttoncpprm, *buttonemptynote, *buttonemptyfx, *buttondelnote, *buttoninsnote2,
		*buttondelnote2, *buttoninsnote, *buttonlerpfx;
	BitButton *buttonswitchsub, *buttonplay, *buttonstop, *buttonpause;
	CheckBox *cbscrolllock;
	ToggleButton *tbrecord, *tbmultisample;
	Label *labeladd, *labeloct, *labelfxcat, *labelfxop, *labeleffectpar;
	NumberBox *numberboxadd, *numberboxoctave, *numberboxfxcat;
	Piano *kb;
	FXKeyboard *fxkb;
	ListBox *lbinstruments, *lbsamples;
	u16 lbinstruments_height, lbsamples_height;
	TabBox *tabbox;
	GradientIcon *pixmaplogo;
// </Misc GUI>

// <Disk op gui>
	Label *labelitem, *labelFilename, *labelramusage_disk;
	RadioButton *rbsong, *rbsample, *rbinst;
	RadioButton::RadioButtonGroup *rbgdiskop;
	Button *buttonsave, *buttonload, *buttondelfile, *buttonchangefilename;
	BitButton *buttonnewfolder;
	FileSelector *fileselector;
	MemoryIndicator *memoryiindicator_disk;
	CheckBox *cbsamplepreview;
// </Disk op gui>

// <Song Gui>
	Label *labelsonglen, *labeltempo, *labelbpm, *labelptns, *labelptnlen,
		*labelchannels, *labelsongname, *labelrestartpos, *labelramusage;
	ListBox *lbpot;
	Button *buttonpotup, *buttonpotdown, *buttoncloneptn,
		*buttonmorechannels, *buttonlesschannels, *buttonzap, *buttonrenamesong;
	ToggleButton *tbqueuelock, *tbpotloop;
	NumberBox *nbtempo;
	NumberSlider *nsptnlen, *nsbpm, *nsrestartpos;
	MemoryIndicator *memoryiindicator;
// </Song Gui>

// <Sample Gui>
	RecordBox *recordbox;
	NormalizeBox *normalizeBox;
	SampleDisplay *sampledisplay;
	TabBox *sampletabbox;

	Label *labelsamplevolume, *labelrelnote, *labelfinetune, *labelpanning;
	NumberSlider *nssamplevolume, *nsfinetune, *nspanning;
	NumberSliderRelNote *nsrelnote;

	Label *labelsampleedit_select, *labelsampleedit_edit, *labelsampleedit_record;
	BitButton *buttonsmpfadein, *buttonsmpfadeout, *buttonsmpselall, *buttonsmpselnone, *buttonsmpseldel,
		*buttonsmpreverse, *buttonrecord, *buttonsmpnormalize, *buttonsmptrim;

	GroupBox *gbsampleloop;
	RadioButton::RadioButtonGroup *rbg_sampleloop;
	RadioButton *rbloop_none, *rbloop_forward, *rbloop_pingpong;
	CheckBox *cbsnapto0xing;

	ToggleButton *buttonsmpdraw;
// </Sample Gui>

// <Instrument Gui>
	EnvelopeEditor *volenvedit;
	Button *btnaddenvpoint, *btndelenvpoint, *btnenvdrawmode, *btnenvsetsuspoint;
	ToggleButton *tbmapsamples;
	CheckBox *cbvolenvenabled, *cbsusenabled;
// </Instrument Gui>

// <Settings Gui>
	RadioButton::RadioButtonGroup *rbghandedness;
	RadioButton *rblefthanded, *rbrighthanded;
	Button *bttheme;
	ThemeSelectorBox *fbtheme;
	GroupBox *gbhandedness, *gbdsmw, *gbtheme;
	CheckBox *cbdsmwsend, *cbdsmwrecv;
	Button *btndsmwtoggleconnect;
	RadioButton::RadioButtonGroup *rbgoutput;
	RadioButton *rboutputmono, *rboutputstereo;
	GroupBox *gboutput;
#ifdef NT_PLATFORM_NDS
	RadioButton::RadioButtonGroup *rbgfreq;
	RadioButton *rbfreq32, *rbfreq47;
	GroupBox *gbfreq;
#endif
	GroupBox *gblinesbeat;
	NumberBox *nblinesbeat;
	Button *btnconfigsave;
// </Settings Gui>

// <Main Screen>
	Button *buttonins, *buttondel, *buttonstopnote2, *buttoncolselect, *buttonemptynote2, *buttonunmuteall;
	BitButton *buttonswitchmain;
	Button *buttoncut, *buttoncopy, *buttonpaste, *buttonsetnotevol;
	Button *buttontransposedown, *buttontransposeup;
	Button *buttonseteffectpar;
	BitButton *buttonundo, *buttonredo;
	PatternView *pv;
	NumberSlider *nsnotevolume;
	DigitBox *dbeffectpar;
	Label *labelnotevol, *labeleffectcmd, *labeltranspose;
	ToggleButton *tbeffects;
// </Main Screen>

// <Things that suddenly pop up>
#if defined(NT_PLATFORM_NDS)
    Typewriter *tw = NULL;
#endif
	void (*twOkCallback)(const char*);
	MessageBox *mb = NULL;
// </Things that suddenly pop up>

u16 *b1n, *b1d;
int lastx, lasty;
Song *song;
State *state;
Settings *settings;
ModTransport mod_transport;
XMTransport xm_transport;

CellArray *clipboard = NULL;
ActionBuffer *action_buffer = NULL;

DSMIDIHandler dsmidi_handler;
char last_themepath[SETTINGS_FILENAME_LEN + 1];
char *preview_smp_path = NULL;

bool fastscroll = false;
bool multisamp_from_mapsamp = false;
bool mod_loading = false;

// TODO: Make own class for tracker control and remove forward declarations
void handleButtons(u16 buttons, u16 buttonsheld);
void HandleTick(void);
void handlePotPosChangeFromSong(u16 newpotpos);
void handleSampleChange(u16 sample);
void handleToggleMapSamples(bool on);
void setMultisamplesEnabled(bool show);
void drawMainScreen(void);
void redrawSubScreen(void);
void showMessage(const char *msg, bool error);
void deleteMessageBox(void);
void stopPlay(void);
void setHasUnsavedChanges(bool unsaved);
void handleClearFx(void);
void updateSampleOffsetGuide(void);

#include "debug_helpers.h"

void clearMainScreen(void)
{
	PlatformClearMainScreen(settings->getTheme()->col_bg);
}

void clearSubScreen(void)
{
	PlatformClearSubScreen(settings->getTheme()->col_bg);
}

void drawSampleNumbers(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == NULL)
	{
		for(int key=0; key<kb->getKeyCount(); ++key)
		{
			kb->setKeyLabel(key, '0');
		}
	}
	else
	{
    	char label;
    	u8 note, sample_id;
    	for(int key=0; key<kb->getKeyCount(); ++key)
    	{
    		note = state->basenote + key;
    		sample_id = inst->getNoteSample(note) & 0x0F;
    		label = (sample_id >= 0xA) ? (sample_id - 0xA + 'a') : (sample_id + '0');

    		kb->setKeyLabel(key, label);
    	}
	}
#ifndef NT_PLATFORM_NDS
    kb->pleaseDraw();
#endif
}

void updateKeyLabels(void)
{
	if (fxkb->is_visible()) return;

	kb->hideKeyLabels();
	if(state->map_samples)
	{
		drawSampleNumbers();
		kb->showKeyLabels();
	}
}

void setHasUnsavedChanges(bool unsaved)
{
	bool has_unsaved = unsaved && !mod_loading;

	state->unsaved_changes = has_unsaved;

	if (!tabbox || tabbox->getCount() != 5) return;
	tabbox->setIcon(1, has_unsaved ? icon_disk_unsaved_raw : icon_disk_raw);
}

static void handleNoteAdvanceRow(void)
{
	// Check if we are not at the bottom and only scroll down as far as possible
	if((state->playing == false)||(state->pause==true)||state->scroll_lock)
	{
		u16 row = state->getCursorRow();
		row += state->add;
		row %= song->getPatternLength(song->getPotEntry(state->potpos));
		state->setCursorRow(row);
	}
}

static Cell getChangedNote(Cell cell, u8 note)
{
	// Check if this was an empty- or stopnote
	if((note==EMPTY_NOTE)||(note==STOP_NOTE)) {
		// Because then we don't use the offset since they have fixed indices
		song->clearCell(&cell);
		if(note==STOP_NOTE)
			cell.note = note;
	} else {
		cell.note = state->basenote + note;
		cell.instrument = state->instrument;
	}

	return cell;
}

// returns selection or currently highlighted cell
static bool uiPotSelection(u16 *sel_x1, u16 *sel_y1, u16 *sel_x2, u16 *sel_y2, bool clear)
{
	bool is_box = pv->getSelection(sel_x1, sel_y1, sel_x2, sel_y2);
	if (!is_box)
	{
		*sel_x1 = *sel_x2 = state->channel;
		*sel_y1 = *sel_y2 = state->getCursorRow();
	}
	else
	{
		if (clear) {
			pv->clearSelection();
		}
	}

	return is_box;
}

void deleteSong()
{
    CommandSetSong(nullptr);
    delete song;
}

void handleNoteFill(u8 note)
{
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	bool is_box;
	is_box = uiPotSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2, true);

	if (!is_box)
	{
		// smaller cell set
		Cell targetCell = getChangedNote(
			song->getPattern(song->getPotEntry(state->potpos))[state->channel][state->getCursorRow()], note
		);

		action_buffer->add(song, new SingleCellSetAction(state, state->channel, state->getCursorRow(), targetCell));
		handleNoteAdvanceRow();
		return;
	}

    CellArray *fill = new CellArray(sel_x2 - sel_x1 + 1, sel_y2 - sel_y1 + 1);
    if (fill != NULL && fill->valid())
    {
		for (u16 chn = sel_x1; chn <= sel_x2; chn++)
			for (u16 row = sel_y1; row <= sel_y2; row++)
				*fill->ptr(chn - sel_x1, row - sel_y1) = getChangedNote(
					song->getPattern(song->getPotEntry(state->potpos))[chn][row], note
				);
        action_buffer->add(song, new MultipleCellSetAction(state, sel_x1, sel_y1, fill, false));
    }
}

// swap the sample display if the key has another
// sample mapped
void onKeypress(u8 note)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if (inst==0) return;

	u16 newsamp = inst->getNoteSample(note + state->basenote);

	handleSampleChange(newsamp);
}

void onKeyrelease(void)
{
	// stop cursor
}

void handleNoteStroke(u8 note)
{
	if (note == EMPTY_NOTE || note == STOP_NOTE) return;

	// If we are recording
	if(state->recording == true)
	{
		// TODO: restore pattern setting while preserving undo
		/* uiSetNote(state->channel, state->getCursorRow(), note);

		// Redraw
		ntxm_flush_dcache();
		redraw_main_requested = true; */
	}
	// If we are in sample mapping mode, map the pressed key to the selected sample for the current instrument
	if(state->map_samples == true)
	{
		Instrument *inst = song->getInstrument(state->instrument);
		if(inst != NULL)
		{
			inst->setNoteSample(state->basenote + note, state->sample);
			ntxm_flush_dcache();
		}

		char label;
		u8 sample_id = state->sample & 0xF;
		label = (sample_id >= 0xA) ? (sample_id - 0xA + 'a') : (sample_id + '0');
		kb->setKeyLabel(note, label);
#ifndef NT_PLATFORM_NDS
        kb->pleaseDraw();
#endif
	}

	onKeypress(note);

	// Play the note
	CommandPlayNoteAuto(state->instrument, state->basenote + note, 255, note);

	dsmidi_handler.noteStroke(true, state->instrument & 0xF, state->basenote + note);
}

void handleNoteRelease(u8 note, bool moved)
{
	if (note == EMPTY_NOTE || note == STOP_NOTE) return;

	// If we are recording
	if((state->recording == true) && !moved)
	{
		Cell newCell = getChangedNote(song->getPattern(song->getPotEntry(state->potpos))[state->channel][state->getCursorRow()], note);
		action_buffer->add(song, new SingleCellSetAction(state, state->channel, state->getCursorRow(), newCell));

		// Advance row
		handleNoteAdvanceRow();

		// Redraw
		ntxm_flush_dcache();
		redraw_main_requested = true;
	}

	onKeyrelease();
	CommandStopNoteAuto(note);

	dsmidi_handler.noteStroke(false, state->instrument & 0xF, state->basenote + note);
}

void handlePianoPakStroke(u8 note)
{
	if(state->recording == true)
	{
		Cell newCell = getChangedNote(song->getPattern(song->getPotEntry(state->potpos))[state->channel][state->getCursorRow()], note);
		action_buffer->add(song, new SingleCellSetAction(state, state->channel, state->getCursorRow(), newCell));

		// Advance row
		handleNoteAdvanceRow();

		// Redraw
		ntxm_flush_dcache();
		redraw_main_requested = true;
	}

	onKeypress(note);

	// Play the note
	CommandPlayNoteAuto(state->instrument, state->basenote + note, 255, note);

	dsmidi_handler.noteStroke(true, state->instrument & 0xF, state->basenote + note);
}

void handlePianoPakRelease(u8 note)
{
	onKeyrelease();
	CommandStopNoteAuto(note);

	dsmidi_handler.noteStroke(false, state->instrument & 0xF, state->basenote + note);
}

void updateSampleList(Instrument *inst)
{
	if(inst == NULL)
	{
		for(u8 i=0; i<MAX_INSTRUMENT_SAMPLES; ++i)
		{
			lbsamples->set(i, "");
		}
	}
	else
	{
		Sample *sample;
		char *str=(char*) ntxm_ccalloc(1, SAMPLE_NAME_LENGTH + 1);
		for(u8 i=0; i<MAX_INSTRUMENT_SAMPLES; ++i)
		{
			sample = inst->getSample(i);
			if(sample != NULL)
			{
				strncpy(str, sample->getName(), SAMPLE_NAME_LENGTH);
				lbsamples->set(i, str);
			} else {
				lbsamples->set(i, "");
			}
		}
		ntxm_free(str);
	}
}

void updateMemoryState(bool print)
{
	memoryiindicator->pleaseDraw();
	memoryiindicator_disk->pleaseDraw();
#ifdef DEBUG
	if (print) PrintFreeMem();
#endif
}

void updateFilesystemState(bool draw)
{
	fileselector->invalidateFileList();
	if(draw) fileselector->pleaseDraw();
}

void handleSampleChange(const u16 newsample)
{
	bool had_changes = state->unsaved_changes;

	state->sample = newsample;
	Instrument *inst = song->getInstrument(lbinstruments->getidx());
	Sample *smp = inst ? inst->getSample(newsample) : NULL;
	bool is_null_sample = smp == NULL || smp->getData() == NULL;

	rbloop_none->set_enabled(!is_null_sample);
	rbloop_forward->set_enabled(!is_null_sample);
	rbloop_pingpong->set_enabled(!is_null_sample);
	nssamplevolume->set_enabled(!is_null_sample);
	nspanning->set_enabled(!is_null_sample);
	nsrelnote->set_enabled(!is_null_sample);
	nsfinetune->set_enabled(!is_null_sample);
	buttonsmpfadein->set_enabled(!is_null_sample);
	buttonsmpfadeout->set_enabled(!is_null_sample);
	buttonsmpselall->set_enabled(!is_null_sample);
	buttonsmpselnone->set_enabled(!is_null_sample);
	buttonsmpseldel->set_enabled(!is_null_sample);
	buttonsmptrim->set_enabled(!is_null_sample);
	buttonsmpreverse->set_enabled(!is_null_sample);
	buttonsmpnormalize->set_enabled(!is_null_sample);
	cbsnapto0xing->set_enabled(!is_null_sample);
	buttonsmpdraw->set_enabled(!is_null_sample);
	buttonrenameinst->set_enabled(inst != NULL);
	buttonrenamesample->set_enabled(smp != NULL);
	lbsamples->select(newsample);

	if(is_null_sample)
	{
		sampledisplay->setSample(NULL);
		nssamplevolume->setValue(0);
		nspanning->setValue(64);
		nsrelnote->setValue(0);
		nsfinetune->setValue(0);
		rbg_sampleloop->setActive(0);

		return;
	}

	sampledisplay->setSample(smp);
	sampledisplay->hideLoopPoints();
	nssamplevolume->setValue( (smp->getVolume()+1)/4 );
	nspanning->setValue(smp->getPanning()/2);
	nsrelnote->setValue(smp->getRelNote());
	nsfinetune->setValue(smp->getFinetune());

	if( (smp->getLoop() >= 0) && (smp->getLoop() <= 2) )
		rbg_sampleloop->setActive(smp->getLoop());
	else
		rbg_sampleloop->setActive(0);

	if (smp != NULL) {
		const char *str = smp->getName();
		strncpy(state->sample_filename, str, STATE_FILENAME_LEN);

		if(rbsample->getActive() == true)
		{
			labelFilename->setCaption(str);
		}
	}

	updateSampleOffsetGuide();
	updateKeyLabels();
	if (!had_changes) setHasUnsavedChanges(false);
	/*
	printf("Selected:");
	if(smp->is16bit()) {
		printf("16bit ");
	} else {
		printf("8bit ");
	}
	if(smp->getLoop() != 0) {
		printf("looping ");
	}
	printf("Sample.\n");
	printf("length: %u\n", smp->getNSamples());
	*/
}

void handleOverlayWidgetChange(u8 screen, bool visible)
{
#ifdef NT_PLATFORM_NDS
	if (screen == SUB_SCREEN)
	{
		if (visible) oamDisable(&oamSub);
		else if (!sampledisplay->is_occluded()) oamEnable(&oamSub);
	}
#endif
}

void volEnvSetInst(Instrument *inst)
{
	bool had_unsaved = state->unsaved_changes;
	if(inst == NULL)
	{
		volenvedit->setZoomAndPos(0, 0);
		volenvedit->setPoints(0, 0, 0);
	}
	else
	{
		u16 *xs, *ys;
		u16 n = inst->getVolumeEnvelope(&xs, &ys);
		bool s = inst->getVolumeEnvelopeSustainFlag();
		u8 susp = inst->getVolumeEnvelopeSustainPoint();
		volenvedit->setZoomAndPos(2, 0);
		volenvedit->setPoints(xs, ys, n);
		volenvedit->setEditorSustainParams(s, susp);
	}
	btnenvdrawmode->set_enabled(inst != NULL);
	btnaddenvpoint->set_enabled(inst != NULL);
	btndelenvpoint->set_enabled(inst != NULL);
	btnenvsetsuspoint->set_enabled(inst != NULL);
	cbvolenvenabled->set_enabled(inst != NULL);
	cbsusenabled->set_enabled(inst != NULL);
	tbmapsamples->set_enabled(inst != NULL);
	volenvedit->pleaseDraw();
	if (!had_unsaved) setHasUnsavedChanges(false);
}

void handleInstChange(const u16 newinst, const bool reset=true)
{
	state->instrument = newinst;

	Instrument *inst = song->getInstrument(newinst);
	updateSampleOffsetGuide();
	updateSampleList(inst);
	volEnvSetInst(inst);
	updateKeyLabels();

	if (reset)
		handleSampleChange(0); // handles the state sample
	else if(inst == NULL)
		handleSampleChange(state->sample); // preserve current sample so user can load new smp into slot >0 on null inst

	cbvolenvenabled->setChecked(inst != NULL && inst->getVolEnvEnabled());

}

void handleInstChangeReset(u16 newinst)
{
	handleInstChange(newinst, true);
}

void updateLabelSongLen(void)
{
	/* char labelstr[12];
	snprintf(labelstr, 12, "songlen:%2d", song->getPotLength());
	labelsonglen->setCaption(labelstr); */
}

void updateLabelChannels(void)
{
	char labelstr[9];
	snprintf(labelstr, 9, "chn: %2d", song->getChannels());
	labelchannels->setCaption(labelstr);
}

void updateTempoAndBpm(void)
{
	nsbpm->setValue(song->getBPM());
	nbtempo->setValue(song->getTempo());
}

void setSong(Song *newsong)
{
	song = newsong;
	char str[256];
	str[255] = 0;

	CommandSetSong(song);

	state->resetSong();

	pv->setSong(song);

	// Clear sample display
	sampledisplay->setSample(0);

	// Clear action buffer
	action_buffer->clear();

	// Update POT
	lbpot->clear();
	u8 potentry;
	for(int i=0;i<song->getPotLength();++i) {
		potentry = song->getPotEntry(i);
		snprintf(str, sizeof(str)-1, "%2x", potentry);
		lbpot->add(str);
	}

	// Update instrument list
	Instrument *inst;
	for(int i=MAX_INSTRUMENTS-1;i>=0;i--)
	{
		inst = song->getInstrument(i);
		if(inst!=NULL) {
			strncpy(str, inst->getName(), sizeof(str)-1);
			lbinstruments->set(i, str);
		} else {
			lbinstruments->set(i, "");
		}
	}

	// inst is now equal to song->getInstrument(0)
	lbinstruments->select(0);
	updateSampleList(inst);
	handleSampleChange(0);
	volEnvSetInst(inst);

	if(inst != 0)
	{
		cbvolenvenabled->setChecked(inst->getVolEnvEnabled());
		cbsusenabled->setChecked(inst->getVolumeEnvelopeSustainFlag());
	}

	updateLabelChannels();
	updateLabelSongLen();
	updateTempoAndBpm();
	buttonpotdown->set_enabled(song->getPotEntry(state->potpos) > 0);
	buttonpotup->set_enabled(song->getPotEntry(state->potpos) < MAX_PATTERNS-1);
	buttonmorechannels->set_enabled(song->getChannels() < MAX_CHANNELS);
	buttonlesschannels->set_enabled(song->getChannels() > 1);
	buttondel->set_enabled(song->getPotLength()>1);
	nsptnlen->setValue(song->getPatternLength(song->getPotEntry(state->potpos)));
	nsrestartpos->setValue(song->getRestartPosition());
	tbqueuelock->setState(false);
	tbpotloop->setState(false);

	numberboxadd->setValue(state->add);
	numberboxoctave->setValue(state->basenote/12);

	tbrecord->setState(false);
	cbscrolllock->setChecked(false);

	inst = song->getInstrument(state->instrument);
	if(inst != NULL) {
		sampledisplay->setSample(inst->getSample(state->sample));
	}

	strncpy(str, song->getName(), sizeof(str)-1);
	labelsongname->setCaption(str);

	mod_loading = false;
	setHasUnsavedChanges(false);
	drawMainScreen();
}

bool loadSample(const char *filename_with_path)
{
	const char *filename = strrchr(filename_with_path, '/') + 1;
	debugprintf("file: %s %s\n",filename_with_path, filename);

	bool load_success;
	Sample *newsmp = new Sample(filename_with_path, false, &load_success);
	if(load_success == false)
	{
		delete newsmp;
		return false;
	}

	u8 instidx = lbinstruments->getidx();
	u8 smpidx = state->sample;
	//
	// Create the instrument if it doesn't exist
	//
	Instrument *inst = song->getInstrument(instidx);
	if(inst == 0)
	{
		char *instname = (char*)ntxm_cmalloc(MAX_INST_NAME_LENGTH+1);
		strncpy(instname, filename, MAX_INST_NAME_LENGTH);

		inst = new Instrument(instname);
		song->setInstrument(instidx, inst);

		ntxm_free(instname);

		lbinstruments->set(state->instrument, song->getInstrument(state->instrument)->getName());
		handleInstChange(instidx, false); // don't implicitly reset lbsamples to pos 0 if loading a sample!
	}

	//
	// Insert new sample (if there's already one, it's deleted)
	//
	inst->setSample(smpidx, newsmp);

	lbsamples->set(lbsamples->getidx(), newsmp->getName());

	// Rename the instrument if we are in "single sample mode"
	if(!lbsamples->is_visible())
	{
		inst->setName(newsmp->getName());
		lbinstruments->set(state->instrument, song->getInstrument(state->instrument)->getName());
	}

	handleSampleChange(smpidx);

	ntxm_flush_dcache();

	setHasUnsavedChanges(true);
	return true;
}

void showSlowLoadOperation(std::function<const char*(void)> loadOp)
{
	// This IRQ approach occasionally causes a libc mutex deadlock.
	// A better idea would be to use cothread_yield(); or a
	// callback function.

	// SetYtrigger(191);
	// irqSet(IRQ_VCOUNT, updateMemoryState);
	// irqEnable(IRQ_VCOUNT);

	mb = new MessageBox(sub_screen, "one moment", 0);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->show();
	mb->pleaseDraw();

	const char* res = loadOp();
	updateMemoryState(true);
	ntxm_flush_dcache();

	deleteMessageBox();

	// irqDisable(IRQ_VCOUNT);
	// irqClear(IRQ_VCOUNT);

	if (res != NULL)
		showMessage(res, true);
}

File *getSelectedFile(void)
{
	File *file = fileselector->getSelectedFile();
	if((file==0)||(file->is_dir == true))
		return NULL;

	return file;
}

void handleDelfileConfirmed(void)
{
	deleteMessageBox();

	File *file = getSelectedFile();
	if(file==0) return;
	debugprintf("%s\n", file->name_with_path.c_str());

	const char *fn = file->name_with_path.c_str();
	if (unlink(fn)) {
		showMessage("error deleting file", true);
		updateFilesystemState(false);
	} else {
		updateFilesystemState(true);
	}
}

void handleDelfile(void)
{
	File *file = getSelectedFile();
	if(file==0) return;
	debugprintf("%s\n", file->name_with_path.c_str());

	mb = new MessageBox(sub_screen, "are you sure?", 2, "yes", handleDelfileConfirmed, "no", deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
	mb->pleaseDraw();
}


void loadSong(void)
{
	deleteMessageBox();

	File *file = getSelectedFile();
	if(file==0) return;

	pv->unmuteAll();
	deleteSong(); // TODO: Do some checks before deleting the song?

	mod_loading = true;
	showSlowLoadOperation([file](){
	    const char *fn = file->name_with_path.c_str();
		Song *newsong;
		FormatTransportError err;
		if(!strcasecmp(fn + strlen(fn) - 3, ".xm"))
		    err = xm_transport.load(fn, &newsong);
		else
		    err = mod_transport.load(fn, &newsong);
		if (err != FormatTransportError::SUCCESS)
		{
			setSong(new Song());
			return xm_transport.getError(err);
		}
		else
		{
			setSong(newsong);
			return (const char*) NULL;
		}
	});
	setHasUnsavedChanges(false);
}

void handleLoad(void)
{
	File *file = getSelectedFile();
	if(file==0) return;

	const char *fn = file->name.c_str();
	if(!strcasecmp(fn + strlen(fn) - 3, ".xm") || !strcasecmp(fn + strlen(fn) - 4, ".mod"))
	{
		stopPlay();

		if (state->unsaved_changes) {
			mb = new MessageBox(sub_screen, "you have unsaved changes", 2, "load", loadSong, "cancel", deleteMessageBox);
			gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
			mb->reveal();
			mb->pleaseDraw();
		} else
			loadSong();

	}
	else if(!strcasecmp(fn + strlen(fn) - 4, ".wav"))
	{
		showSlowLoadOperation([file](){
			bool success = loadSample(file->name_with_path.c_str());
			return !success ? "wav loading failed" : (const char*) NULL;
		});
	}
}



// Reads filename and path from fileselector and saves the file
void saveFile(void)
{
	stopPlay();

	char *filename = labelFilename->getCaption();

	// Create a .tmp file first, so the original file is not corrupted in the case of a crash
	char *filename_tmp = (char*) ntxm_cmalloc(strlen(filename) + 5);
	strcpy(filename_tmp, filename);
	strcat(filename_tmp, ".tmp");

	chdir(fileselector->getDir().c_str());

	debugprintf("saving %s ...\n", filename);

	mb = new MessageBox(sub_screen, "one moment", 0);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->show();
	mb->pleaseDraw();

	bool saved = false;
	FormatTransportError err = FormatTransportError::SUCCESS;
	if(rbsong->getActive() == true) // Save the song
	{
		if(song != 0) {
			err = xm_transport.save(filename_tmp, song);
			saved = true;
		}
	}
	else if(rbsample->getActive() == true) // Save the sample
	{
		if(song != 0) {
			auto inst = song->getInstrument(state->instrument);
			if (inst != 0) {
				auto smp = inst->getSample(state->sample);
				if (smp != 0) {
					smp->saveAsWav(filename_tmp);
					saved = true;
				}
			}
		}
	}

	if (saved && err == FormatTransportError::SUCCESS) {
		unlink(filename);
		rename(filename_tmp, filename);
	}
	ntxm_free(filename_tmp);

	deleteMessageBox();
	updateFilesystemState(true);

	debugprintf("done\n");

	if(err != FormatTransportError::SUCCESS)
	{
		showMessage(xm_transport.getError(err), true);
	} else
		setHasUnsavedChanges(false);
}

void mbOverwrite(void) {

	deleteMessageBox();
	saveFile();
}

void handleSave(void)
{
	// sporadic filename sanity check
	char *filename = labelFilename->getCaption();
	if(strlen(filename)==0) {
		showMessage("no filename!", true);
		return;
	}

	if(rbsample->getActive() == true) // Sample sanity checks
	{
		Instrument *inst = song->getInstrument(state->instrument);
		if(inst == NULL)
		{
			showMessage("empty instrument!", true);
			return;
		}
		Sample *smp = inst->getSample((state->sample));
		if(smp == NULL)
		{
			showMessage("empty sample!", true);
			return;
		}
	}

	chdir(fileselector->getDir().c_str());

	// Check if file already exists
	if(ntxm_isFileExists(filename))
	{
		mb = new MessageBox(sub_screen, "overwrite file", 2, "yes", mbOverwrite, "no", deleteMessageBox);
		gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
		mb->reveal();
		mb->pleaseDraw();
	} else {
		saveFile();
	}

	updateMemoryState(false);
}


void handleDiskOPChangeFileType(u8 newidx)
{
	if(newidx==FILETYPE_SONG)
	{
		fileselector->setDir(settings->getSongPath());

		fileselector->selectFilter("song");
		cbsamplepreview->hide();

		labelFilename->setCaption(state->song_filename);
	}
	else if(newidx==FILETYPE_SAMPLE)
	{
		fileselector->setDir(settings->getSamplePath());

		fileselector->selectFilter("sample");
		cbsamplepreview->show();
		tabbox->pleaseDraw();

		labelFilename->setCaption(state->sample_filename);
	}
	else if(newidx==FILETYPE_INST)
	{
		fileselector->selectFilter("instrument");
	}

	fileselector->pleaseDraw();
}


void deleteTypewriter(void)
{
    gui->unregisterOverlayWidget();
    typewriter_active = false;
#if defined(NT_PLATFORM_NDS)
    if(tw)
    {
        delete tw;
        tw = NULL;
    }
#endif
    twOkCallback = NULL;
	redrawSubScreen();
}

void clearTypewriterText(void)
{
#if defined(NT_PLATFORM_NDS)
	tw->setText("");
#endif
}

void handleTypewriterFilenameOk(const char *text)
{
	char *name = NULL;
	int textlen = strlen(text);
	debugprintf("%s\n", text);
	if(strcmp(text,"") != 0)
	{
		if( (rbsong->getActive() == true) && (textlen <= 3 || strcasecmp(text+textlen-3, ".xm") != 0) )
		{
			// Append extension
			name = (char*)ntxm_cmalloc(textlen+3+1);
			strcpy(name,text);
			strcpy(name+textlen,".xm");
		}
		else if( (rbsample->getActive() == true) && (textlen <= 4 || strcasecmp(text+textlen-4, ".wav") != 0) )
		{
			// Append extension
			name = (char*)ntxm_cmalloc(textlen+4+1);
			strcpy(name,text);
			strcpy(name+textlen,".wav");
		}
		else
		{
			// Leave as is
			name = (char*)ntxm_cmalloc(textlen+1);
			strcpy(name,text);
		}
		labelFilename->setCaption(name);

		// Remember the name
		if(rbsong->getActive() == true)
		{
			strcpy(state->song_filename, name);
		}
		else if(rbsample->getActive() == true)
		{
			strcpy(state->sample_filename, name);
		}
	}
	deleteTypewriter();
	if (name != NULL) ntxm_free(name);
}


void emptyNoteStroke(void) {
	handleNoteFill(EMPTY_NOTE);
	redraw_main_requested = true;
}


void stopNoteStroke(void) {
	handleNoteFill(STOP_NOTE);
	redraw_main_requested = true;
}

void copyFxParam(void) {
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	uiPotSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2, false);

	// if multiple cells are selected, it's ambiguous which one they want to
	// get the param of
	if (sel_x1 != sel_x2 || sel_y1 != sel_y2)
		return;

	Cell targetcell = song->getPattern(song->getPotEntry(state->potpos))[sel_x1][sel_y1];

	u8 prm = targetcell.effect_param;
	u8 prm2 = targetcell.effect2_param; // if no main param

	if (prm == 0 && prm2 != 0)
		dbeffectpar->setValue(prm2);
	else
		dbeffectpar->setValue(prm);


	redraw_main_requested = true;
}

static void actionBufferChangeCallback(void) {
	buttonundo->set_enabled(action_buffer->can_undo());
	buttonredo->set_enabled(action_buffer->can_redo());
	if (action_buffer->can_undo() || action_buffer->can_redo()) setHasUnsavedChanges(true);
	redraw_main_requested = true;
}

void undoOp(void) {
	action_buffer->undo(song);
}

void redoOp(void) {
	action_buffer->redo(song);
}

void delNote(void) // Delete a cell and move the cells below it up
{
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	uiPotSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2, true);

	//if(!state->recording) return;
	action_buffer->add(song, new CellDeleteAction(state, sel_x1, sel_y1, sel_x2 - sel_x1 + 1, sel_y2 - sel_y1 + 1));

	redraw_main_requested = true;
}


void insNote(void)
{
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	uiPotSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2, true);

	action_buffer->add(song, new CellInsertAction(state, sel_x1, sel_y1, sel_x2 - sel_x1 + 1, sel_y2 - sel_y1 + 1));

	redraw_main_requested = true;
}


void changeAdd(u8 newadd) {
	state->add = newadd;
}


void changeOctave(u8 newoctave)
{
	state->basenote = 12*newoctave;

	if(lbsamples->is_visible() == true)
		drawSampleNumbers();
}

void handleEffectsCategoryChange(u8 newcat)
{
	fxkb->setCategory(newcat);
	dbeffectpar->setSingleDigit(newcat == FX_CATEGORY_E);
}


void drawMainScreen(void)
{
	// Draw widgets (to back buffer)
	gui->drawMainScreen();

	PlatformFlipMainScreen();
}

void redrawSubScreen(void)
{
	u16 col = settings->getTheme()->col_bg;
#ifdef NT_PLATFORM_NDS
    // clean only ~3/4ths of the screen, as the rest is covered by the piano
	u32 colcol = col | col << 16;
	dmaFillWords(colcol, sub_screen->pixels, 256 * 153 * 2);
#else
	PlatformClearSubScreen(col);
#endif

	// Redraw GUI
	gui->drawSubScreen();
}


// Called on every tick when the song is playing
void HandleTick(void)
{
	//drawMainScreen();
}


void startPlay(void)
{
	// Send play command
	if(state->pause == false)
		CommandStartPlay(state->potpos, 0, true);
	else
		CommandStartPlay(state->potpos, state->getCursorRow(), true);

	state->playing = true;
	state->pause = false;

	buttonplay->hide();
	buttonpause->show();
}


void stop(void)
{
	// Send stop command
	CommandStopPlay();

	// Also stop a previewing sample, if there is one.
	if (state->preview_sample)
		CommandStopSample(0);

	state->playing = false;

	// The arm7 will get the command with a slight delay and may continue playing for
	// some ticks. But for saving battery, we only draw the screen continuously
	// if state->playing == true. So, by setting it to false here we might miss ticks
	// resultsing in the pattern view being out of sync with the song. So we wait two
	// frames to make sure the arm7 has really stopped and redraw the pattern.
	PlatformWaitVBlank(); PlatformWaitVBlank();
	redraw_main_requested = false;
	drawMainScreen();
}

void stopPlay(void)
{
	state->pause = false;
	state->setPlaybackRow(0);

	updateSampleOffsetGuide();

	stop();

	buttonpause->hide();
	buttonplay->show();
}

void pausePlay(void)
{
	state->pause = true;

	// Send stop command
	CommandStopPlay();

	updateSampleOffsetGuide();

	buttonpause->hide();
	buttonplay->show();
}

bool potGoto(u8 pos)
{
	if(state->playing == true) {
		if (tbqueuelock->getState()) {
			state->queued_potpos = pos;
			lbpot->select(state->potpos, false);
			lbpot->highlight(state->queued_potpos, true);
			return false;
		} else {
			state->potpos = pos;
			state->setPlaybackRow(0);
			if (state->pause == false) {
				CommandStartPlay(state->potpos, state->getPlaybackRow(), true);
			}
			return true;
		}
	} else {
		state->potpos = pos;
		state->setPlaybackRow(0);
		return true;
	}
}


void setRecordMode(bool is_on)
{
	state->recording = is_on;
	redraw_main_requested = false;
	drawMainScreen(); // <- must redraw because of orange lines

	// Draw border
	u16 col;

	if(is_on)
		col = settings->getTheme()->col_signal; // red
	else
		col = settings->getTheme()->col_bg; // bg color

#ifdef NT_PLATFORM_NDS
	u32 colcol = (col) | (col << 16);
	dmaFillWords(colcol, sub_screen->pixels, 256 * 2);
	dmaFillWords(colcol, sub_screen->pixels + (256*191), 256 * 2);
#else
	for(int i=0; i<sub_screen->getWidth(); ++i)
	{
		sub_screen->drawPixel(i, 0, col);
		sub_screen->drawPixel(i, sub_screen->getHeight() - 1, col);
	}
#endif

	for(int i=1; i<sub_screen->getHeight()-1; ++i)
	{
		sub_screen->drawPixel(0, i, col);
		sub_screen->drawPixel(sub_screen->getWidth() - 1, i, col);
	}
}


// Updates several GUI elements that display pattern
// related info to the new pattern
void updateGuiToNewPattern(u8 newpattern)
{
	// Update pattern length slider
	nsptnlen->setValue(song->getPatternLength(newpattern));

	buttonpotdown->set_enabled(newpattern > 0);
	buttonpotup->set_enabled(newpattern < MAX_PATTERNS-1);
}


// Callback called from song when the pot element changes during playback
void handlePotPosChangeFromSong(u16 newpotpos)
{
	if (newpotpos != state->potpos)
		pv->clearSelection();

	if(newpotpos>=song->getPotLength()) {
		newpotpos = song->getPotLength() - 1;
	}

	if (state->queued_potpos >= 0) {
		state->potpos = state->queued_potpos;
		state->setPlaybackRow(0);

		CommandStartPlay(state->potpos, state->getPlaybackRow(), true);
		state->queued_potpos = -1;
		lbpot->highlight(state->queued_potpos, false);
	} else {
		state->potpos = newpotpos;
		state->setPlaybackRow(0);
	}

	// Update lbpot
	lbpot->select(state->potpos);

	// Update other GUI Elements
	updateGuiToNewPattern(song->getPotEntry(state->potpos));

#if defined(NT_PLATFORM_NDS)
	if (tw)
		tw->pleaseDraw();
#endif

	if (mb)
		mb->pleaseDraw();
}

// Callback called from lbpot when the user changes the pot element
void handlePotPosChangeFromUser(u16 newpotpos)
{
	if (newpotpos != state->potpos)
		pv->clearSelection();

	// Update potpos in song
	if(newpotpos>=song->getPotLength()) {
		newpotpos = song->getPotLength() - 1;
	}
	if (!potGoto(newpotpos)) return;

	// Update other GUI Elements
	updateGuiToNewPattern(song->getPotEntry(newpotpos));

	redraw_main_requested = true;
}

void handlePotDec(void) {

	u8 pattern = song->getPotEntry(state->potpos);
	if(pattern>0) {
		pattern--;
		song->setPotEntry(state->potpos, pattern);
		// TODO: turn into undo operation
		action_buffer->clear();
		// If the current pos was changed, switch the pattern
		ntxm_flush_dcache();

		redraw_main_requested = true;

		// Update pattern length slider
		nsptnlen->setValue(song->getPatternLength(song->getPotEntry(state->potpos)));
	}
	char str[3];
	snprintf(str, sizeof(str), "%2x", pattern);
	lbpot->set(state->potpos, str);
	buttonpotdown->set_enabled(song->getPotEntry(state->potpos) > 0);
	buttonpotup->set_enabled(song->getPotEntry(state->potpos) < MAX_PATTERNS-1);
	setHasUnsavedChanges(true);
}


void handlePotInc(void)
{
	u8 pattern = song->getPotEntry(state->potpos);
	if(pattern<MAX_PATTERNS-1) {
		pattern++;

		// Add new pattern if patterncount exceeded
		if(pattern > song->getNumPatterns()-1) {
			song->addPattern(nsptnlen->getValue());
		}

		song->setPotEntry(state->potpos, pattern);
		// TODO: turn into undo operation
		action_buffer->clear();
		ntxm_flush_dcache();

		redraw_main_requested = true;

		// Update pattern length slider
		nsptnlen->setValue(song->getPatternLength(song->getPotEntry(state->potpos)));
	}
	char str[3];
	snprintf(str, sizeof(str), "%2x", pattern);
	lbpot->set(state->potpos, str);
	buttonpotdown->set_enabled(song->getPotEntry(state->potpos) > 0);
	buttonpotup->set_enabled(song->getPotEntry(state->potpos) < MAX_PATTERNS-1);
	setHasUnsavedChanges(true);
}


// Inserts a pattern into the pot (copies the current pattern)
void handlePotIns(void)
{
	if (!song->potIns(state->potpos, song->getPotEntry(state->potpos)))
		return;

	buttondel->set_enabled(song->getPotLength()>1);

	// TODO: turn into undo operation
	action_buffer->clear();
	ntxm_flush_dcache();
	lbpot->ins(lbpot->getidx(), lbpot->get(lbpot->getidx()));
	updateLabelSongLen();
	setHasUnsavedChanges(true);
}


void handlePotDel(void)
{
	if(song->getPotLength()>1) {
		lbpot->del();
	}

	song->potDel(state->potpos);
	buttondel->set_enabled(song->getPotLength()>1);

	// TODO: turn into undo operation
	action_buffer->clear();
	ntxm_flush_dcache();

	if(state->potpos>=song->getPotLength()) {
		state->potpos = song->getPotLength() - 1;
	}

	updateLabelSongLen();

	if(song->getRestartPosition() >= song->getPotLength()) {
		song->setRestartPosition( song->getPotLength() - 1 );
		nsrestartpos->setValue( song->getRestartPosition() );
		ntxm_flush_dcache();
	}
	setHasUnsavedChanges(true);
}

void handlePtnClone(void)
{
	if (song->getPotLength() >= MAX_POT_LENGTH)
		return;
	u16 newidx = song->getNumPatterns();
	if(newidx >= MAX_PATTERNS)
		return;

	u16 ptnlength = song->getPatternLength(song->getPotEntry(state->potpos));
	song->addPattern(ptnlength);
	song->potIns(state->potpos+1, newidx);

	Cell **srcpattern = song->getPattern(song->getPotEntry(state->potpos));
	Cell **destpattern = song->getPattern(newidx);

	for(u16 chn=0; chn<song->getChannels(); ++chn) {
		for(u16 row=0; row<ptnlength; ++row) {
			destpattern[chn][row] = srcpattern[chn][row];
		}
	}

	// TODO: turn into undo operation
	action_buffer->clear();
	ntxm_flush_dcache();
	char numberstr[3] = {0};
	sprintf(numberstr, "%2x", newidx);
	lbpot->ins(lbpot->getidx()+1, numberstr);

	updateLabelSongLen();
	setHasUnsavedChanges(true);
}

void handleChannelAdd(void)
{
	// TODO: turn into undo operation
	song->channelAdd();

	buttonlesschannels->enable();
	buttonmorechannels->set_enabled(song->getChannels() < MAX_CHANNELS);


	redraw_main_requested = true;
	updateLabelChannels();
	setHasUnsavedChanges(true);
}


void handleChannelDel(void)
{
	// TODO: turn into undo operation
	song->channelDel();

	buttonmorechannels->enable();
	buttonlesschannels->set_enabled(song->getChannels() > 1);

	// Move back cursor if necessary
	if(state->channel > song->getChannels()-1) {
		state->channel = song->getChannels()-1;
	}

	// Unmute channel if it is muted
	if(pv->isMuted(song->getChannels()))
	{
		pv->unmute(song->getChannels());
	}

	// Unsolo channel is it is solo
	if(pv->soloChannel() == song->getChannels())
	{
		pv->unmuteAll();
	}

	redraw_main_requested = true;
	updateLabelChannels();

	u16 x1, y1, x2, y2;
	if(pv->getSelection(&x1, &y1, &x2, &y2) == true) {
		if (x2 >= song->getChannels()-1) pv->setSelection(x1, y1, song->getChannels()-1, y2);
	}

	setHasUnsavedChanges(true);
}


void handlePtnLengthChange(s32 newlength)
{
	// TODO: turn into undo operation
	if(newlength != song->getPatternLength(song->getPotEntry(state->potpos)))
	{
		song->resizePattern(song->getPotEntry(state->potpos), newlength);
		ntxm_flush_dcache();
		// Scroll back if necessary
		if(state->getPlaybackRow() >= newlength) {
			state->setPlaybackRow(newlength-1);
		}
		if(state->getCursorRow() >= newlength) {
			state->setCursorRow(newlength-1);
		}

		u16 x1, y1, x2, y2;
		if(pv->getSelection(&x1, &y1, &x2, &y2) == true) {
			if (y2 >= newlength) pv->setSelection(x1, y1, x2, newlength-1);
		}

		redraw_main_requested = true;
		setHasUnsavedChanges(true);
	}
}


void handleTempoChange(u8 tempo) {
	song->setTempo(tempo);
	setHasUnsavedChanges(true);
	ntxm_flush_dcache();
}

void handleBpmChange(s32 bpm) {
	song->setBpm(bpm);
	setHasUnsavedChanges(true);
	ntxm_flush_dcache();
}

void handleLinesBeatChange(u8 lpb) {
	settings->setLinesPerBeat(lpb);
	pv->setLinesPerBeat(lpb);
	redraw_main_requested = true;
	setHasUnsavedChanges(true);
}

void handleRestartPosChange(s32 restartpos)
{
	if(restartpos > song->getPotLength()-1) {
		nsrestartpos->setValue(song->getPotLength()-1);
		restartpos = song->getPotLength()-1;
	}
	song->setRestartPosition(restartpos);
	setHasUnsavedChanges(true);
	ntxm_flush_dcache();
}

void confirmZap(void (*onConfirm)(void))
{
	deleteMessageBox();
	mb = new MessageBox(sub_screen, "are you sure", 2, "zap", onConfirm, "cancel", deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
}

void zapPatterns(void)
{
	song->zapPatterns();
	action_buffer->clear();
	ntxm_flush_dcache();
	deleteMessageBox();

	// Update POT
	lbpot->clear();
	lbpot->add(" 0");

	updateLabelSongLen();
	updateLabelChannels();
	updateGuiToNewPattern(0);

	state->potpos = 0;
	state->setPlaybackRow(0);
	state->setCursorRow(0);
	state->channel = 0;

	redraw_main_requested = false;
	drawMainScreen();

	CommandSetSong(song);
	updateMemoryState(true);
}

void zapUnusedInstruments(void) {
	bool used_insts[MAX_INSTRUMENTS] = { false };

	song->zapUnusedInstruments(used_insts);

	for (int i = 0; i < MAX_INSTRUMENTS; i++) {
		if (used_insts[i]) continue;

		lbinstruments->set(i, "");
		if (lbinstruments->getidx() == i) {
			sampledisplay->setSample(NULL);
			handleSampleChange(0);
			for(u8 i=0;i<MAX_INSTRUMENT_SAMPLES;++i) {
				lbsamples->set(i, "");
			}
		}
	}

	ntxm_flush_dcache();
	deleteMessageBox();
	CommandSetSong(song);
	updateMemoryState(true);
}

void zapCurrentInstrument(void) {
	PrintFreeMem();
	u8 inst = lbinstruments->getidx();
	song->zapInstrument(inst);
	sampledisplay->setSample(NULL);
	handleSampleChange(0);
	ntxm_flush_dcache();
	deleteMessageBox();

	lbinstruments->set(inst, "");
	for(u8 i=0;i<MAX_INSTRUMENT_SAMPLES;++i) {
		lbsamples->set(i, "");
	}
	CommandSetSong(song);
	updateMemoryState(true);
}

void zapInstruments(void)
{
	song->zapInstruments();
	ntxm_flush_dcache();
	deleteMessageBox();

	// Update instrument list
	for(u8 i=0;i<MAX_INSTRUMENTS;++i) {
		lbinstruments->set(i, "");
	}

	// Update sample list
	for(u8 i=0;i<MAX_INSTRUMENT_SAMPLES;++i) {
		lbsamples->set(i, "");
	}

	// Clear sample display
	sampledisplay->setSample(0);

	CommandSetSong(song);
	updateMemoryState(true);
	setHasUnsavedChanges(true);
}


void zapSong(void) {
	deleteMessageBox();
	deleteSong();
	setSong(new Song());
	updateMemoryState(true);
}

void confirmZapSong(void)
{
	confirmZap(zapSong);
}

void confirmZapInsts(void)
{
	confirmZap(zapInstruments);
}

void confirmZapPatterns(void)
{
	confirmZap(zapPatterns);
}

void zapInstrumentsChoice(void)
{
	deleteMessageBox();
	mb = new MessageBox(sub_screen, "which instruments", 4, "selected", zapCurrentInstrument, "unused", zapUnusedInstruments,
		"  all  ", confirmZapInsts, "cancel",
		deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
}

void handleZap(void)
{
	stopPlay(); // Safety first

	mb = new MessageBox(sub_screen, "what to zap", 4, "patterns", confirmZapPatterns,
		"instruments", zapInstrumentsChoice, "song", confirmZapSong, "cancel",
		deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
}

void handleRowChangeFromSong(u16 row)
{
	state->setPlaybackRow(row);

	if(!state->playing)
		return;

	redraw_main_requested = true;

	dsmidi_handler.rowUpdate(song, state->getCursorRow(), state->potpos);
}

void handleStop(void)
{
	state->playing = false;
}

void handleSamplePreviewToggled(bool on)
{
	settings->setSamplePreview(on);
}



u32 calcFileSize(const char *path) {
	struct stat filestats;
	int stat_res = stat(path, &filestats);

	if(stat_res != -1) {
		return filestats.st_size;
	}

	return 0;
}

void previewWav(void) {
	if (mb != NULL)
		deleteMessageBox();

	debugprintf("previewing\n");

	// Load sample
	bool success;
	Sample *smp = new Sample(preview_smp_path, false, &success);
	if(!success)
	{
		delete smp;
		return;
	}

	updateMemoryState(false);

	// Stop and delete previously playing preview sample
	if(state->preview_sample)
		CommandStopSample(0);

	// Wait until previously playing preview sample is deleted
	while(state->preview_sample)
		PlatformWaitVBlank();

	// Play it
	state->preview_sample = smp;
	ntxm_flush_dcache();
	CommandPlaySample(smp, 4*12, 255, 0);

	// When the sample has finished playing, the arm7 sends a signal,
	// so the arm9 can delete the sample
}

void confirmWavPreview(void)
{
	if (mb != 0) deleteMessageBox();
	mb = new MessageBox(sub_screen, "preview large audio file?", 2, "preview", previewWav, "cancel", deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
}

void handleFileChange(File file)
{
	if(!file.is_dir)
	{
		const char *str = file.name.c_str();
		int slen = strlen(str);
		labelFilename->setCaption(str);

		if(rbsong->getActive() == true)
		{
			strncpy(state->song_filename, str, STATE_FILENAME_LEN);
		}
		else if(rbsample->getActive() == true)
		{
			strncpy(state->sample_filename, str, STATE_FILENAME_LEN);
		}

		// Preview WAV files
		if(slen > 4 && (strcasecmp(&str[slen-4], ".wav") == 0) && (settings->getSamplePreview() == true) )
		{
			// Pause song playback if ongoing
			if(state->playing)
				pausePlay();

			if (preview_smp_path != NULL)
			{
				ntxm_free(preview_smp_path);
				preview_smp_path = NULL;
			}

			preview_smp_path = ntxm_ustrdup(file.name_with_path.c_str());
			if (!preview_smp_path)
			{
				showMessage("not enough ram free!", true);
				return;
			}

			if (calcFileSize(str) > 3 * 1024 * 1024 /* 3MiB */)
				confirmWavPreview();
			else
				previewWav();
		}
	}
}

void handleDirChange(const char *newdir)
{
	if(rbsong->getActive() == true)
	{
		settings->setSongPath(newdir);
	}
	else if(rbsample->getActive() == true)
	{
		settings->setSamplePath(newdir);
	}
}

void handlePreviewSampleFinished(void)
{
	debugprintf("Sample finished\n");
	delete state->preview_sample;
	state->preview_sample = 0;

	// FIXME: this is still inside an IRQ, so may freeze
	// updateMemoryState(false);
}

void setNoteVol(u16 vol)
{
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	uiPotSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2, false);
    CellArray *fill = new CellArray(sel_x2 - sel_x1 + 1, sel_y2 - sel_y1 + 1);
    if (fill != NULL && fill->valid())
    {
		for (u16 chn = sel_x1; chn <= sel_x2; chn++)
			for (u16 row = sel_y1; row <= sel_y2; row++)
			{
				Cell cell = song->getPattern(song->getPotEntry(state->potpos))[chn][row];
				cell.volume = vol;
				*fill->ptr(chn - sel_x1, row - sel_y1) = cell;
			}
        action_buffer->add(song, new MultipleCellSetAction(state, sel_x1, sel_y1, fill, false));
		redraw_main_requested = true;
	}
}

void setEffectCommand(u16 eff)
{
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	uiPotSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2, false);
    CellArray *fill = new CellArray(sel_x2 - sel_x1 + 1, sel_y2 - sel_y1 + 1);
    if (fill != NULL && fill->valid())
    {
		for (u16 chn = sel_x1; chn <= sel_x2; chn++)
			for (u16 row = sel_y1; row <= sel_y2; row++)
			{
				Cell cell = song->getPattern(song->getPotEntry(state->potpos))[chn][row];
				cell.effect = eff;
				*fill->ptr(chn - sel_x1, row - sel_y1) = cell;
			}
        action_buffer->add(song, new MultipleCellSetAction(state, sel_x1, sel_y1, fill, false));
		redraw_main_requested = true;
    	updateSampleOffsetGuide();
	}
}

void setEffectParam(u16 eff_par, bool new_e_cmd, bool force_clear=false, bool overwrite=true)
{
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	uiPotSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2, false);
    CellArray *fill = new CellArray(sel_x2 - sel_x1 + 1, sel_y2 - sel_y1 + 1);
    if (fill != NULL && fill->valid())
    {
		for (u16 chn = sel_x1; chn <= sel_x2; chn++)
			for (u16 row = sel_y1; row <= sel_y2; row++)
			{
				Cell cell = song->getPattern(song->getPotEntry(state->potpos))[chn][row];

				bool cell_has_param = cell.effect_param != 0xff && cell.effect_param != 0x0;


				if (force_clear)
					eff_par = 0x00;
				// this gets messy because Exy commands use the param for both command and param info D:
				else if (fxkb->getCategory() == FX_CATEGORY_E)
				{
					u8 ecmd_cmd = eff_par & 0xF0;
					u8 ecmd_par = eff_par & 0x0F;

					if (new_e_cmd) {
						// if existing fx parameter in cell, only update the E command (the first digit)
						if (cell_has_param) {
							ecmd_cmd = eff_par & 0xF0;
							ecmd_par = cell.effect_param & 0x0F;
						}
					} else {
						// ..or if only new param, only alter the second digit, to keep the E command
						if (cell_has_param) {
							ecmd_cmd = cell.effect_param & 0xF0;
							ecmd_par = eff_par & 0x0F;
						// ....OR if the cell has no parameter, use whatever E button they last pressed
						// (otherwise 0)
						} else {
							ecmd_cmd = fxkb->getLastCmd() << 4;
							ecmd_par = eff_par & 0x0F;
						}
					}

					eff_par = ecmd_cmd | ecmd_par;
				}

				if (!cell_has_param || overwrite) cell.effect_param = eff_par;
				*fill->ptr(chn - sel_x1, row - sel_y1) = cell;
			}
        action_buffer->add(song, new MultipleCellSetAction(state, sel_x1, sel_y1, fill, false));
		redraw_main_requested = true;

    	updateSampleOffsetGuide();
	}
}

void handleTranspose(s32 transpose_amount)
{
	const s32 min_note = 0;  // c-0
	const s32 max_note = 95; // h-7
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	uiPotSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2, false);
	CellArray *fill = new CellArray(sel_x2 - sel_x1 + 1, sel_y2 - sel_y1 + 1);
	if (fill != NULL && fill->valid())
	{
		for (u16 chn = sel_x1; chn <= sel_x2; chn++)
			for (u16 row = sel_y1; row <= sel_y2; row++)
			{
				Cell cell = song->getPattern(song->getPotEntry(state->potpos))[chn][row];
				if (cell.note != EMPTY_NOTE && cell.note != STOP_NOTE)
				{
					s32 new_note = (s32)cell.note + transpose_amount;
					if (new_note >= min_note && new_note <= max_note)
						cell.note = (u8)new_note;
				}
				*fill->ptr(chn - sel_x1, row - sel_y1) = cell;
			}
		action_buffer->add(song, new MultipleCellSetAction(state, sel_x1, sel_y1, fill, false));
		redraw_main_requested = true;
		updateSampleOffsetGuide();
	}
}

void destroyThemeDialog(void)
{
	gui->unregisterOverlayWidget();
	delete fbtheme;
	fbtheme = 0;
	redrawSubScreen();
}

void reloadSkin(void)
{
	gui->setTheme(settings->getTheme(), settings->getTheme()->col_bg);

#ifdef NT_PLATFORM_NDS
	// fill the quirky little square next to the piano
	for (int y = 153; y < 192;++y)
	{
		u16 col = settings->getTheme()->col_bg;
		u32 colcol = col | col << 16;
		dmaFillWords(colcol, sub_screen->pixels + (256 * y) + 224, (256 - 224) * 2);
	}
#endif

	gui->draw();
	redrawSubScreen();
	setRecordMode(state->recording);
	redraw_main_requested = true;
}

void handleThemeChosen(File file)
{
	if(!file.is_dir)
	{
		const char *str = file.name.c_str();
		int slen = strlen(str);

		if(slen > 8 && (strcasecmp(&str[slen-8], ".nttheme") == 0))
		{
			settings->getTheme()->loadTheme(file.name_with_path.c_str());
			settings->setThemePath(file.name_with_path.c_str());

			reloadSkin();
		}
	}
}


void handleThemeCancel(void)
{
	settings->getTheme()->loadTheme(last_themepath);

	settings->setThemePath(last_themepath);
	reloadSkin();

	destroyThemeDialog();
}

void handleThemeReset(void)
{
	destroyThemeDialog();

	settings->getTheme()->loadDefault();
	settings->setThemePath("/");
	settings->writeIfChanged();
	reloadSkin();
}

void handleThemeApply(void)
{
	settings->writeIfChanged();
	destroyThemeDialog();
}

void handleThemeButton(void)
{
	pausePlay();
	strncpy(last_themepath, settings->getThemePath(), SETTINGS_FILENAME_LEN);
	fbtheme = new tobkit::ThemeSelectorBox(sub_screen, handleThemeChosen, handleThemeApply, handleThemeReset, handleThemeCancel);
	std::string themepath_(settings->getThemePath());
	fbtheme->setDir(themepath_.substr(0, themepath_.find_last_of("/")));
	gui->registerOverlayWidget(fbtheme, 0, SUB_SCREEN);
	fbtheme->reveal();
}

void handleTransposeUp(void)
{
	handleTranspose((PlatformKeysHeld & PlatformKey_R) ? 12 : 1);
}

void handleTransposeDown(void)
{
	handleTranspose((PlatformKeysHeld & PlatformKey_R) ? -12 : -1);
}

// number slider
void handleNoteVolumeChanged(s32 vol)
{
	setNoteVol(vol);
}

// button
void handleSetNoteVol(void)
{
	setNoteVol(nsnotevolume->getValue());
}

void handleToggleEffectsVisibility(bool on)
{
	pv->toggleEffectsVisibility(on);

	// order of hiding/showing is important to avoid
	// inappropriate bg overdraw over widgets
	if (on)
	{
		if (tbmultisample->getState())
			setMultisamplesEnabled(false);

		tbeffects->setState(true);

		kb->hide();
		kb->disable();

		labeloct->hide();
		labelfxcat->show();
		numberboxoctave->hide();
		numberboxfxcat->show();
		fxkb->show();
		fxkb->enable();
		fxkb->setCategory(fxkb->getCategory());

		dbeffectpar->show();
		labeleffectpar->show();
		buttonseteffectpar->show();

		buttoninsnote2->hide();
		buttondelnote2->hide();
		buttonemptynote->hide();
		buttonstopnote->hide();

		labelfxop->show();
		buttonlerpfx->show();
		buttonemptyfx->show();
		buttoncpprm->show();
	}
	else
	{
		tbeffects->setState(false);

		if (!PlatformVideoAreScreensSwapped())
			pv->clearSelection();

		kb->show();
		fxkb->hide();
		numberboxfxcat->hide();
		labelfxcat->hide();
		labeloct->show();
		numberboxoctave->show();
		kb->enable();
		fxkb->disable();
		dbeffectpar->hide();
		labeleffectpar->hide();
		buttonseteffectpar->hide();

		labelfxop->hide();
		buttonlerpfx->hide();
		buttonemptyfx->hide();
		buttoncpprm->hide();

		buttoninsnote2->show();
		buttondelnote2->show();
		buttonemptynote->show();
		buttonstopnote->show();
	}

	buttonundo->pleaseDraw(); // gets occluded by oct/cat label bg otherwise

	pv->recalcHscroll();
	updateSampleOffsetGuide();
	setRecordMode(state->recording); // ensure red border gets drawn!
}

void onFxKeyPressed(u8 val)
{
	if (val == NO_EFFECT || !state->recording) return;
	// for E effects, the button's val is the E sub-command, rather than just 'E'

	if (fxkb->getCategory() == FX_CATEGORY_E) {
		setEffectCommand(0xE);
		setEffectParam((val << 4) | (dbeffectpar->getValue() & 0x0f), true);
	} else {
		setEffectCommand(val);
		setEffectParam(dbeffectpar->getValue(), false, false, false);
	}

	pv->clearSelection();
	handleNoteAdvanceRow();
}

// box arrows or pen slide
void handleEffectParamChanged(u8 eff_par)
{
	if (!state->recording) return;
	setEffectParam(eff_par, false);
}

// "set" button
void handleSetEffectParam(void)
{
	setEffectParam(dbeffectpar->getValue(), false);
	pv->clearSelection();
	handleNoteAdvanceRow();
}

void handleClearFx(void)
{
	setEffectParam(0, false, true);
	setEffectCommand(0xff);
	pv->clearSelection();
}

struct TypewriterState {
    const char *prompt;
    const char *str;
    void (*okCallback)(const char*);
    void (*clearCallback)(void);
    void (*cancelCallback)(void);
    bool isFileName;
};

#if defined(NT_PLATFORM_NDS)
void handleTypewriterOk()
{
    twOkCallback(tw->getText());
}
#endif

void switchScreens();

void showTypewriter(TypewriterState state)
{
    // TODO: Migrate to new TobKit to eliminate such ugliness
#if defined(NT_PLATFORM_NDS)
#define SUB_BG1_X0 (*(vu16*)0x04001014)
#define SUB_BG1_Y0 (*(vu16*)0x04001016)

	tw = new Typewriter(state.prompt, (u16*)CHAR_BASE_BLOCK_SUB(1),
		(u16*)SCREEN_BASE_BLOCK_SUB(12), 3, sub_screen, &SUB_BG1_X0, &SUB_BG1_Y0, state.isFileName);
	tw->setTheme(settings->getTheme(), settings->getTheme()->col_bg);
	tw->setText(state.str);
	gui->registerOverlayWidget(tw, PlatformKey_LEFT|PlatformKey_RIGHT, SUB_SCREEN);
	if(state.okCallback) {
	    twOkCallback = state.okCallback;
		tw->registerOkCallback(handleTypewriterOk);
	}
	if(state.cancelCallback) {
		tw->registerCancelCallback(state.cancelCallback);
	}
	if(state.clearCallback) {
		tw->registerClearCallback(state.clearCallback);
	}
	typewriter_active = true;
	tw->reveal();
#else
#if defined(NT_PLATFORM_3DS)
    static char text[512];

    // Ensure the bottom screen is on top
    bool unswitch = false;
    if (!PlatformVideoAreScreensSwapped()) {
        switchScreens();
        PlatformWaitVBlank();
        unswitch = true;
    }

    static SwkbdState swkbd;

    swkbdInit(&swkbd, SWKBD_TYPE_QWERTY, 2, -1);
    swkbdSetInitialText(&swkbd, state.str);
    swkbdSetHintText(&swkbd, state.prompt);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "cancel", false);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "ok", true);
    swkbdSetValidation(&swkbd,
        (state.isFileName ? SWKBD_NOTEMPTY_NOTBLANK : SWKBD_ANYTHING),
        (state.isFileName ? SWKBD_FILTER_BACKSLASH : 0), 0);
    swkbdSetFeatures(&swkbd, SWKBD_DARKEN_TOP_SCREEN | SWKBD_ALLOW_HOME | SWKBD_ALLOW_RESET | SWKBD_ALLOW_POWER
        | (state.isFileName ? 0 : SWKBD_PREDICTIVE_INPUT));

    while (aptMainLoop()) {
        swkbdInputText(&swkbd, text, sizeof(text));
        SwkbdResult result = swkbdGetResult(&swkbd);
        if (result < SWKBD_HOMEPRESSED) {
            if (state.okCallback && result == SWKBD_D1_CLICK1) {
                state.okCallback(text);
            } else if (state.cancelCallback) {
                state.cancelCallback();
            }
            break;
        } else {
            if (!aptMainLoop()) {
                if(state.cancelCallback) {
                    state.cancelCallback();
                }
                return;
            }
        }
    }

    if (unswitch) {
        switchScreens();
    }
#else
    if(state.cancelCallback) {
        state.cancelCallback();
    }
#endif
#endif
}


void showTypewriterForFilename(void) {
	showTypewriter({"filename", labelFilename->getCaption(), handleTypewriterFilenameOk, clearTypewriterText, deleteTypewriter, true});
}

void handleTypewriterNewFolderOk(const char *text)
{
	if(text[0] != '\0' && strchr(text, '/') == NULL && strchr(text, ':') == NULL)
	{
		mkdir(text, 0777);
		// TODO: Enter directory after creating it?
		updateFilesystemState(true);
	}
	deleteTypewriter();
}

void showTypewriterForNewFolder(void) {
	showTypewriter({"dir name", "", handleTypewriterNewFolderOk, clearTypewriterText, deleteTypewriter, true});
}

void handleTypewriterInstnameOk(const char *text)
{
	song->getInstrument(lbinstruments->getidx())->setName(text);
	lbinstruments->set( lbinstruments->getidx(), text );

	deleteTypewriter();
}


void showTypewriterForInstRename(void)
{
	Instrument *inst = song->getInstrument(lbinstruments->getidx());
	if(inst==NULL) {
		return;
	}

	showTypewriter({"inst name", lbinstruments->get(lbinstruments->getidx()), handleTypewriterInstnameOk, clearTypewriterText, deleteTypewriter, false});
}

void handleTypewriterSongnameOk(const char *text)
{
	song->setName(text);
	labelsongname->setCaption(song->getName());
	deleteTypewriter();
}

void showTypewriterForSongRename(void)
{
	showTypewriter({"song name", song->getName(), handleTypewriterSongnameOk, clearTypewriterText, deleteTypewriter, false});
}

void handleTypewriterSampleOk(const char *text)
{
	song->getInstrument(lbinstruments->getidx())->getSample(lbsamples->getidx())->setName(text);
	lbsamples->set( lbsamples->getidx(), text );

	deleteTypewriter();
}

void handleLoopToggle(bool on)
{
	CommandSetPatternLoop(on || state->scroll_lock);
}

void handleToggleScrollLock(bool on)
{
	state->scroll_lock = on;
	handleLoopToggle(tbpotloop->getState());
}

void handleToggleMultiSample(bool on)
{
	multisamp_from_mapsamp = false;

	if (!on) {
		handleToggleMapSamples(false);
		tbmapsamples->setState(false);
	}

	setMultisamplesEnabled(on);
}

void showTypewriterForSampleRename(void)
{
	Instrument *inst = song->getInstrument(lbinstruments->getidx());
	if(inst == 0)
		return;

	Sample *sample = inst->getSample(lbsamples->getidx());
	if(sample == 0)
		return;

	showTypewriter({"sample name", lbsamples->get(lbsamples->getidx()), handleTypewriterSampleOk, clearTypewriterText, deleteTypewriter, false});
}

void handleRecordSampleOK(void)
{
	Sample *smp = recordbox->getSample();

	// Kill record box
	gui->unregisterOverlayWidget();
	delete recordbox;

	// Turn off the mic
	CommandMicOff();

	// Add instrument if necessary
	Instrument *inst = song->getInstrument(state->instrument);

	if(inst == 0)
	{
		inst = new Instrument("rec");
		song->setInstrument(state->instrument, inst);

		lbinstruments->set(state->instrument, inst->getName());
	}

	// Insert the sample into the instrument
	inst->setSample(state->sample, smp);

	volEnvSetInst(inst);

	cbvolenvenabled->setChecked(inst->getVolEnvEnabled());

	handleSampleChange(state->sample);
	setHasUnsavedChanges(true);
	redrawSubScreen();
	setRecordMode(state->recording);
}

void handleRecordSampleCancel(void)
{
	// Kill record box
	gui->unregisterOverlayWidget();
	delete recordbox;

	// Turn off the mic
	CommandMicOff();

	redrawSubScreen();
	setRecordMode(state->recording);
}

// OMG FUCKING BEST FEATURE111
void handleRecordSample(void)
{
	// Check RAM first!
	void *testbuf = ntxm_umalloc(RECORDBOX_SOUNDDATA_SIZE * 2);
	if(!testbuf)
	{
		showMessage("not enough ram free!", true);
		return;
	}

	ntxm_free(testbuf);

	// Turn on the mic
	CommandMicOn();

	// Get sample
	Sample *smp = 0;
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst != 0)
		smp = inst->getSample(state->sample);

	// Show record box

	recordbox = new RecordBox(sub_screen, handleRecordSampleOK, handleRecordSampleCancel, smp, inst, state->sample);

	gui->registerOverlayWidget(recordbox, PlatformKey_A | PlatformKey_B, SUB_SCREEN);

	recordbox->reveal();

}

void handleNormalizeOK(void)
{
	u16 percent = normalizeBox->getValue();

	Sample *sample = song->getInstrument(state->instrument)->getSample(state->sample);

	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);
	if(!sel_exists)
	{
		startsample = 0;
		endsample = sample->getNSamples() - 1;
	}

	sample->normalize(percent, startsample, endsample);
	setHasUnsavedChanges(true);
	gui->unregisterOverlayWidget();
	delete normalizeBox;
	redrawSubScreen();
}

void handleNormalizeAuto(void)
{
	Sample *sample = song->getInstrument(state->instrument)->getSample(state->sample);
	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);
	if(!sel_exists)
	{
		startsample = 0;
		endsample = sample->getNSamples() - 1;
	}

	u32 max_amplitude = sample->getMaxAmplitude(startsample, endsample);
	u16 factor = ((sample->getDynamicRange() / 2) << 16) / ((max_amplitude << 16) / 100);
	factor = ntxm_clamp(factor, 100, 2000);
	sample->normalize(factor, startsample, endsample);
	setHasUnsavedChanges(true);
	gui->unregisterOverlayWidget();
	delete normalizeBox;
	redrawSubScreen();
}

void handleNormalizeCancel(void)
{
	gui->unregisterOverlayWidget();
	delete normalizeBox;
	redrawSubScreen();
}

void sample_show_normalize_window(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(!inst) return;
	Sample *smp = inst->getSample(state->sample);
	if(!smp) return;

	normalizeBox = new NormalizeBox(sub_screen, handleNormalizeOK, handleNormalizeAuto, handleNormalizeCancel);
	gui->registerOverlayWidget(normalizeBox, 0, SUB_SCREEN);
	normalizeBox->reveal();
}

#define RIGHT_SIDE_BUTTON_WIDTH 30
#ifdef NT_PLATFORM_NDS
#define RIGHT_SIDE_BUTTON_X(screen) (255 - (RIGHT_SIDE_BUTTON_WIDTH))
#else
#define RIGHT_SIDE_BUTTON_X(screen) ((screen)->getWidth() - 1 - RIGHT_SIDE_BUTTON_WIDTH)
#endif

void swapPatternButtons(Handedness handedness)
{
	u16 x, y;
	pv->getPos(&x, &y, NULL, NULL);

	Handedness current_handedness = x == 30 ? LEFT_HANDED : RIGHT_HANDED;
	if (current_handedness == handedness) return;

	std::vector<Widget*> widgets = gui->getWidgets(MAIN_SCREEN);

	for (Widget* widget : widgets) {
		int offset = (handedness == LEFT_HANDED) ? -RIGHT_SIDE_BUTTON_X(widget->getScreen()) : RIGHT_SIDE_BUTTON_X(widget->getScreen());
		widget->getPos(&x, &y, NULL, NULL);
		widget->setPos(x + offset, y);
	}
	pv->setPos(handedness == LEFT_HANDED ? 30 : 0, 0);

	redraw_main_requested = true;
}

void handleHandednessChange(u8 handedness)
{
	clearMainScreen();

	Handedness h = (handedness == 0) ? LEFT_HANDED : RIGHT_HANDED;
	settings->setHandedness(h);
	PlatformSetInputLayout(h);
	swapPatternButtons(h);
}

void handleOutputModeChange(u8 outputMode)
{
	settings->setStereoOutput(outputMode != 0);
	CommandSetStereoOutput(outputMode != 0);
	stopPlay();
}

#ifdef NT_PLATFORM_NDS
void handleOutputFreqChange(u8 freq)
{
	settings->setFreq47kHz(freq != 0);
	soundExtSetFrequency(freq ? 47 : 32);
}
#endif

void adjustMainScreenWidgets(int width_delta)
{
	u16 x, y;
	std::vector<Widget*> widgets = gui->getWidgets(MAIN_SCREEN);

	pv->getPos(NULL, NULL, &x, &y);
	pv->setSize(x + width_delta, y);

	for (Widget* widget : widgets) {
		widget->getPos(&x, &y, NULL, NULL);
		if (x > 0)
			widget->setPos(x + width_delta, y);
	}
}

void switchScreens(void)
{
#ifndef NT_PLATFORM_NDS
	int old_main_screen_width = main_screen->getWidth();
#endif
	if (!PlatformVideoSwapScreens()) return;
#ifndef NT_PLATFORM_NDS
	int new_main_screen_width = main_screen->getWidth();
	if (old_main_screen_width != new_main_screen_width) {
		adjustMainScreenWidgets(new_main_screen_width - old_main_screen_width);
	}
#endif
	gui->switchScreens();
	if (!fxkb->is_visible())
		pv->clearSelection();
	redraw_main_requested = false;
	drawMainScreen();
#ifndef NT_PLATFORM_NDS
	redrawSubScreen();
#endif
}



// Create the song and do other init stuff yet to be determined.
void setupSong(void) {
	song = new Song();
#ifdef NT_PLATFORM_NDS
	action_buffer = new ActionBuffer(isDSiMode() ? 1024 : 256);
#else
	action_buffer = new ActionBuffer(2048);
#endif
	ntxm_flush_dcache();
}


void deleteMessageBox(void)
{
	gui->unregisterOverlayWidget();

	delete mb;

	mb = 0;
	redrawSubScreen();
}

void requestExit(void)
{
	exit_requested = true;
}

void showExitBox(void)
{
	if (mb != 0) deleteMessageBox();

	mb = new MessageBox(sub_screen, "really exit", 2, "yes", requestExit, "no", deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
	mb->pleaseDraw();
}

void showMessage(const char *msg, bool error)
{
	mb = new MessageBox(sub_screen, msg, 1, error ? "doh!" : "yay!", deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
}


void showAboutBox(void)
{
	char msg[256];
	snprintf(msg, 256, "NitrousTracker " VERSION " (" GIT_HASH ")");
	mb = new MessageBox(sub_screen, msg, 2, "track on!", deleteMessageBox, "exit", showExitBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
}

void ptnCopy(bool cut)
{
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	uiPotSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2, true);

	Cell **ptn = song->getPattern(song->getPotEntry(state->potpos));

	if(clipboard != NULL) delete clipboard;
	clipboard = new CellArray(ptn, sel_x1, sel_y1, sel_x2, sel_y2);
	if (!clipboard->valid())
	{
		delete clipboard;
		clipboard = NULL;
	}

	buttonpaste->set_enabled(clipboard != NULL);

	if(cut == true) {
		action_buffer->add(song, newCellClearAction(state, song, sel_x1, sel_y1, sel_x2, sel_y2));
	}
}

void handleCut(void)
{
	ptnCopy(true);
	redraw_main_requested = true;
}

void handleCopy(void)
{
	ptnCopy(false);
	redraw_main_requested = true;
}

void handlePaste(void)
{
	if(clipboard != NULL) {
		int ptn_n_rows = song->getPatternLength(song->getPotEntry(state->potpos));
		int n_channels = song->getChannels();
		u8 rows_over = std::max((s16)(clipboard->height() + state->getCursorRow()) - ptn_n_rows, 0);
		u8 cols_over = std::max((s16)(clipboard->width() + state->channel) - n_channels, 0);

		if (rows_over > 0 || cols_over > 0) {
			ntxm_dprintf("paste is oversized by %u rows and %u cols, trimming\n", rows_over, cols_over);
			u8 new_height = clipboard->height() - rows_over;
			u8 new_width = clipboard->width() - cols_over;
			CellArray *new_i = new CellArray(new_width, new_height);
			if (new_i) {
				clipboard->paste(new_i, 0, 0);
				action_buffer->add(song, new MultipleCellSetAction(state, state->channel, state->getCursorRow(), new_i, true));
				delete new_i;
			}
		} else {
			action_buffer->add(song, new MultipleCellSetAction(state, state->channel, state->getCursorRow(), clipboard, true));
		}
	}
	redraw_main_requested = true;
}

void handleButtonColumnSelect(void)
{
	// Is there a selection?
	u16 x1, y1, x2, y2;
	if(pv->getSelection(&x1, &y1, &x2, &y2) == true) {
		// Yes: Expand the selection to use the complete rows
		u16 new_y1 = 0;
		u16 new_y2 = song->getPatternLength(song->getPotEntry(state->potpos)) - 1;
		if (new_y1 != y1 || new_y2 != y2) {
			pv->setSelection(x1, new_y1, x2, new_y2);
		} else {
			// Already expanded: Clear selection
			pv->clearSelection();
		}
	} else {
		// No: Select row at cursor
		x1 = x2 = state->channel;
		y1 = 0;
		y2 = song->getPatternLength(song->getPotEntry(state->potpos)) - 1;
		pv->setSelection(x1, y1, x2, y2);
	}
	redraw_main_requested = true;
}

void handleSampleVolumeChange(s32 newvol)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	u8 vol;
	if(newvol>=64) {
		vol = 255;
	} else {
		vol = newvol*4;
	}

	smp->setVolume(vol);
	ntxm_flush_dcache();
	setHasUnsavedChanges(true);
}

void handleSamplePanningChange(s32 newpanning)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	u8 pan = newpanning * 2;

	if (smp->getPanning() != pan) setHasUnsavedChanges(true);
	smp->setPanning(pan);
	smp->setBasePanning();
	ntxm_flush_dcache();
}

void handleSampleRelNoteChange(s32 newnote)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	ntxm_flush_dcache();

	if (smp->getRelNote() != newnote) setHasUnsavedChanges(true);
	smp->setRelNote(newnote);

}

void handleSampleFineTuneChange(s32 newfinetune)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	ntxm_flush_dcache();

	if (smp->getFinetune() != newfinetune) setHasUnsavedChanges(true);
	smp->setFinetune(newfinetune);
}

void handleMuteAll(void)
{
	pv->muteAll();
	redraw_main_requested = true;
}

void handleUnmuteAll(void)
{
	pv->unmuteAll();
	redraw_main_requested = true;
}

void sample_select_all(void)
{
	sampledisplay->select_all();
}

void sample_clear_selection(void)
{
	sampledisplay->clear_selection();
}

void sample_del_selection(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	stopPlay();

	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);
	if(sel_exists==false) return;

	smp->delPart(startsample, endsample);

	ntxm_flush_dcache();

	sampledisplay->setSample(smp);
	handleSampleChange(state->sample);

	setHasUnsavedChanges(true);
}

void sample_crop_selection(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	stopPlay();

	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);

	if(!sel_exists || startsample == endsample) return;

	if (endsample < smp->getNSamples()) smp->delPart(endsample, smp->getNSamples() - 1);
	if (startsample > 0) smp->delPart(0, startsample - 1);

	ntxm_flush_dcache();
	sampledisplay->setSample(smp);
}

void sample_fade_in(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	stopPlay();

	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);
	if(sel_exists==false) return;

	smp->fadeIn(startsample, endsample);

	ntxm_flush_dcache();

	sampledisplay->setSample(smp);
	setHasUnsavedChanges(true);
}

void sample_fade_out(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	stopPlay();

	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);
	if(sel_exists==false) return;

	smp->fadeOut(startsample, endsample);

	ntxm_flush_dcache();

	sampledisplay->setSample(smp);
	setHasUnsavedChanges(true);
}

void sample_reverse(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	stopPlay();

	u32 startsample, endsample;
	bool sel_exists = sampledisplay->getSelection(&startsample, &endsample);
	if(sel_exists==false) {
		startsample = 0;
		endsample = smp->getNSamples();
	}

	smp->reverse(startsample, endsample);

	ntxm_flush_dcache();

	sampledisplay->setSample(smp);
	setHasUnsavedChanges(true);
}

void sampleTabBoxChage(u8 tab)
{
	if( (tab==0) or (tab==1) )
		sampledisplay->setActive();
	else
		sampledisplay->setInactive();

	if(tab != 1) {
		sampledisplay->setDrawMode(false);
		buttonsmpdraw->setState(false);
	}

	if(tab==2)
	{
		Instrument *inst = song->getInstrument(state->instrument);
		if(inst == NULL) {
			sampledisplay->hideLoopPoints();
			return;
		}
		Sample *sample = inst->getSample(state->sample);
		if(sample == NULL) {
			sampledisplay->hideLoopPoints();
			return;
		}
		if(sample->getLoop() == 0)
			sampledisplay->hideLoopPoints();
		else
			sampledisplay->showLoopPoints();
	}
}

#ifdef MIDI

void dsmiConnect(void)
{
	mb = new MessageBox(sub_screen, "connecting ...", 0);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->show();
	mb->pleaseDraw();

	int res = dsmidi_handler.connect();
	deleteMessageBox();

	if(res == 0) {
		showMessage("Sorry, couldn't connect.", true);
	} else {
		debugprintf("YAY, connected!\n");
		btndsmwtoggleconnect->setCaption("disconnect");
        btndsmwtoggleconnect->pleaseDraw();
	}
}

void dsmiDisconnect(void)
{
	dsmidi_handler.disconnect();

	btndsmwtoggleconnect->setCaption("connect");
	btndsmwtoggleconnect->pleaseDraw();
}

void dsmiToggleConnect(void)
{
	if(!dsmidi_handler.dsmi_connected)
		dsmiConnect();
	else
		dsmiDisconnect();
}

void handleDsmiSendToggled(bool is_active)
{
	dsmidi_handler.dsmi_send = is_active;
}

void handleDsmiRecvToggled(bool is_active)
{
	dsmidi_handler.dsmi_recv = is_active;
}

#endif

void saveConfig(void)
{
	if (settings->writeIfChanged())
		showMessage("config saved!", false);
}

void setMultisamplesEnabled(bool show)
{
	if (show)
	{
		if(fxkb->is_visible())
			handleToggleEffectsVisibility(false);

		drawSampleNumbers();
		kb->showKeyLabels();
		if(lbsamples->getY() < (lbinstruments->getY() + lbinstruments_height)) {
		    lbinstruments->resize(lbinstruments->getWidth(), lbinstruments_height - lbsamples_height);
			lbsamples->show();
			buttonrenamesample->show();
		}
	} else
	{
	    if(lbsamples->getY() < (lbinstruments->getY() + lbinstruments_height)) {
			lbsamples->hide();
			lbinstruments->resize(lbinstruments->getWidth(), lbinstruments_height);
			buttonrenamesample->hide();
		}
		kb->hideKeyLabels();
	}

	tbmultisample->setCaption(show ? "-" : "+");
	tbmultisample->setState(show);
}

void handleLerp(void)
{
	if (!fxkb->is_visible()) return;
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	uiPotSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2, false);
	CellArray *fill = new CellArray(sel_x2 - sel_x1 + 1, sel_y2 - sel_y1 + 1);

	Cell start = song->getPattern(song->getPotEntry(state->potpos))[sel_x1][sel_y1];
	Cell end = song->getPattern(song->getPotEntry(state->potpos))[sel_x2][sel_y2];

	if (sel_x1 != sel_x2)
	{
		ntxm_dprintf("select one col only!\n");
		return;
	}

	u16 starteff = start.effect_param;
	u16 endeff = end.effect_param;

	u16 maxeff = std::max(starteff, endeff);
	u16 mineff = std::min(starteff, endeff);

	u16 sel_height = std::max(sel_y1, sel_y2) - std::min(sel_y1, sel_y2);
	int i = 0;
	if (fill != NULL && fill->valid())
	{
		for (u16 row = sel_y1; row <= sel_y2; row++)
		{
			Cell cell = song->getPattern(song->getPotEntry(state->potpos))[sel_x1][row];
			if (start.effect_param != end.effect_param)
			{
				if (starteff < endeff)
				{
					cell.effect_param = mineff + ((maxeff - mineff) * i++) / sel_height;
				}
				else
				{
					cell.effect_param = maxeff - ((maxeff - mineff) * i++) / sel_height;
				}
			}
			*fill->ptr(0, row - sel_y1) = cell;
		}
		action_buffer->add(song, new MultipleCellSetAction(state, sel_x1, sel_y1, fill, false));
		pv->clearSelection();
		redraw_main_requested = true;
	}
}

void handleToggleMapSamples(bool is_active)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == NULL)
		return;

	if(is_active)
	{
		if(tbmultisample->getState() == false) {
			setMultisamplesEnabled(true);
			multisamp_from_mapsamp = true;
		}
	} else {
		if (multisamp_from_mapsamp)
			setMultisamplesEnabled(false);
	}

	state->map_samples = is_active;
	kb->setInMappingMode(is_active);
}

void toggleQueueLock(bool is_active)
{
	if(!is_active) {
		state->queued_potpos = -1;
		lbpot->highlight(state->queued_potpos, false);
	}
}

void addEnvPoint(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst != NULL)
		volenvedit->addPoint();
}

void delEnvPoint(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst != NULL)
		volenvedit->delPoint();
}

void toggleVolEnvEnabled(bool is_enabled)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst != NULL)
		inst->setVolEnvEnabled(is_enabled);
}

void handleMuteChannelsChanged(bool *muted_channels)
{
	for(u8 chn=0; chn < song->getChannels(); ++chn)
	{
		song->setChannelMute(chn, muted_channels[chn]);
	}

	ntxm_flush_dcache();
}

void handleSampleLoopChanged(u8 val)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == 0)
		return;

	Sample *smp = inst->getSample(state->sample);
	if(smp == 0)
		return;

	smp->setLoop(val);

	if(val == NO_LOOP)
		sampledisplay->hideLoopPoints();
	else
		sampledisplay->showLoopPoints();

	ntxm_flush_dcache();
}

void handleSnapTo0XingToggled(bool on)
{
	sampledisplay->setSnapToZeroCrossing(on);
}

void volEnvPointsChanged(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == 0)
		return;

	u16 *xs, *ys;
	u8 n_points = volenvedit->getPoints(&xs, &ys);

	inst->setVolumeEnvelopePoints(xs, ys, n_points);

	toggleVolEnvEnabled(inst->getVolEnvEnabled());
	volenvedit->pleaseDraw();

	ntxm_flush_dcache();
	setHasUnsavedChanges(true);
}

void volEnvDrawFinish(void)
{
	cbvolenvenabled->setChecked(true);
	toggleVolEnvEnabled(true);
	ntxm_flush_dcache();
	setHasUnsavedChanges(true);
}

void envStartDrawMode(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == 0)
		return;

	volenvedit->startDrawMode();
}

void envSetSustainPoint(void)
{
  Instrument *inst = song->getInstrument(state->instrument);
	if(inst == 0)
		return;

	u16 active_point = volenvedit->getActivePoint();

	inst->setVolumeEnvelopeSustainPoint((u8)active_point);

	bool s = inst->getVolumeEnvelopeSustainFlag();
	u8 susp = inst->getVolumeEnvelopeSustainPoint();
	volenvedit->setEditorSustainParams(s, susp);
	volenvedit->pleaseDraw();

	ntxm_flush_dcache();
	setHasUnsavedChanges(true);
}

void envToggleSustainEnabled(bool is_enabled)
{

  Instrument *inst = song->getInstrument(state->instrument);
	if(inst != NULL)
	{
		inst->toggleVolumeEnvelopeSustain(is_enabled);
		volenvedit->toggleSustain(is_enabled);
		volenvedit->pleaseDraw();
	}
	setHasUnsavedChanges(true);
}

void sampleDrawToggle(bool on)
{
	sampledisplay->setDrawMode(on);
}

void setupGUI(bool dldi_enabled)
{
    int piano_width = (sub_screen->getWidth() - 32) & ~0xF;
    int piano_height = 40;
    int piano_y = sub_screen->getHeight() - piano_height;

	gui = new GUI();
	gui->setTheme(settings->getTheme(), settings->getTheme()->col_bg);
	gui->setOnOverlayChanged(handleOverlayWidgetChange);

#ifdef NT_PLATFORM_NDS
	kb = new Piano(0, piano_y, piano_width, piano_height, (u16*)CHAR_BASE_BLOCK_SUB(0), (u16*)SCREEN_BASE_BLOCK_SUB(1/*8*/), sub_screen);
#else
	kb = new Piano(0, piano_y, piano_width, piano_height, NULL, NULL, sub_screen);
#endif
	kb->set_overdraw(false);
	kb->registerNoteCallback(handleNoteStroke);
	kb->registerReleaseCallback(handleNoteRelease);

#ifdef NT_PLATFORM_NDS
	fxkb = new FXKeyboard(0, piano_y, (u16*)CHAR_BASE_BLOCK_SUB(0), (u16*)SCREEN_BASE_BLOCK_SUB(1/*8*/), sub_screen, onFxKeyPressed, false);
#else
	fxkb = new FXKeyboard(0, piano_y, NULL, NULL, sub_screen, onFxKeyPressed, false);
#endif
	fxkb->set_overdraw(false);

	int midbar_x = 98;
	int midbar_width = sub_screen->getWidth() - 20 - midbar_x;
	int midbar_contents_width = 256 - 20 - midbar_x;
	int midbar_gap = (midbar_width - midbar_contents_width) / 5;

	pixmaplogo = new GradientIcon(midbar_x + midbar_gap, 1, 80, 17,
		(const u32*) nitrotracker_logo_raw, sub_screen);
	pixmaplogo->registerPushCallback(showAboutBox);

	int tabbox_endx = (140 * sub_screen->getWidth()) >> 8;
	int tabbox_width = tabbox_endx - 1;
	int tabbox_height = piano_y - 1;
	tabbox = new TabBox(1, 1, tabbox_width, tabbox_height, sub_screen, TABBOX_ORIENTATION_TOP, 16);
	tabbox->setTheme(settings->getTheme(), settings->getTheme()->col_bg);
	// Note that setHasUnsavedChanges depends on this count and order of tabs.
	tabbox->addTab(icon_song_raw, 0);
	if (dldi_enabled)
		tabbox->addTab(icon_disk_raw, 1);
	tabbox->addTab(icon_sample_raw, 2);
	tabbox->addTab(icon_trumpet_raw, 3);
	tabbox->addTab(icon_wrench_raw, 4);

	// <Disk OP GUI>
	{
	    int fileselector_y = 21;
		int fileselector_width = tabbox_width - 39;
		int fileselector_height = tabbox_height - 40;
	    int below_fileselector_y1 = fileselector_y + fileselector_height + 2;
		fileselector = new FileSelector(38, fileselector_y, fileselector_width, fileselector_height, sub_screen);

		std::vector<std::string> samplefilter;
		samplefilter.push_back("wav");

		fileselector->addFilter("sample", samplefilter);

		std::vector<std::string> songfilter;
		songfilter.push_back("mod");
		songfilter.push_back("xm");
		fileselector->addFilter("song", songfilter);
		std::vector<std::string> instfilter;
		instfilter.push_back("xi");
		fileselector->addFilter("instrument", instfilter);
		fileselector->selectFilter("song");
		fileselector->registerFileSelectCallback(handleFileChange);
		fileselector->registerDirChangeCallback(handleDirChange);

		rbgdiskop = new RadioButton::RadioButtonGroup();

		rbsong   = new RadioButton(2, 21, 36, 14, sub_screen, rbgdiskop);

		rbsong->setCaption("sng");

		rbsample = new RadioButton(2, 36, 36, 14, sub_screen, rbgdiskop);

		rbsample->setCaption("smp");

		//rbinst   = new RadioButton(2, 51, 36, 14, sub_screen, "ins", rbgdiskop);
		rbgdiskop->setActive(0);

		rbgdiskop->registerChangeCallback(handleDiskOPChangeFileType);

		memoryiindicator_disk = new MemoryIndicator(3, 51, 34, 8, sub_screen, true);

		labelramusage_disk = new Label(8, 59, 34, 10, sub_screen, false);
		labelramusage_disk->setCaption("ram");

		cbsamplepreview = new CheckBox(4, 70, 34, 14, sub_screen, false, true);
		cbsamplepreview->setCaption("pre");
		cbsamplepreview->registerToggleCallback(handleSamplePreviewToggled);

		buttonload = new Button(3, 86, 34, 14, sub_screen);
		buttonload->setCaption("load");
		buttonload->registerPushCallback(handleLoad);

		buttonsave = new Button(3, 102, 34, 14, sub_screen);
		buttonsave->setCaption("save");
		buttonsave->registerPushCallback(handleSave);

		buttondelfile = new Button(3, 118, 34, 14, sub_screen);
		buttondelfile->setCaption("del");
		buttondelfile->registerPushCallback(handleDelfile);

		labelFilename = new Label(3, below_fileselector_y1, fileselector_width - 3, 14, sub_screen);
		labelFilename->setCaption("");
		labelFilename->registerPushCallback(showTypewriterForFilename);

		buttonchangefilename = new Button(fileselector_width + 1, below_fileselector_y1, 22, 14, sub_screen);
		buttonchangefilename->setCaption("...");
		buttonchangefilename->registerPushCallback(showTypewriterForFilename);

		buttonnewfolder = new BitButton(fileselector_width + 24, below_fileselector_y1, 14, 14, sub_screen, icon_new_folder_raw, 8, 8, 3, 3);
		buttonnewfolder->registerPushCallback(showTypewriterForNewFolder);
	}

	if (dldi_enabled)
	{
		tabbox->registerWidget(fileselector, 0, 1);
		tabbox->registerWidget(rbsong, 0, 1);
		tabbox->registerWidget(rbsample, 0, 1);
		//tabbox->registerWidget(rbinst, 0, 1);
		tabbox->registerWidget(memoryiindicator_disk, 0, 1);
		tabbox->registerWidget(labelramusage_disk, 0, 1);
		tabbox->registerWidget(cbsamplepreview, 0, 1);
		tabbox->registerWidget(buttondelfile, 0, 1);
		tabbox->registerWidget(buttonsave, 0, 1);
		tabbox->registerWidget(buttonload, 0, 1);
		tabbox->registerWidget(labelFilename, 0, 1);
		tabbox->registerWidget(buttonchangefilename, 0, 1);
		tabbox->registerWidget(buttonnewfolder, 0, 1);
	}
	// </Disk OP GUI>

	// <Song gui>
	{
	    int pot_y = 21;
		int pot_height = tabbox_height - 73;
		lbpot = new ListBox(4, pot_y, 50, pot_height, sub_screen, 1, true);
		lbpot->set(0," 0");
		lbpot->registerChangeCallback(handlePotPosChangeFromUser);
		buttonpotup = new Button(70, 47, 14, 12, sub_screen);
		buttonpotup->setCaption(">");
		buttonpotup->registerPushCallback(handlePotInc);
		buttonpotdown = new Button(55, 47, 14, 12, sub_screen);
		buttonpotdown->setCaption("<");
		buttonpotdown->registerPushCallback(handlePotDec);
		buttonins = new Button(55, 21, 29, 12, sub_screen);
		buttonins->setCaption("ins");
		buttonins->registerPushCallback(handlePotIns);
		buttondel = new Button(55, 60, 29, 12, sub_screen);
		buttondel->setCaption("del");
		buttondel->registerPushCallback(handlePotDel);
		buttoncloneptn = new Button(55, 34, 29, 12, sub_screen);
		buttoncloneptn->setCaption("cln");
		buttoncloneptn->registerPushCallback(handlePtnClone);
		tbqueuelock = new ToggleButton(55, 74, 29, 12, sub_screen, true);
		tbqueuelock->setCaption("lock");
		tbqueuelock->registerToggleCallback(toggleQueueLock);
		tbpotloop = new ToggleButton(55, 87, 29, 12, sub_screen, true);
		tbpotloop->setCaption("loop");
		tbpotloop->registerToggleCallback(handleLoopToggle);

		labelptnlen = new Label(87, 48, tabbox_width - 2 - 87, 12, sub_screen, false);
		labelptnlen->setCaption("ptn len:");
		nsptnlen = new NumberSlider(tabbox_width - 2 - 32, 60, 32, 17, sub_screen, DEFAULT_PATTERN_LENGTH, 1, 256, true);
		nsptnlen->registerChangeCallback(handlePtnLengthChange);

		labelchannels = new Label(87, 22, 48, 12, sub_screen, false);
		labelchannels->setCaption("chn:  4");
		buttonlesschannels = new Button(tabbox_width - 2 - 25, 34, 12, 12, sub_screen);
		buttonlesschannels->setCaption("-");
		buttonlesschannels->registerPushCallback(handleChannelDel);
		buttonmorechannels = new Button(tabbox_width - 2 - 12, 34, 12, 12, sub_screen);
		buttonmorechannels->setCaption("+");
		buttonmorechannels->registerPushCallback(handleChannelAdd);

		int below_pot_y1 = pot_y + pot_height + 4;
		int below_pot_y2 = below_pot_y1 + 12;
		int below_pot_y3 = below_pot_y2 + 19;

		labeltempo = new Label(4, below_pot_y1, 32, 12, sub_screen, false);
		labeltempo->setCaption("tmp");
		labelbpm = new Label(38, below_pot_y1, 32, 12, sub_screen, false);
		labelbpm->setCaption("bpm");
		labelrestartpos = new Label(72, below_pot_y1, 46, 12, sub_screen, false);
		labelrestartpos->setCaption("restart");
		nbtempo = new NumberBox(4, below_pot_y2, 32, 17, sub_screen, 1, 1, 31);
		nbtempo->registerChangeCallback(handleTempoChange);
#ifdef DEBUG
		nsbpm = new NumberSlider(38, below_pot_y2, 32, 17, sub_screen, 120, 1, 255);
#else
		nsbpm = new NumberSlider(38, below_pot_y2, 32, 17, sub_screen, 120, 32, 255);
#endif
		nsbpm->registerChangeCallback(handleBpmChange);
		nsrestartpos = new NumberSlider(72, below_pot_y2, 32, 17, sub_screen, 0, 0, 255, true);
		nsrestartpos->registerChangeCallback(handleRestartPosChange);

		labelsongname = new Label(4, below_pot_y3, tabbox_width - 26, 14, sub_screen, true);
		labelsongname->setCaption("unnamed");
		labelsongname->registerPushCallback(showTypewriterForSongRename);

		buttonrenamesong = new Button(tabbox_width - 21, below_pot_y3, 20, 14, sub_screen);
		buttonrenamesong->setCaption("...");
		buttonrenamesong->registerPushCallback(showTypewriterForSongRename);

		buttonzap = new Button(107, below_pot_y2 + 1, 30, 14, sub_screen);
		buttonzap->setCaption("zap!");
		buttonzap->registerPushCallback(handleZap);

		labelramusage = new Label(87, 78, 52, 12, sub_screen, false);
		labelramusage->setCaption("ram use");

		memoryiindicator = new MemoryIndicator(87, 90, tabbox_width - 2 - 87, 8, sub_screen);

		tabbox->registerWidget(lbpot, 0, 0);
		tabbox->registerWidget(buttonpotup, 0, 0);
		tabbox->registerWidget(buttonpotdown, 0, 0);
		tabbox->registerWidget(buttonins, 0, 0);
		tabbox->registerWidget(buttondel, 0, 0);
		tabbox->registerWidget(buttoncloneptn, 0, 0);
		tabbox->registerWidget(tbqueuelock, 0, 0);
		tabbox->registerWidget(tbpotloop, 0, 0);
		tabbox->registerWidget(nsptnlen, 0, 0);
		tabbox->registerWidget(labelptnlen, 0, 0);
		tabbox->registerWidget(labelchannels, 0, 0);
		tabbox->registerWidget(buttonmorechannels, 0, 0);
		tabbox->registerWidget(buttonlesschannels, 0, 0);
		tabbox->registerWidget(labeltempo, 0, 0);
		tabbox->registerWidget(labelbpm, 0, 0);
		tabbox->registerWidget(labelrestartpos, 0, 0);
		tabbox->registerWidget(nbtempo, 0, 0);
		tabbox->registerWidget(nsbpm, 0, 0);
		tabbox->registerWidget(nsrestartpos, 0, 0);
		tabbox->registerWidget(labelsongname, 0, 0);
		tabbox->registerWidget(buttonrenamesong, 0, 0);
		tabbox->registerWidget(buttonzap, 0, 0);
		tabbox->registerWidget(memoryiindicator, 0, 0);
		tabbox->registerWidget(labelramusage, 0, 0);
	}
	// </Song gui>

	// <Sample Gui>
	int sampletabbox_height = 55;
	int sampledisplay_height = tabbox_height - sampletabbox_height - 26;
	int sampletabbox_y = 23 + sampledisplay_height + 1;
	sampledisplay = new SampleDisplay(4, 23, tabbox_width - 6, sampledisplay_height, sub_screen);
	sampledisplay->setActive();

	sampletabbox = new TabBox(3, sampletabbox_y, tabbox_width - 6, sampletabbox_height, sub_screen, TABBOX_ORIENTATION_LEFT, 11);
	sampletabbox->setTheme(settings->getTheme(), settings->getTheme()->col_smp_bg);
	sampletabbox->addTab(sampleedit_wave_icon_raw, 0);
	sampletabbox->addTab(sampleedit_draw_small_raw, 1);
	sampletabbox->addTab(sampleedit_control_icon_raw, 2);
	sampletabbox->addTab(sampleedit_loop_icon_raw, 3);


	//sampletabbox->addTab(sampleedit_chip_icon);
	sampletabbox->registerTabChangeCallback(sampleTabBoxChage);

	// <Sample editing>
	{
		labelsampleedit_record = new Label(18, sampletabbox_y + 5, 21, 30, sub_screen, true);
		labelsampleedit_record->setCaption("rec");

		buttonrecord = new BitButton(20, sampletabbox_y + 16, 17, 17, sub_screen, sampleedit_record_raw);
		buttonrecord->registerPushCallback(handleRecordSample);

		labelsampleedit_select = new Label(38, sampletabbox_y + 5, 39, 30, sub_screen, true);
		labelsampleedit_select->setCaption("select");

		buttonsmpselall = new BitButton(40, sampletabbox_y + 16, 17, 17, sub_screen, sampleedit_all_raw);
		buttonsmpselall->registerPushCallback(sample_select_all);

		buttonsmpselnone = new BitButton(58, sampletabbox_y + 16, 17, 17, sub_screen, sampleedit_none_raw);
		buttonsmpselnone->registerPushCallback(sample_clear_selection);

		labelsampleedit_edit = new Label(76, sampletabbox_y + 5, 57, 48, sub_screen, true);
		labelsampleedit_edit->setCaption("edit");

		buttonsmpfadein = new BitButton(78, sampletabbox_y + 16, 17, 17, sub_screen, sampleedit_fadein_raw);
		buttonsmpfadein->registerPushCallback(sample_fade_in);

		buttonsmpfadeout = new BitButton(96, sampletabbox_y + 16, 17, 17, sub_screen, sampleedit_fadeout_raw);
		buttonsmpfadeout->registerPushCallback(sample_fade_out);

		buttonsmpreverse = new BitButton(78, sampletabbox_y + 34, 17, 17, sub_screen, sampleedit_reverse_raw);
		buttonsmpreverse->registerPushCallback(sample_reverse);

		buttonsmpseldel = new BitButton(96, sampletabbox_y + 34, 17, 17, sub_screen, sampleedit_del_raw);
		buttonsmpseldel->registerPushCallback(sample_del_selection);

		buttonsmptrim = new BitButton(114, sampletabbox_y + 34, 17, 17, sub_screen, sampleedit_trim_raw);
		buttonsmptrim->registerPushCallback(sample_crop_selection);

		buttonsmpnormalize = new BitButton(114, sampletabbox_y + 16, 17, 17, sub_screen, sampleedit_normalize_raw);
		buttonsmpnormalize->registerPushCallback(sample_show_normalize_window);

		sampletabbox->registerWidget(buttonrecord, 0, 0);
		sampletabbox->registerWidget(buttonsmpselall, 0, 0);
		sampletabbox->registerWidget(buttonsmpselnone, 0, 0);
		sampletabbox->registerWidget(buttonsmpseldel, 0, 0);
		sampletabbox->registerWidget(buttonsmptrim, 0, 0);
		sampletabbox->registerWidget(buttonsmpfadein, 0, 0);
		sampletabbox->registerWidget(buttonsmpfadeout, 0, 0);
		sampletabbox->registerWidget(buttonsmpreverse, 0, 0);
		sampletabbox->registerWidget(buttonsmpnormalize, 0, 0);
		sampletabbox->registerWidget(labelsampleedit_edit, 0, 0);
		sampletabbox->registerWidget(labelsampleedit_select, 0, 0);
		sampletabbox->registerWidget(labelsampleedit_record, 0, 0);
	}
	// </Sample editing>

	// <Drawing and Generating>
	{
		buttonsmpdraw = new ToggleButton(18, sampletabbox_y + 2, 17, 17, sub_screen);
		buttonsmpdraw->setBitmap(sampleedit_draw_raw);
		buttonsmpdraw->registerToggleCallback(sampleDrawToggle);

		sampletabbox->registerWidget(buttonsmpdraw, 0, 1);
	}
	// </Drawing and Generating>

	// <Sample settings>
	{
		labelsamplevolume = new Label(22, sampletabbox_y + 14, 25, 10, sub_screen, false);
		labelsamplevolume->setCaption("vol");

		labelpanning = new Label(19, sampletabbox_y + 33, 25, 10, sub_screen, false);
		labelpanning->setCaption("pan");

		labelrelnote = new Label(79, sampletabbox_y + 14, 25, 10, sub_screen, false);
		labelrelnote->setCaption("rel");

		labelfinetune = new Label(75, sampletabbox_y + 33, 30, 10, sub_screen, false);
		labelfinetune->setCaption("tun");

		nssamplevolume = new NumberSlider(40, sampletabbox_y + 9, 32, 17, sub_screen, 64, 0, 64);
		nssamplevolume->registerChangeCallback(handleSampleVolumeChange);

		nspanning = new NumberSlider(40, sampletabbox_y + 28, 32, 17, sub_screen, 64, 0, 127, false);
		nspanning->registerChangeCallback(handleSamplePanningChange);

		nsrelnote = new NumberSliderRelNote(94, sampletabbox_y + 9, 38, 17, sub_screen, 0);
		nsrelnote->registerChangeCallback(handleSampleRelNoteChange);

		nsfinetune = new NumberSlider(94, sampletabbox_y + 28, 38, 17, sub_screen, 0, -128, 127);
		nsfinetune->registerChangeCallback(handleSampleFineTuneChange);

		sampletabbox->registerWidget(nssamplevolume, 0, 2);
		sampletabbox->registerWidget(nspanning, 0, 2);
		sampletabbox->registerWidget(nsrelnote, 0, 2);
		sampletabbox->registerWidget(nsfinetune, 0, 2);
		sampletabbox->registerWidget(labelfinetune, 0, 2);
		sampletabbox->registerWidget(labelrelnote, 0, 2);
		sampletabbox->registerWidget(labelsamplevolume, 0, 2);
		sampletabbox->registerWidget(labelpanning, 0, 2);
	}
	// </Sample settings>

	// <Looping>
	{
		gbsampleloop = new GroupBox(19, sampletabbox_y + 5, 110, 32, sub_screen);
		gbsampleloop->setText("loop type");

		rbg_sampleloop = new RadioButton::RadioButtonGroup();

		rbloop_none     = new RadioButton(21, sampletabbox_y + 16, 40, 10, sub_screen, rbg_sampleloop);
		rbloop_forward  = new RadioButton(68, sampletabbox_y + 16, 40, 10, sub_screen, rbg_sampleloop);
		rbloop_pingpong = new RadioButton(21, sampletabbox_y + 26, 40, 10, sub_screen, rbg_sampleloop);

		rbloop_none->setCaption("none");
		rbloop_forward->setCaption("forward");
		rbloop_pingpong->setCaption("ping-pong");

		rbloop_none->setActive(true);

		rbg_sampleloop->registerChangeCallback(handleSampleLoopChanged);

		cbsnapto0xing = new CheckBox(50, sampletabbox_y + 40, 77, 10, sub_screen, true, true);
		cbsnapto0xing->setCaption("snap");
		cbsnapto0xing->registerToggleCallback(handleSnapTo0XingToggled);

		sampletabbox->registerWidget(rbloop_none, 0, 3);
		sampletabbox->registerWidget(rbloop_forward, 0, 3);
		sampletabbox->registerWidget(rbloop_pingpong, 0, 3);
		sampletabbox->registerWidget(gbsampleloop, 0, 3);
		sampletabbox->registerWidget(cbsnapto0xing, 0, 3);
	}
	// </Looping>

	tabbox->registerWidget(sampledisplay, 0, 2);
	tabbox->registerWidget(sampletabbox, 0, 2);
	// </Sample Gui>

	// <Instruments Gui>
	{
    	int volenvedit_height = tabbox_height - 79;
        int volenvedit_y = 24 + volenvedit_height;

		volenvedit = new EnvelopeEditor(5, 24, tabbox_width - 9, volenvedit_height, sub_screen, MAX_ENV_X, MAX_ENV_Y, MAX_ENV_POINTS);
		volenvedit->registerPointsChangeCallback(volEnvPointsChanged);
		volenvedit->registerDrawFinishCallback(volEnvDrawFinish);

		cbvolenvenabled = new CheckBox(6, volenvedit_y + 1, 60, 10, sub_screen, true, false);
		cbvolenvenabled->setCaption("env on");
		cbvolenvenabled->registerToggleCallback(toggleVolEnvEnabled);

		btnaddenvpoint = new Button(tabbox_width - 4 - 30 - 2 - 30, volenvedit_y + 4, 30, 10, sub_screen);
		btnaddenvpoint->setCaption("add");
		btnaddenvpoint->registerPushCallback(addEnvPoint);

		btndelenvpoint = new Button(tabbox_width - 4 - 30, volenvedit_y + 4, 30, 10, sub_screen);
		btndelenvpoint->setCaption("del");
		btndelenvpoint->registerPushCallback(delEnvPoint);

		btnenvdrawmode = new Button(6, volenvedit_y + 16, 60, 10, sub_screen);
		btnenvdrawmode->setCaption("draw env");
		btnenvdrawmode->registerPushCallback(envStartDrawMode);

        btnenvsetsuspoint = new Button(6, volenvedit_y + 26, 60, 10, sub_screen);
        btnenvsetsuspoint->setCaption("set sus");
        btnenvsetsuspoint->registerPushCallback(envSetSustainPoint);

        cbsusenabled = new CheckBox(6, volenvedit_y + 36, 60, 10, sub_screen, true, false);
        cbsusenabled->setCaption("sus on");
        cbsusenabled->registerToggleCallback(envToggleSustainEnabled);

		tbmapsamples = new ToggleButton(72, volenvedit_y + 37, tabbox_width - 4 - 72, 12, sub_screen);
		tbmapsamples->setCaption("map samp.");
		tbmapsamples->registerToggleCallback(handleToggleMapSamples);
		tbmapsamples->disable();

		tabbox->registerWidget(btnaddenvpoint, 0, 3);
		tabbox->registerWidget(btndelenvpoint, 0, 3);
		tabbox->registerWidget(btnenvdrawmode, 0, 3);
		tabbox->registerWidget(btnenvsetsuspoint, 0, 3);
		tabbox->registerWidget(cbsusenabled, 0, 3);
		tabbox->registerWidget(cbvolenvenabled, 0, 3);
		tabbox->registerWidget(volenvedit, 0, 3);
		tabbox->registerWidget(tbmapsamples, 0, 3);
	}
	// </Instruments Gui>

	// <Settings Gui>
	{
		gbhandedness = new GroupBox(5, 23, 80, 25, sub_screen);
		gbhandedness->setText("handedness");

		rbghandedness = new RadioButton::RadioButtonGroup();
		rblefthanded  = new RadioButton(7 , 35, 35, 14, sub_screen, rbghandedness);
		rblefthanded->setCaption("left");
		rbrighthanded = new RadioButton(42, 35, 35, 14, sub_screen, rbghandedness);
		rbrighthanded->setCaption("right");
		rbghandedness->setActive(1);
		rbghandedness->registerChangeCallback(handleHandednessChange);

#if defined(MIDI) || defined(SHOW_ALL_SETTINGS)
		gbdsmw = new GroupBox(5, 55, 80, 54, sub_screen);
		gbdsmw->setText("dsmidi");

		btndsmwtoggleconnect = new Button(10, 67, 71, 14, sub_screen);
		btndsmwtoggleconnect->setCaption("connect");

		cbdsmwsend = new CheckBox(7, 83, 40, 14, sub_screen, true, true);
		cbdsmwsend->setCaption("send");

		cbdsmwrecv = new CheckBox(7, 97, 40, 14, sub_screen, true, true);
		cbdsmwrecv->setCaption("receive");
		gbtheme = new GroupBox(5, 114, 80, 25, sub_screen);
		bttheme = new Button(10, 125, 71, 14, sub_screen);
#else
		gbtheme = new GroupBox(5, 54, 80, 25, sub_screen);
		bttheme = new Button(10, 64, 71, 14, sub_screen);
#endif
		gbtheme->setText("theme");
		bttheme->registerPushCallback(handleThemeButton);
		bttheme->setCaption("select...");

		gboutput = new GroupBox(89, 23, 40, 34, sub_screen);
		gboutput->setText("out");

		rbgoutput = new RadioButton::RadioButtonGroup();
		rboutputmono = new RadioButton(91, 33, 36, 14, sub_screen, rbgoutput);
		rboutputmono->setCaption("1ch");
		rboutputstereo = new RadioButton(91, 47, 36, 14, sub_screen, rbgoutput);
		rboutputstereo->setCaption("2ch");
		rbgoutput->setActive(1);
		rbgoutput->registerChangeCallback(handleOutputModeChange);

		gblinesbeat = new GroupBox(89, 62, 40, 28, sub_screen);
		gblinesbeat->setText("l/b");
		nblinesbeat = new NumberBox(93, 72, 32, 17, sub_screen, settings->getLinesPerBeat(), 1, 64);
		nblinesbeat->registerChangeCallback(handleLinesBeatChange);

#ifdef NT_PLATFORM_NDS
#if !defined(SHOW_ALL_SETTINGS)
		if (isDSiMode())
#endif
		{
			gbfreq = new GroupBox(89, 95, 40, 34, sub_screen);
			gbfreq->setText("freq");

			rbgfreq = new RadioButton::RadioButtonGroup();
			rbfreq32 = new RadioButton(91, 105, 36, 14, sub_screen, rbgfreq);
			rbfreq32->setCaption("32k");
			rbfreq47 = new RadioButton(91, 119, 36, 14, sub_screen, rbgfreq);
			rbfreq47->setCaption("47k");
			rbgfreq->setActive(1);
			rbgfreq->registerChangeCallback(handleOutputFreqChange);
		}
#endif

		btnconfigsave = new Button(tabbox_width - 1 - 40, tabbox_height - 1 - 14, 40, 14, sub_screen);
		btnconfigsave->setCaption("save");
		btnconfigsave->registerPushCallback(saveConfig);

#ifdef MIDI
		btndsmwtoggleconnect->registerPushCallback(dsmiToggleConnect);
		cbdsmwsend->registerToggleCallback(handleDsmiSendToggled);
		cbdsmwrecv->registerToggleCallback(handleDsmiRecvToggled);
#endif
		tabbox->registerWidget(rblefthanded, 0, 4);
		tabbox->registerWidget(rbrighthanded, 0, 4);
#if defined(MIDI) || defined(SHOW_ALL_SETTINGS)
		tabbox->registerWidget(cbdsmwsend, 0, 4);
		tabbox->registerWidget(cbdsmwrecv, 0, 4);
		tabbox->registerWidget(btndsmwtoggleconnect, 0, 4);
		tabbox->registerWidget(gbdsmw, 0, 4);
#endif
		tabbox->registerWidget(btnconfigsave, 0, 4);
		tabbox->registerWidget(gbhandedness, 0, 4);
		if (dldi_enabled)
			tabbox->registerWidget(bttheme, 0, 4);

		if (dldi_enabled)
			tabbox->registerWidget(gbtheme, 0, 4);
		tabbox->registerWidget(rboutputmono, 0, 4);
		tabbox->registerWidget(rboutputstereo, 0, 4);
		tabbox->registerWidget(gboutput, 0, 4);
		tabbox->registerWidget(nblinesbeat, 0, 4);
		tabbox->registerWidget(gblinesbeat, 0, 4);
#ifdef NT_PLATFORM_NDS
#if !defined(SHOW_ALL_SETTINGS)
		if (isDSiMode())
#endif
		{
			tabbox->registerWidget(rbfreq32, 0, 4);
			tabbox->registerWidget(rbfreq47, 0, 4);
			tabbox->registerWidget(gbfreq, 0, 4);
		}
#endif
    }
	// </Settings Gui>

	int lbinstruments_y = 32;
	lbinstruments_height = sub_screen->getHeight() - 103;
	lbsamples_height = 23;
	int lbsamples_y = 0;
	bool lbsamples_visible = false;
	if (lbinstruments_height > 112) {
	    lbsamples_height = lbinstruments_height - 90;
	    lbinstruments_height = 90;
		lbsamples_visible = true;
	} else {
	    lbsamples_y -= lbsamples_height;
    }
	lbsamples_y += lbinstruments_y + lbinstruments_height;

	lbinstruments = new ListBox(tabbox_endx + 1, lbinstruments_y, sub_screen->getWidth() - tabbox_endx - 2, lbinstruments_height,
	    sub_screen, MAX_INSTRUMENTS, true, true, false);
	lbsamples = new ListBox(tabbox_endx + 1, lbsamples_y, sub_screen->getWidth() - tabbox_endx - 2, lbsamples_height, sub_screen, MAX_INSTRUMENT_SAMPLES, true, lbsamples_visible, true);

	buttonswitchsub    = new BitButton(sub_screen->getWidth() - 20, 1  , 19, 19, sub_screen, icon_flp_raw, 15, 15);
	buttonplay         = new BitButton(midbar_x + 82 + 4*midbar_gap, 3  , 23, 15, sub_screen, icon_play_raw, 12, 12, 5, 0, true);
	buttonpause        = new BitButton(midbar_x + 82 + 4*midbar_gap, 3  , 23, 15, sub_screen, icon_pause_raw, 12, 12, 5, 0, false);
	buttonstop         = new BitButton(midbar_x + 106 + 4*midbar_gap, 3  , 23, 15, sub_screen, icon_stop_raw, 12, 12, 5, 0);

	int button2_main_y = main_screen->getHeight() - (13 * 8);
	int button2_sub_y = sub_screen->getHeight() - (13 * 5);

	buttonundo         = new BitButton(RIGHT_SIDE_BUTTON_X(sub_screen), button2_sub_y, 14, 12, sub_screen, icon_undo_raw, 8, 8, 3, 2);
	buttonredo         = new BitButton(RIGHT_SIDE_BUTTON_X(sub_screen) + RIGHT_SIDE_BUTTON_WIDTH - 14, button2_sub_y, 14, 12, sub_screen, icon_redo_raw, 8, 8, 3, 2);
	buttoninsnote2     = new Button(RIGHT_SIDE_BUTTON_X(sub_screen), button2_sub_y + 13, RIGHT_SIDE_BUTTON_WIDTH, 12, sub_screen);
	buttondelnote2     = new Button(RIGHT_SIDE_BUTTON_X(sub_screen), button2_sub_y + 26, RIGHT_SIDE_BUTTON_WIDTH, 12, sub_screen);
	buttonlerpfx       = new Button(RIGHT_SIDE_BUTTON_X(sub_screen), button2_sub_y + 26, RIGHT_SIDE_BUTTON_WIDTH, 12, sub_screen, false);
	buttonemptynote    = new Button(RIGHT_SIDE_BUTTON_X(sub_screen), button2_sub_y + 39, RIGHT_SIDE_BUTTON_WIDTH, 12, sub_screen);
	buttonemptyfx      = new Button(RIGHT_SIDE_BUTTON_X(sub_screen), button2_sub_y + 39, RIGHT_SIDE_BUTTON_WIDTH, 12, sub_screen, false);
	buttonstopnote     = new Button(RIGHT_SIDE_BUTTON_X(sub_screen), button2_sub_y + 52, RIGHT_SIDE_BUTTON_WIDTH, 12, sub_screen);
	buttoncpprm        = new Button(RIGHT_SIDE_BUTTON_X(sub_screen), button2_sub_y + 52, RIGHT_SIDE_BUTTON_WIDTH, 12, sub_screen, false);
	buttonrenamesample = new Button(tabbox_endx + 1, lbsamples_y + lbsamples_height + 1, 23, 12, sub_screen, lbsamples_visible);
	buttonrenameinst   = new Button(tabbox_endx + 1, 19 , 23, 12, sub_screen);

	tbmultisample      = new ToggleButton(tabbox_endx + 25, 20, 10, 10, sub_screen);

	buttonundo->registerPushCallback(undoOp);
	buttonredo->registerPushCallback(redoOp);

	cbscrolllock = new CheckBox(tabbox_endx + 39, 18, 30, 12, sub_screen, true, false, true);
	cbscrolllock->setCaption("scr lock");
	cbscrolllock->registerToggleCallback(handleToggleScrollLock);

	int add_oct_label_y = piano_y - 27;
	int add_oct_number_y = piano_y - 18;
	int tb_effect_y = piano_y - 17;

	tbrecord = new ToggleButton(tabbox_endx + 1, add_oct_number_y + 1, 16, 16, sub_screen, true, true);
	tbrecord->setBitmap(icon_record_raw, 12, 12);
	tbrecord->registerToggleCallback(setRecordMode);

	labeladd = new Label(tabbox_endx + 42, add_oct_label_y, 22, 12, sub_screen, false, true);
	labeladd->setCaption("add");
	labeloct = new Label(tabbox_endx + 66, add_oct_label_y, 25, 12, sub_screen, false, true);
	labeloct->setCaption("oct");
	labelfxcat = new Label(tabbox_endx + 66, add_oct_label_y, 25, 12, sub_screen, false, true);
	labelfxcat->setCaption("cat");
	labelfxop 		   = new Label(RIGHT_SIDE_BUTTON_X(sub_screen), sub_screen->getHeight() - 51, RIGHT_SIDE_BUTTON_WIDTH, 12, sub_screen, false, true, true);
	labelfxop->setCaption("fx op");
	numberboxfxcat = new NumberBox(tabbox_endx + 66, add_oct_number_y, 18, 17, sub_screen, 0, 0, 3, 1);
	numberboxadd    = new NumberBox(tabbox_endx + 38, add_oct_number_y, 25, 17, sub_screen, state->add, 0, 32, 2, true);
	numberboxoctave = new NumberBox(tabbox_endx + 66, add_oct_number_y, 18, 17, sub_screen, state->basenote/12, 0, 6, 1);
	labeleffectpar = new Label(185, piano_y, 38, 10, sub_screen, false, true, true);
	labeleffectpar->set_overdraw(false);
	labeleffectpar->setCaption("param");
	dbeffectpar	 = new DigitBox(185, piano_y + 11, 35, 17, sub_screen, 0, 0, 255, 2);
	dbeffectpar->set_overdraw(false);
	dbeffectpar->registerChangeCallback(handleEffectParamChanged);
	buttonseteffectpar = new Button(185, piano_y + 27, 35, 10, sub_screen);
	buttonseteffectpar->setCaption("set");
	buttonseteffectpar->set_overdraw(false);
	buttonseteffectpar->registerPushCallback(handleSetEffectParam);
	buttonswitchsub->registerPushCallback(switchScreens);
	buttonplay->registerPushCallback(startPlay);
	buttonstop->registerPushCallback(stopPlay);
	buttonpause->registerPushCallback(pausePlay);

	buttoninsnote2->registerPushCallback(insNote);
	buttondelnote2->registerPushCallback(delNote);
	buttonemptynote->registerPushCallback(emptyNoteStroke);
	buttonstopnote->registerPushCallback(stopNoteStroke);
	buttonlerpfx->registerPushCallback(handleLerp);
	buttonemptyfx->registerPushCallback(handleClearFx);
	buttoncpprm->registerPushCallback(copyFxParam);
	buttonrenameinst->registerPushCallback(showTypewriterForInstRename);
	buttonrenamesample->registerPushCallback(showTypewriterForSampleRename);

	tbmultisample->registerToggleCallback(handleToggleMultiSample);

	numberboxadd->registerChangeCallback(changeAdd);
	numberboxoctave->registerChangeCallback(changeOctave);
	numberboxfxcat->registerChangeCallback(handleEffectsCategoryChange);

	lbinstruments->registerChangeCallback(handleInstChangeReset);
	lbsamples->registerChangeCallback(handleSampleChange);

	buttoninsnote2->setCaption("ins");
	buttondelnote2->setCaption("del");
	buttonemptynote->setCaption("clr");
	buttonstopnote->setCaption("--");
	buttonlerpfx->setCaption("lerp");
	buttonemptyfx->setCaption("clr");
	buttoncpprm->setCaption("get");
	buttonrenameinst->setCaption("ren");
	buttonrenamesample->setCaption("ren");

	tbmultisample->setCaption("+");

	// <Main Screen>
		buttonswitchmain = new BitButton(main_screen->getWidth() - 20, 1 , 19, 19, main_screen, icon_flp_raw, 15, 15);
		buttonswitchmain->registerPushCallback(switchScreens);

		buttonunmuteall = new Button(RIGHT_SIDE_BUTTON_X(main_screen), 22, RIGHT_SIDE_BUTTON_WIDTH, 12, main_screen);
		buttonunmuteall->setCaption("-m/s");

		labelnotevol = new Label(RIGHT_SIDE_BUTTON_X(main_screen) + 5, 34, RIGHT_SIDE_BUTTON_WIDTH - 7, 9, main_screen, false, true, true);
		labelnotevol->setCaption("vol");

		nsnotevolume	 = new NumberSlider(RIGHT_SIDE_BUTTON_X(main_screen), 45, RIGHT_SIDE_BUTTON_WIDTH, 17, main_screen, 127, 0, 127, true, true);
		nsnotevolume->registerPostChangeCallback(handleNoteVolumeChanged);

		buttonsetnotevol = new Button(RIGHT_SIDE_BUTTON_X(main_screen), 61, RIGHT_SIDE_BUTTON_WIDTH, 12, main_screen);
		buttonsetnotevol->setCaption("set");
		buttonsetnotevol->registerPushCallback(handleSetNoteVol);

		/* labeltranspose = new Label(200, 1, 48, 12, main_screen, false, true);
		labeltranspose->setCaption("trps"); */
		buttontransposedown = new Button(RIGHT_SIDE_BUTTON_X(main_screen), 74, 14, 12, main_screen);
		buttontransposedown->setCaption("-");
		buttontransposedown->registerPushCallback(handleTransposeDown);
		buttontransposeup = new Button(RIGHT_SIDE_BUTTON_X(main_screen) + RIGHT_SIDE_BUTTON_WIDTH - 14, 74, 14, 12, main_screen);
		buttontransposeup->setCaption("+");
		buttontransposeup->registerPushCallback(handleTransposeUp);

		tbeffects = new ToggleButton(tabbox_endx + 18, add_oct_number_y + 1, 16, 16, sub_screen);
		tbeffects->setBitmap(icon_fx_raw, 12, 12);
		tbeffects->registerToggleCallback(handleToggleEffectsVisibility);

		//buttoncut         = new BitButton(232,  52, 22, 21, main_screen, icon_cut_raw, 16, 16, 3, 2);
		//buttoncopy        = new BitButton(232,  74, 22, 21, main_screen, icon_copy_raw, 16, 16, 3, 3);
		//buttonpaste       = new BitButton(232,  96, 22, 21, main_screen, icon_paste_raw, 16, 16, 3, 3);

		buttoncut         = new Button(RIGHT_SIDE_BUTTON_X(main_screen), button2_main_y, RIGHT_SIDE_BUTTON_WIDTH, 12, main_screen);
		buttoncopy        = new Button(RIGHT_SIDE_BUTTON_X(main_screen), button2_main_y + 13, RIGHT_SIDE_BUTTON_WIDTH, 12, main_screen);
		buttonpaste       = new Button(RIGHT_SIDE_BUTTON_X(main_screen), button2_main_y + 26, RIGHT_SIDE_BUTTON_WIDTH, 12, main_screen);

		buttoncut->setCaption("cut");
		buttoncopy->setCaption("cp");
		buttonpaste->setCaption("pst");
		buttonpaste->disable();

		buttoncolselect   = new Button(RIGHT_SIDE_BUTTON_X(main_screen), button2_main_y + 39, RIGHT_SIDE_BUTTON_WIDTH, 12, main_screen);
		buttoninsnote     = new Button(RIGHT_SIDE_BUTTON_X(main_screen), button2_main_y + 52, RIGHT_SIDE_BUTTON_WIDTH, 12, main_screen);
		buttondelnote     = new Button(RIGHT_SIDE_BUTTON_X(main_screen), button2_main_y + 65, RIGHT_SIDE_BUTTON_WIDTH, 12, main_screen);
		buttonemptynote2  = new Button(RIGHT_SIDE_BUTTON_X(main_screen), button2_main_y + 78, RIGHT_SIDE_BUTTON_WIDTH, 12, main_screen);
		buttonstopnote2   = new Button(RIGHT_SIDE_BUTTON_X(main_screen), button2_main_y + 91, RIGHT_SIDE_BUTTON_WIDTH, 12, main_screen);

		buttonunmuteall->registerPushCallback(handleUnmuteAll);
		buttoncut->registerPushCallback(handleCut);
		buttoncopy->registerPushCallback(handleCopy);
		buttonpaste->registerPushCallback(handlePaste);
		buttoncolselect->registerPushCallback(handleButtonColumnSelect);
		buttoninsnote->registerPushCallback(insNote);
		buttondelnote->registerPushCallback(delNote);
		buttonemptynote2->registerPushCallback(emptyNoteStroke);
		buttonstopnote2->registerPushCallback(stopNoteStroke);

		buttonstopnote2->setCaption("--");
		buttoncolselect->setCaption("sel");
		buttoninsnote->setCaption("ins");
		buttondelnote->setCaption("del");
		buttonemptynote2->setCaption("clr");

		pv = new PatternView(0, 0, RIGHT_SIDE_BUTTON_X(main_screen), main_screen->getHeight(), main_screen, state);
		pv->setSong(song);
		pv->registerMuteCallback(handleMuteChannelsChanged);

		gui->registerWidget(buttonunmuteall, 0, MAIN_SCREEN);
		gui->registerWidget(buttonswitchmain, 0, MAIN_SCREEN);
		gui->registerWidget(labelnotevol, 0, MAIN_SCREEN);
		gui->registerWidget(nsnotevolume, 0, MAIN_SCREEN);
		gui->registerWidget(buttonsetnotevol, 0, MAIN_SCREEN);
		/* gui->registerWidget(labeltranspose, 0, MAIN_SCREEN); */
		gui->registerWidget(buttontransposedown, 0, MAIN_SCREEN);
		gui->registerWidget(buttontransposeup, 0, MAIN_SCREEN);
		gui->registerWidget(buttoncut, 0, MAIN_SCREEN);
		gui->registerWidget(buttoncopy, 0, MAIN_SCREEN);
		gui->registerWidget(buttonpaste, 0, MAIN_SCREEN);
		gui->registerWidget(buttoncolselect, 0, MAIN_SCREEN);
		gui->registerWidget(buttoninsnote, 0, MAIN_SCREEN);
		gui->registerWidget(buttondelnote, 0, MAIN_SCREEN);
		gui->registerWidget(buttonemptynote2, 0, MAIN_SCREEN);
		gui->registerWidget(buttonstopnote2, 0, MAIN_SCREEN);
		gui->registerWidget(pv, 0, MAIN_SCREEN);
	// </Main Screen>

	gui->registerWidget(cbscrolllock, 0, SUB_SCREEN);
	gui->registerWidget(buttonswitchsub, 0, SUB_SCREEN);
	gui->registerWidget(buttonplay, 0, SUB_SCREEN);
	gui->registerWidget(buttonstop, 0, SUB_SCREEN);
	gui->registerWidget(buttonpause, 0, SUB_SCREEN);
	gui->registerWidget(buttonemptynote, 0, SUB_SCREEN);
	gui->registerWidget(buttonundo, 0, SUB_SCREEN);
	gui->registerWidget(buttonredo, 0, SUB_SCREEN);
	gui->registerWidget(buttoninsnote2, 0, SUB_SCREEN);
	gui->registerWidget(buttondelnote2, 0, SUB_SCREEN);
	gui->registerWidget(buttonlerpfx, 0, SUB_SCREEN);
	gui->registerWidget(buttonemptyfx, 0, SUB_SCREEN);
	gui->registerWidget(buttoncpprm, 0, SUB_SCREEN);
	gui->registerWidget(buttonrenameinst, 0, SUB_SCREEN);
	gui->registerWidget(buttonrenamesample, 0, SUB_SCREEN);
	gui->registerWidget(tbmultisample, 0, SUB_SCREEN);
	gui->registerWidget(numberboxadd, 0, SUB_SCREEN);
	gui->registerWidget(numberboxoctave, 0, SUB_SCREEN);
	gui->registerWidget(numberboxfxcat, 0, SUB_SCREEN);
	gui->registerWidget(dbeffectpar, 0, SUB_SCREEN);
	gui->registerWidget(buttonseteffectpar, 0, SUB_SCREEN);
	gui->registerWidget(labelfxop, 0, SUB_SCREEN);
	gui->registerWidget(labelfxcat, 0, SUB_SCREEN);
	gui->registerWidget(labeleffectpar, 0, SUB_SCREEN);
	gui->registerWidget(labeladd, 0, SUB_SCREEN);
	gui->registerWidget(labeloct, 0, SUB_SCREEN);
	gui->registerWidget(kb, 0, SUB_SCREEN);
	gui->registerWidget(fxkb, 0, SUB_SCREEN);
	gui->registerWidget(buttonstopnote, 0, SUB_SCREEN);
	gui->registerWidget(tbrecord, 0, SUB_SCREEN);
	gui->registerWidget(tbeffects, 0, SUB_SCREEN);
	gui->registerWidget(pixmaplogo, 0, SUB_SCREEN);
	gui->registerWidget(tabbox, 0, SUB_SCREEN);
	gui->registerWidget(lbinstruments, 0, SUB_SCREEN);
	gui->registerWidget(lbsamples, 0, SUB_SCREEN);

	gui->revealAll();
	handleSampleChange(0); // disable samp ed buttons at first as we have no sample!
	actionBufferChangeCallback();
	updateTempoAndBpm();
	handleLinesBeatChange(settings->getLinesPerBeat());
	setHasUnsavedChanges(false);
	handleToggleEffectsVisibility(false);

	gui->drawSubScreen(); // GUI
	drawMainScreen(); // Pattern view. The function also flips buffers
}

void move_to_bottom(void)
{
	state->setCursorRow(song->getPatternLength(song->getPotEntry(state->potpos))-1);
	pv->updateSelection();
	redraw_main_requested = true;
}

void move_to_top(void)
{
	state->setCursorRow(0);
	pv->updateSelection();
	redraw_main_requested = true;
}

void updateSampleOffsetGuide(void)
{
	sampledisplay->setOffsetGuide(0);

	if (fxkb->is_visible()) {
		Cell targetcell = song->getPattern(song->getPotEntry(state->potpos))[state->channel][state->getCursorRow()];

		u8 fx = targetcell.effect;
		u8 prm = targetcell.effect_param;

		if (targetcell.instrument == state->instrument)
		{
			Instrument *inst = song->getInstrument(state->instrument);

			if (inst != NULL && fx == EFFECT_SAMPLE_OFFSET
				&& inst->getNoteSample(targetcell.note) == state->sample) {
				sampledisplay->setOffsetGuide(FT_OFFSET_SCALAR * prm);
			}
		}
	}

	sampledisplay->pleaseDraw();
}


// Update the state for certain keypresses
void handleButtons(u16 buttons, u16 buttonsheld)
{
	u16 ptnlen = song->getPatternLength(song->getPotEntry(state->potpos));

	if(!(buttonsheld & PlatformKey_R))
	{
		if(buttons & PlatformKey_UP)
		{
			int newrow = state->getCursorRow();

			if(fastscroll == false) {
				newrow--;
			} else
			{
				newrow -= 4;
			}
			while(newrow < 0)
				newrow += ptnlen;

			state->setCursorRow(newrow);

			pv->updateSelection();
			updateSampleOffsetGuide();
			redraw_main_requested = true;

		}
		else if(buttons & PlatformKey_DOWN)
		{
			int newrow = state->getCursorRow();

			if(fastscroll == false){
				newrow++;
			} else {
				newrow += 4;
			}

			newrow %= ptnlen;

			state->setCursorRow(newrow);

			pv->updateSelection();
			updateSampleOffsetGuide();
			redraw_main_requested = true;
		}
	}

	if((buttons & PlatformKey_LEFT)&&(!typewriter_active))
	{
		if(state->channel>0) {
			state->channel--;
			pv->updateSelection();
			updateSampleOffsetGuide();
			redraw_main_requested = true;
		}
	}
	else if((buttons & PlatformKey_RIGHT)&&(!typewriter_active))
	{
		if(state->channel < song->getChannels()-1)
		{
			state->channel++;
			pv->updateSelection();
			updateSampleOffsetGuide();
			redraw_main_requested = true;
		}
	}
	else if(buttons & PlatformKey_START)
	{
#ifdef DEBUG
		debugprintf("\x1b[2J");
#else
		if( (state->playing == false) || (state->pause == true) )
			startPlay();
		else
			pausePlay();
#endif
	}
	else if(buttons & PlatformKey_SELECT)
	{
#ifdef DEBUG
		PrintFreeMem();
		printMallInfo();
#else
		stopPlay();
#endif
	}
#ifdef DEBUG
	/*else if(buttons & PlatformKey_Y) {
		saveScreenshot();
	} else if(buttons & PlatformKey_R) {
		dumpSample();
	}
	*/
#endif
}

void VblankHandler(void)
{
	PlatformInputUpdate();

	if(PlatformKeysDown & PlatformKey_TOUCH)
	{
		gui->penDown(PlatformTouchX, PlatformTouchY);
		redraw_main_requested = true;
	}

	if(PlatformKeysUp & PlatformKey_TOUCH)
	{
		gui->penUp(PlatformTouchX, PlatformTouchY);
		lastx = -255;
		lasty = -255;
	}

	if( (PlatformKeysHeld & PlatformKey_TOUCH) && ( (abs(PlatformTouchX - lastx)>0) || (abs(PlatformTouchY - lasty)>0) ) ) // PenMove
	{
		gui->penMove(PlatformTouchX, PlatformTouchY);
		lastx = PlatformTouchX;
		lasty = PlatformTouchY;
		if(gui->getActiveScreen() == MAIN_SCREEN)
			redraw_main_requested = true;
	}

	if(PlatformKeysHeld & PlatformKey_R)
	{
		if(PlatformKeysHeld & PlatformKey_DOWN)
			move_to_bottom();
		else if(PlatformKeysHeld & PlatformKey_UP)
			move_to_top();
	}

	if(PlatformKeysDown & ~PlatformKey_TOUCH)
	{
		if((PlatformKeysDown & PlatformKey_X)||(PlatformKeysDown & PlatformKey_L)) {
			switchScreens();
		}

		if(PlatformKeysDown & PlatformKey_B) {
			fastscroll = true;
		}

		gui->buttonPress(PlatformKeysDown);
		handleButtons(PlatformKeysDown, PlatformKeysHeld);
		pv->pleaseDraw();
	}

	if(PlatformKeysUp)
	{
		gui->buttonRelease(PlatformKeysUp);

		if(PlatformKeysUp & PlatformKey_B)
			fastscroll = false;
	}

#ifdef ENABLE_PIANO_PAK
	if (pianoIsInserted())
	{
		pianoScanKeys();

		u16 piano_down = pianoKeysDown();
		u16 piano_up = pianoKeysUp();

		for (u16 i = 0; i < 15; i++) {
			if (i > 10 && i < 13)
				continue;

			u16 note_val = (i >= 13) ? (i - 2) : i;

			if (piano_down & (1 << i)) {
				handlePianoPakStroke(note_val);
			}
			if (piano_up & (1 << i)) {
				handlePianoPakRelease(note_val);
			}
 		}
	}
#endif

#ifdef NT_PLATFORM_NDS
	oamUpdate(&oamSub);
#endif

	// Constantly update pattern view while playing
	if(redraw_main_requested)
	{
		redraw_main_requested = false;
		drawMainScreen();
	}

	frame = (frame + 1) % 2;
}

void applySettings(void)
{
	bool handedness = settings->getHandedness();
	rbghandedness->setActive(handedness == LEFT_HANDED ? 0 : 1);
	rbgoutput->setActive(settings->getStereoOutput() ? 1 : 0);

	bool samplepreview = settings->getSamplePreview();
	cbsamplepreview->setChecked(samplepreview);

	if(rbsong->getActive() == true)
	{
		fileselector->setDir(settings->getSongPath());
	}
	else if(rbsample->getActive() == true)
	{
		fileselector->setDir(settings->getSamplePath());
	}

	fileselector->pleaseDraw();
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
#ifdef GURU
	defaultExceptionHandler();
#endif

	if (!PlatformInit()) exit(1);
	bool fat_success = PlatformInitFilesystem();

#if defined(NT_PLATFORM_NDS) || defined(NT_PLATFORM_3DS)
	// parse argv[0], if present
	if (argc >= 1 && argv != NULL && argv[0] != NULL) {
		char *path_split = strrchr(argv[0], '/');
		if (path_split != NULL && (path_split - argv[0]) >= 1) {
			int launch_path_len = path_split - argv[0];
			launch_path = (char*) ntxm_cmalloc(launch_path_len + 1);
			strncpy(launch_path, argv[0], launch_path_len);
			launch_path[launch_path_len] = '\0';

			if (!dirExists(launch_path)) {
				ntxm_free(launch_path);
				launch_path = NULL;
			}
		}
	}
#else
	launch_path = (char*) ntxm_cmalloc(4097);
	launch_path[0] = 0;
	getcwd(launch_path, 4096);
	launch_path[4096] = 0;
#endif

	settings = new Settings(launch_path, fat_success);

	last_themepath[SETTINGS_FILENAME_LEN] = '\0';
	state = new State();

	clearMainScreen();
	clearSubScreen();

	// Key repeat handling: enable repeat
	PlatformInputSetRepeat(REPEAT_START_DELAY, 60 / REPEAT_FREQ);

	// Init interprocessor communication
	if (!CommandInit()) {
	    // TODO: Error message
	    PlatformExit();
		return 1;
	}
	RegisterRowCallback(handleRowChangeFromSong);
	RegisterStopCallback(handleStop);
	RegisterPlaySampleFinishedCallback(handlePreviewSampleFinished);
	RegisterPotPosChangeCallback(handlePotPosChangeFromSong);

	setupSong();

	CommandSetSong(song);

	setupGUI(fat_success);
	action_buffer->register_change_callback({&actionBufferChangeCallback});

	applySettings();
	setSong(song);

#ifndef DEBUG
	PlatformVideoFadeIn();
#endif

#ifdef NT_PLATFORM_NDS
	if(!fat_success)
		showMessage("dldi init failed", true);
#endif

#ifdef DEBUG
	debugprintf("NitroTracker debug build.\nBuilt %s %s\n<Start> clears messages.\n", __DATE__, __TIME__);
#endif

	while(!exit_requested)
	{
		VblankHandler();

		dsmidi_handler.tick();

#ifdef DEBUG
        if(PlatformKeysHeld == (KEY_START | KEY_SELECT | KEY_L | KEY_R)) {
            exit_requested = true;
			break;
        }
#endif

		exit_requested |= !PlatformWaitVBlank();
	}

	if (launch_path) ntxm_free(launch_path);

	CommandExit();
	PlatformExit();

	return 0;
}
