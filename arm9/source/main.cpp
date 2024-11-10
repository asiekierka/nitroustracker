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
#define GURU // Show guru meditations
#define USE_FAT
// #define ENABLE_EFFECT_MENU

#include <nds.h>
#include <nds/arm9/console.h>
#include <nds/arm9/sound.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <tobkit/tobkit.h>

// Special tracker widgets
#include "tobkit/recordbox.h"
#include "tobkit/numbersliderrelnote.h"
#include "tobkit/envelope_editor.h"
#include "tobkit/sampledisplay.h"
#include "tobkit/patternview.h"
#include "tobkit/normalizebox.h"

#include <ntxm/fifocommand.h>
#include <ntxm/song.h>
#include <ntxm/xm_transport.h>
#include <ntxm/wav.h>
#include <ntxm/instrument.h>
#include <ntxm/sample.h>
#include <ntxm/ntxmtools.h>
#include "state.h"
#include "settings.h"
#include "tools.h"

#include "icon_disk_raw.h"
#include "icon_song_raw.h"
#include "icon_sample_raw.h"
#include "icon_wrench_raw.h"
#include "icon_trumpet_raw.h"

#include "icon_flp_raw.h"
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
#include "sampleedit_reverse_raw.h"
#include "sampleedit_record_raw.h"
#include "sampleedit_normalize_raw.h"
#include "sampleedit_draw_raw.h"
#include "sampleedit_draw_small_raw.h"

#include "cell_array.h"
#include "action.h"

#include <fat.h>
#ifdef MIDI
#include <libdsmi.h>
#include <dswifi9.h>
#endif

#define REPEAT_FREQ	10 /* Hz */
#define REPEAT_START_DELAY 15 /* frames */

#define FRONT_BUFFER	0
#define BACK_BUFFER	1

#define FILETYPE_SONG	0
#define FILETYPE_SAMPLE	1
#define FILETYPE_INST	2

touchPosition touch;
u8 frame = 0;

u8 active_buffer = FRONT_BUFFER;

u16 *main_vram_front, *main_vram_back, *sub_vram;
char *launch_path = NULL;

bool typewriter_active = false;
bool exit_requested = false;
volatile bool redraw_main_requested = false;

u16 keys_that_are_repeated = KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT;
u16 repeatkeys = 0, repeatkeys_last = 0;

// Make the key botmasks variable for switching handedness
u16 mykey_LEFT = KEY_LEFT, mykey_UP = KEY_UP, mykey_RIGHT = KEY_RIGHT, mykey_DOWN = KEY_DOWN,
	mykey_A = KEY_A, mykey_B = KEY_B, mykey_X = KEY_X, mykey_Y = KEY_Y, mykey_L = KEY_L, mykey_R = KEY_R;

GUI *gui;

// <Misc GUI>
	Button *buttonrenameinst, *buttonrenamesample, *buttontest, *buttonstopnote, *buttonemptynote, *buttondelnote, *buttoninsnote2,
		*buttondelnote2, *buttoninsnote;
	BitButton *buttonswitchsub, *buttonplay, *buttonstop, *buttonpause;
	CheckBox *cbscrolllock;
	ToggleButton *tbrecord, *tbmultisample;
	Label *labeladd, *labeloct;
	NumberBox *numberboxadd, *numberboxoctave;
	Piano *kb;
	ListBox *lbinstruments, *lbsamples;
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
		*buttonsmpreverse, *buttonrecord, *buttonsmpnormalize;

	GroupBox *gbsampleloop;
	RadioButton::RadioButtonGroup *rbg_sampleloop;
	RadioButton *rbloop_none, *rbloop_forward, *rbloop_pingpong;
	CheckBox *cbsnapto0xing;

	ToggleButton *buttonsmpdraw;
// </Sample Gui>

// <Instrument Gui>
	EnvelopeEditor *volenvedit;
	Button *btnaddenvpoint, *btndelenvpoint, *btnenvzoomin, *btnenvzoomout, *btnenvdrawmode, *btnenvsetsuspoint;
	ToggleButton *tbmapsamples;
	CheckBox *cbvolenvenabled, *cbsusenabled;
// </Instrument Gui>

// <Settings Gui>
	RadioButton::RadioButtonGroup *rbghandedness;
	RadioButton *rblefthanded, *rbrighthanded;
	GroupBox *gbhandedness, *gbdsmw;
	CheckBox *cbdsmwsend, *cbdsmwrecv;
	Button *btndsmwtoggleconnect;
	RadioButton::RadioButtonGroup *rbgoutput;
	RadioButton *rboutputmono, *rboutputstereo;
	GroupBox *gboutput;
	RadioButton::RadioButtonGroup *rbgfreq;
	RadioButton *rbfreq32, *rbfreq47;
	GroupBox *gbfreq;
	GroupBox *gblinesbeat;
	NumberBox *nblinesbeat;
	Button *btnconfigsave;
// </Settings Gui>

// <Main Screen>
	Button *buttonins, *buttondel, *buttonstopnote2, *buttoncolselect, *buttonemptynote2, *buttonunmuteall;
	BitButton *buttonswitchmain;
	Button *buttoncut, *buttoncopy, *buttonpaste, *buttonsetnotevol, *buttonseteffectcmd, *buttonseteffectpar;
	BitButton *buttonundo, *buttonredo;
	PatternView *pv;
	NumberSlider *nsnotevolume, *nseffectcmd, *nseffectpar;
	Label *labelmute, *labelnotevol, *labeleffectcmd, *labeleffectpar;
	CheckBox *cbtoggleeffects;
// </Main Screen>

// <Things that suddenly pop up>
	Typewriter *tw;
	MessageBox *mb;
// </Things that suddenly pop up>

u16 *b1n, *b1d;
int lastx, lasty;
Song *song;
State *state;
Settings *settings;
XMTransport xm_transport;

CellArray *clipboard = NULL;
ActionBuffer *action_buffer = NULL;

u8 dsmw_lastnotes[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
u8 dsmw_lastchannels[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

bool fastscroll = false;

uint16* map;

// TODO: Make own class for tracker control and remove forward declarations
void handleButtons(u16 buttons, u16 buttonsheld);
void HandleTick(void);
void handlePotPosChangeFromSong(u16 newpotpos);
void drawMainScreen(void);
void redrawSubScreen(void);
void showMessage(const char *msg, bool error);
void deleteMessageBox(void);
void stopPlay(void);

#ifdef DEBUG
void saveScreenshot(void);
void dumpSample(void);
#endif

void clearMainScreen(void)
{
	u16 col = settings->getTheme()->col_dark_bg;
	u32 colcol = col | col << 16;
	dmaFillWords(colcol, main_vram_front, 256 * 192 * 2);
	dmaFillWords(colcol, main_vram_back, 256 * 192 * 2);
}

void clearSubScreen(void)
{
	u16 col = settings->getTheme()->col_dark_bg;
	u32 colcol = col | col << 16;
	// Fill the bg with the bg color except for the place where the keyboard is
	dmaFillWords(colcol, sub_vram, 256 * 153 * 2);
	for(int y=154;y<192;++y)
	{
		dmaFillWords(0, sub_vram + (256*y), 224 * 2);
		dmaFillWords(colcol, sub_vram + (256*y) + 224, (256 - 224) * 2);
	}
}

void drawSampleNumbers(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == NULL)
	{
		for(u8 key=0; key<24; ++key)
		{
			kb->setKeyLabel(key, '0');
		}
		return;
	}

	char label;
	u8 note, sample_id;
	for(u8 key=0; key<24; ++key)
	{
		note = state->basenote + key;
		sample_id = inst->getNoteSample(note) & 0x0F;
		label = (sample_id >= 0xA) ? (sample_id - 0xA + 'a') : (sample_id + '0');

		kb->setKeyLabel(key, label);
	}
}

void updateKeyLabels(void)
{
	kb->hideKeyLabels();
	if(lbsamples->is_visible() == true)
	{
		drawSampleNumbers();
		kb->showKeyLabels();
	}
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

void handleNoteFill(u8 note, bool while_playing)
{
	u16 sel_x1, sel_y1, sel_x2, sel_y2;
	bool is_box;
	if (while_playing)
	{
		sel_x1 = 0;
		sel_x2 = song->getChannels()-1;
		sel_y1 = sel_y2 = state->getCursorRow();
		is_box = true;
	}
	else
	{
		is_box = uiPotSelection(&sel_x1, &sel_y1, &sel_x2, &sel_y2, true);
	}

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

void handleNoteStroke(u8 note)
{
	if (note == EMPTY_NOTE || note == STOP_NOTE) return;

	// If we are recording
	if(state->recording == true)
	{
		// TODO: restore pattern setting while preserving undo
		/* uiSetNote(state->channel, state->getCursorRow(), note);

		// Redraw
		DC_FlushAll();
		redraw_main_requested = true; */
	}

	// If we are in sample mapping mode, map the pressed key to the selected sample for the current instrument
	if(state->map_samples == true)
	{
		Instrument *inst = song->getInstrument(state->instrument);
		if(inst != NULL)
		{
			inst->setNoteSample(state->basenote + note, state->sample);
			DC_FlushAll();
		}

		char label;
		u8 sample_id = state->sample & 0xF;
		label = (sample_id >= 0xA) ? (sample_id - 0xA + 'a') : (sample_id + '0');
		kb->setKeyLabel(note, label);
	}

	// Play the note
	// Send "play inst" command
	CommandPlayInst(state->instrument, state->basenote + note, 255, 255); // channel==255 -> search for free channel

#ifdef MIDI
	u8 midichannel = state->instrument % 16;
	if( (state->dsmi_connected) && (state->dsmi_send) )
		dsmi_write(NOTE_ON | midichannel, state->basenote + note, 127);
#endif
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
		DC_FlushAll();
		redraw_main_requested = true;
	}

	CommandStopInst(255);

#ifdef MIDI
	u8 midichannel = state->instrument % 16;
	if( (state->dsmi_connected) && (state->dsmi_send) )
		dsmi_write(NOTE_OFF | midichannel, state->basenote + note, 127);
#endif
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
		char *str=(char*) calloc(1, SAMPLE_NAME_LENGTH + 1);
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
		free(str);
	}
}

void updateMemoryState(void)
{
	memoryiindicator->pleaseDraw();
	memoryiindicator_disk->pleaseDraw();
}

void updateFilesystemState(bool draw)
{
	fileselector->invalidateFileList();
	if(draw) fileselector->pleaseDraw();
}

void sampleChange(Sample *smp)
{
	if(smp == NULL)
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
	/*
	iprintf("Selected:");
	if(smp->is16bit()) {
		iprintf("16bit ");
	} else {
		iprintf("8bit ");
	}
	if(smp->getLoop() != 0) {
		iprintf("looping ");
	}
	iprintf("Sample.\n");
	iprintf("length: %u\n", smp->getNSamples());
	*/
}

void volEnvSetInst(Instrument *inst)
{
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
	volenvedit->pleaseDraw();
}

void handleSampleChange(u16 newsample)
{
	state->sample = newsample;

	Instrument *inst = song->getInstrument(lbinstruments->getidx());
	if(inst == 0)
		return;

	Sample *smp = inst->getSample(newsample);
	sampleChange(smp);
}
void handleInstChange(u16 newinst)
{

	state->instrument = newinst;

	lbsamples->select(0);

	Instrument *inst = song->getInstrument(newinst);
	updateSampleList(inst);
	volEnvSetInst(inst);
	updateKeyLabels();
	if(inst != NULL)
	{
		cbvolenvenabled->setChecked(inst->getVolEnvEnabled());
		handleSampleChange(0);
	}
	else
	{
		sampleChange(NULL);
		return;
	}
}

void updateLabelSongLen(void)
{
	/* char labelstr[12];
	sniprintf(labelstr, 12, "songlen:%2d", song->getPotLength());
	labelsonglen->setCaption(labelstr); */
}

void updateLabelChannels(void)
{
	char labelstr[9];
	sniprintf(labelstr, 9, "chn: %2d", song->getChannels());
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
	char *str = (char*) calloc(1, 256);

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
	for(u8 i=0;i<song->getPotLength();++i) {
		potentry = song->getPotEntry(i);
		sniprintf(str, 255, "%2x", potentry);
		lbpot->add(str);
	}

	// Update instrument list
	Instrument *inst;
	for(u8 i=0;i<MAX_INSTRUMENTS;++i)
	{
		inst = song->getInstrument(i);
		if(inst!=NULL) {
			strncpy(str, inst->getName(), 255);
			lbinstruments->set(i, str);
		} else {
			lbinstruments->set(i, "");
		}
	}

	lbinstruments->select(0);

	updateSampleList(song->getInstrument(0));

	inst = song->getInstrument(0);
	if(inst != 0)
		sampleChange(inst->getSample(0));

	volEnvSetInst(song->getInstrument(0));

	inst = song->getInstrument(0);
	if(inst != 0)
	{
		cbvolenvenabled->setChecked(inst->getVolEnvEnabled());
		cbsusenabled->setChecked(inst->getVolumeEnvelopeSustainFlag());
	}

	updateLabelChannels();
	updateLabelSongLen();
	updateTempoAndBpm();
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

	lbsamples->select(0);

	strncpy(str, song->getName(), 255);
	labelsongname->setCaption(str);

	free(str);

	drawMainScreen();

	PrintFreeMem();
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
	u8 smpidx = lbsamples->getidx();

	//
	// Create the instrument if it doesn't exist
	//
	Instrument *inst = song->getInstrument(instidx);
	if(inst == 0)
	{
		char *instname = (char*)malloc(MAX_INST_NAME_LENGTH+1);
		strncpy(instname, filename, MAX_INST_NAME_LENGTH);

		inst = new Instrument(instname);
		song->setInstrument(instidx, inst);

		free(instname);

		lbinstruments->set(state->instrument, song->getInstrument(state->instrument)->getName());
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

	sampleChange(newsmp);

	DC_FlushAll();

	return true;
}

void showSlowLoadOperation(std::function<const char*(void)> loadOp)
{
	SetYtrigger(191);
	irqSet(IRQ_VCOUNT, updateMemoryState);
	irqEnable(IRQ_VCOUNT);

	mb = new MessageBox(&sub_vram, "one moment", 0);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->show();
	mb->pleaseDraw();

	const char* res = loadOp();
	DC_FlushAll();

	deleteMessageBox();

	irqDisable(IRQ_VCOUNT);
	irqClear(IRQ_VCOUNT);

	if (res != NULL)
		showMessage(res, true);
#ifdef DEBUG
	PrintFreeMem();
#endif
}

void handleDelfileConfirmed(void)
{
	deleteMessageBox();

	File *file = fileselector->getSelectedFile();
	debugprintf("%s\n", file->name_with_path.c_str());
	if((file==0)||(file->is_dir == true)) {
		return;
	}

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
	File *file = fileselector->getSelectedFile();
	debugprintf("%s\n", file->name_with_path.c_str());
	if((file==0)||(file->is_dir == true)) {
		return;
	}

	mb = new MessageBox(&sub_vram, "are you sure?", 2, "yes", handleDelfileConfirmed, "no", deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
	mb->pleaseDraw();
}

void handleLoad(void)
{
	File *file = fileselector->getSelectedFile();
	if((file==0)||(file->is_dir == true)) {
		return;
	}

	const char *fn = file->name.c_str();
	
	if(strcasecmp(fn + strlen(fn) - 3, ".xm")==0)
	{
		stopPlay();

		delete song; // For christs sake do some checks before deleting the song!!
		pv->unmuteAll();

		showSlowLoadOperation([file](){
			Song *newsong;
			u16 err;
			err = xm_transport.load(file->name_with_path.c_str(), &newsong);
			if (err)
			{
				setSong(new Song(10, 125));
				return xm_transport.getError(err);
			}
			else
			{
				setSong(newsong);
				return (const char*) NULL;
			}
		});
	}
	else if(strcasecmp(fn + strlen(fn) - 4, ".wav")==0)
	{
		showSlowLoadOperation([file](){
			bool success = loadSample(file->name_with_path.c_str());
			return !success ? "wav loading failed" : (const char*) NULL;
		});
	}

	updateMemoryState();
}

// Reads filename and path from fileselector and saves the file
void saveFile(void)
{
	stopPlay();

	char *filename = labelFilename->getCaption();
	chdir(fileselector->getDir().c_str());

	debugprintf("saving %s ...\n", filename);

	mb = new MessageBox(&sub_vram, "one moment", 0);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->show();
	mb->pleaseDraw();

	int err = 0;
	if(rbsong->getActive() == true) // Save the song
	{
		if(song != 0) {
			err = xm_transport.save(filename, song);
		}
	}
	else if(rbsample->getActive() == true) // Save the sample
	{
		song->getInstrument(state->instrument)->getSample(state->sample)->saveAsWav(filename);
	}

	deleteMessageBox();
	updateFilesystemState(true);

	debugprintf("done\n");

	if(err > 0)
	{
		showMessage(xm_transport.getError(err), true);
	}
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
		showMessage("No filename!", true);
		return;
	}

	if(rbsample->getActive() == true) // Sample sanity checks
	{
		Instrument *inst = song->getInstrument(state->instrument);
		if(inst == NULL)
		{
			showMessage("Empty instrument!", true);
			return;
		}
		Sample *smp = inst->getSample((state->sample));
		if(smp == NULL)
		{
			showMessage("Empty sample!", true);
			return;
		}
	}

	chdir(fileselector->getDir().c_str());

	// Check if file already exists
	if(my_file_exists(filename))
	{
		mb = new MessageBox(&sub_vram, "overwrite file", 2, "yes", mbOverwrite, "no", deleteMessageBox);
		gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
		mb->reveal();
		mb->pleaseDraw();
	} else {
		saveFile();
	}

	updateMemoryState();
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
	delete tw;
	redrawSubScreen();
}


void handleTypewriterFilenameOk(void)
{
	char *text = tw->getText();
	char *name = NULL;
	int textlen = strlen(text);
	debugprintf("%s\n", text);
	if(strcmp(text,"") != 0)
	{
		if( (rbsong->getActive() == true) && (strcasecmp(text+textlen-3, ".xm") != 0) )
		{
			// Append extension
			name = (char*)malloc(textlen+3+1);
			strcpy(name,text);
			strcpy(name+textlen,".xm");
		}
		else if( (rbsample->getActive() == true) && (strcasecmp(text+textlen-4, ".wav") != 0) )
		{
			// Append extension
			name = (char*)malloc(textlen+4+1);
			strcpy(name,text);
			strcpy(name+textlen,".wav");
		}
		else
		{
			// Leave as is
			name = (char*)malloc(textlen+1);
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
	if (name != NULL) free(name);
}


void emptyNoteStroke(void) {
	handleNoteFill(EMPTY_NOTE, false);
	redraw_main_requested = true;
}


void stopNoteStroke(void) {
	handleNoteFill(STOP_NOTE, false);
	redraw_main_requested = true;
}

static void actionBufferChangeCallback(void) {
	buttonundo->set_enabled(action_buffer->can_undo());
	buttonredo->set_enabled(action_buffer->can_redo());
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


void drawMainScreen(void)
{
	// Draw widgets (to back buffer)
	gui->drawMainScreen();

	// Flip buffers
	active_buffer = !active_buffer;

	if(active_buffer == FRONT_BUFFER) {
	    bgSetMapBase(2, 2);
		main_vram_front = (uint16*)BG_BMP_RAM(2);
		main_vram_back = (uint16*)BG_BMP_RAM(8);
	} else {
	    bgSetMapBase(2, 8);
		main_vram_front = (uint16*)BG_BMP_RAM(8);
		main_vram_back = (uint16*)BG_BMP_RAM(2);
	}
}


void redrawSubScreen(void)
{
	// Fill screen
	u16 col = settings->getTheme()->col_bg;
	u32 colcol = col | col << 16;
	dmaFillWords(colcol, sub_vram, 256 * 153 * 2);

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
	state->playing = false;

	// The arm7 will get the command with a slight delay and may continue playing for
	// some ticks. But for saving battery, we only draw the screen continuously
	// if state->playing == true. So, by setting it to false here we might miss ticks
	// resultsing in the pattern view being out of sync with the song. So we wait two
	// frames to make sure the arm7 has really stopped and redraw the pattern.
	swiWaitForVBlank(); swiWaitForVBlank();
	redraw_main_requested = false;
	drawMainScreen();

#ifdef MIDI
	if( (state->dsmi_connected) && (state->dsmi_send) )
	{
		for(u8 chn=0; chn<16; ++chn) {
			dsmi_write(MIDI_CC | chn, 120, 0);
			dsmi_write(NOTE_OFF | chn, dsmw_lastnotes[chn], 0);
		}
	}
#endif
}

void stopPlay(void)
{
	state->pause = false;
	state->setPlaybackRow(0);

	stop();

	buttonpause->hide();
	buttonplay->show();
}

void pausePlay(void)
{
	state->pause = true;

	// Send stop command
	CommandStopPlay();

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
	u32 colcol;

	if(is_on)
		col = RGB15(31, 0, 0) | BIT(15); // red
	else
		col = settings->getTheme()->col_bg; // bg color
	colcol = (col) | (col << 16);

	dmaFillWords(colcol, sub_vram, 256 * 2);
	dmaFillWords(colcol, sub_vram + (256*191), 256 * 2);

	for(u8 i=1; i<191; ++i)
	{
		sub_vram[256*i] = col;
		sub_vram[256*i+255] = col;
	}
}


// Updates several GUI elements that display pattern
// related info to the new pattern
void updateGuiToNewPattern(u8 newpattern)
{
	// Update pattern length slider
	nsptnlen->setValue(song->getPatternLength(newpattern));
}


// Callback called from song when the pot element changes during playback
void handlePotPosChangeFromSong(u16 newpotpos)
{
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
}

#ifdef MIDI

void handleDSMWRecv(void)
{
	u8 message, data1, data2;

	while(dsmi_read(&message, &data1, &data2))
	{
		if(state->dsmi_recv) {

			debugprintf("got sth\n");

			u8 type = message & 0xF0;
			switch(type)
			{
				case NOTE_ON: {
					u8 inst = message & 0x0F;
					u8 note = data1;
					u8 volume = data2 * 2;
					u8 channel = 255;
					debugprintf("on %d %d\n", inst, note);
					CommandPlayInst(inst, note, volume, channel);
					break;
				}

				case NOTE_OFF: {
					u8 channel = message & 0x0F;
					CommandStopInst(channel);
					break;
				}
			}
		}
	}
}

#endif

// Callback called from lbpot when the user changes the pot element
void handlePotPosChangeFromUser(u16 newpotpos)
{
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
		DC_FlushAll();

		redraw_main_requested = true;

		// Update pattern length slider
		nsptnlen->setValue(song->getPatternLength(song->getPotEntry(state->potpos)));
	}
	char str[3];
	sniprintf(str, sizeof(str), "%2x", pattern);
	lbpot->set(state->potpos, str);
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
		DC_FlushAll();

		redraw_main_requested = true;

		// Update pattern length slider
		nsptnlen->setValue(song->getPatternLength(song->getPotEntry(state->potpos)));
	}
	char str[3];
	sniprintf(str, sizeof(str), "%2x", pattern);
	lbpot->set(state->potpos, str);
}


// Inserts a pattern into the pot (copies the current pattern)
void handlePotIns(void)
{
	song->potIns(state->potpos, song->getPotEntry(state->potpos));
	// TODO: turn into undo operation
	action_buffer->clear();
	DC_FlushAll();
	lbpot->ins(lbpot->getidx(), lbpot->get(lbpot->getidx()));
	updateLabelSongLen();
}


void handlePotDel(void)
{
	if(song->getPotLength()>1) {
		lbpot->del();
	}

	song->potDel(state->potpos);

	// TODO: turn into undo operation
	action_buffer->clear();
	DC_FlushAll();

	if(state->potpos>=song->getPotLength()) {
		state->potpos = song->getPotLength() - 1;
	}

	updateLabelSongLen();

	if(song->getRestartPosition() >= song->getPotLength()) {
		song->setRestartPosition( song->getPotLength() - 1 );
		nsrestartpos->setValue( song->getRestartPosition() );
		DC_FlushAll();
	}
}

void handlePtnClone(void)
{
	u16 newidx = song->getNumPatterns();
	if(newidx == MAX_PATTERNS)
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
	DC_FlushAll();
	char numberstr[3] = {0};
	siprintf(numberstr, "%2x", newidx);
	lbpot->ins(lbpot->getidx()+1, numberstr);

	updateLabelSongLen();
}

void handleChannelAdd(void)
{
	// TODO: turn into undo operation
	song->channelAdd();
	redraw_main_requested = true;
	updateLabelChannels();
}


void handleChannelDel(void)
{
	// TODO: turn into undo operation
	song->channelDel();

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
}


void handlePtnLengthChange(s32 newlength)
{
	// TODO: turn into undo operation
	if(newlength != song->getPatternLength(song->getPotEntry(state->potpos)))
	{
		song->resizePattern(song->getPotEntry(state->potpos), newlength);
		DC_FlushAll();
		// Scroll back if necessary
		if(state->getPlaybackRow() >= newlength) {
			state->setPlaybackRow(newlength-1);
		}
		if(state->getCursorRow() >= newlength) {
			state->setCursorRow(newlength-1);
		}
		redraw_main_requested = true;
	}
}


void handleTempoChange(u8 tempo) {
	song->setTempo(tempo);
	DC_FlushAll();
}

void handleBpmChange(s32 bpm) {
	song->setBpm(bpm);
	DC_FlushAll();
}

void handleLinesBeatChange(u8 lpb) {
	settings->setLinesPerBeat(lpb);
	pv->setLinesPerBeat(lpb);
	redraw_main_requested = true;
}

void handleRestartPosChange(s32 restartpos)
{
	if(restartpos > song->getPotLength()-1) {
		nsrestartpos->setValue(song->getPotLength()-1);
		restartpos = song->getPotLength()-1;
	}
	song->setRestartPosition(restartpos);
	DC_FlushAll();
}

void zapPatterns(void)
{
	song->zapPatterns();
	action_buffer->clear();
	DC_FlushAll();
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
	updateMemoryState();
}

void zapInstruments(void)
{
	song->zapInstruments();
	DC_FlushAll();
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
	updateMemoryState();
}

void zapSong(void) {
	deleteMessageBox();
	delete song;
	setSong(new Song());
	updateMemoryState();
}

void handleZap(void)
{
	stopPlay(); // Safety first

	mb = new MessageBox(&sub_vram, "what to zap", 4, "patterns", zapPatterns,
		"instruments", zapInstruments, "song", zapSong, "cancel",
		deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
}

void handleRowChangeFromSong(u16 row)
{
	state->setPlaybackRow(row);

	if(!state->playing)
		return;

	if(buttonemptynote->isPenDown())
		handleNoteFill(EMPTY_NOTE, true);
	
	redraw_main_requested = true;

#ifdef MIDI
	if( (state->dsmi_connected) && (state->dsmi_send) )
	{
		Cell ** pattern = song->getPattern( song->getPotEntry( state->potpos ) );

		Cell *curr_cell;

		for(u8 chn=0; chn < song->getChannels(); ++chn)
		{
			if(song->channelMuted(chn))
				continue;

			curr_cell = &(pattern[chn][state->getCursorRow()]);

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
				u8 midichannel = curr_cell->instrument % 16;
				dsmi_write(NOTE_ON | midichannel, curr_cell->note, curr_cell->volume / 2);

				dsmw_lastchannels[chn] = midichannel;
				dsmw_lastnotes[chn] = curr_cell->note;
			}
		}
	}
#endif
}

void handleStop(void)
{
	state->playing = false;
}

void handleSamplePreviewToggled(bool on)
{
	settings->setSamplePreview(on);
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

		// Preview wav files
		if(slen > 4 && (strcasecmp(&str[slen-4], ".wav") == 0) && (settings->getSamplePreview() == true) )
		{
			// Stop playing sample if necessary
			CommandStopInst(0);

			// Check if it's not too big
			u32 smpsize = my_getFileSize(file.name_with_path.c_str());

			// TODO: instead of this
			u8* testptr = (u8*)malloc(smpsize); // Try to malloc it
			if(testptr == 0)
			{
				debugprintf("not enough ram for preview\n");
			}
			else
			{
				debugprintf("previewing\n");
				free(testptr);

				// Load sample
				bool success;
				Sample *smp = new Sample(file.name_with_path.c_str(), false, &success);
				if(!success)
				{
					delete smp;
				}
				else
				{
					// Stop and delete previously playing preview sample
					if(state->preview_sample)
					{
						CommandStopSample(0);
						while(state->preview_sample)
						{
							swiWaitForVBlank();
						}
					}

					// Play it
					state->preview_sample = smp;
					DC_FlushAll();
					CommandPlaySample(smp, 4*12, 255, 0);

					updateMemoryState();

					// When the sample has finished playing, the arm7 sends a signal,
					// so the arm9 can delete the sampleb
				}
			}
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

	updateMemoryState();
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
	}
}

void setEffectParam(u16 eff_par)
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
				cell.effect_param = eff_par;
				*fill->ptr(chn - sel_x1, row - sel_y1) = cell;
			}
        action_buffer->add(song, new MultipleCellSetAction(state, sel_x1, sel_y1, fill, false));
		redraw_main_requested = true;
	}
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
}

// number slider
void handleEffectCommandChanged(s32 eff)
{
	setEffectCommand(eff);
}

// button
void handleSetEffectCommand(void)
{
	setEffectCommand(nseffectcmd->getValue());
}

// number slider
void handleEffectParamChanged(s32 eff_par)
{
	setEffectParam(eff_par);
}

// button
void handleSetEffectParam(void)
{
	setEffectParam(nseffectpar->getValue());
}
void showTypewriter(const char *prompt, const char *str, void (*okCallback)(void), void (*cancelCallback)(void))
{
    // TODO: Migrate to new TobKit to eliminate such ugliness
#define SUB_BG1_X0 (*(vuint16*)0x04001014)
#define SUB_BG1_Y0 (*(vuint16*)0x04001016)

	tw = new Typewriter(prompt, (uint16*)CHAR_BASE_BLOCK_SUB(1),
		(uint16*)SCREEN_BASE_BLOCK_SUB(12), 3, &sub_vram, &SUB_BG1_X0, &SUB_BG1_Y0);

	tw->setText(str);
	gui->registerOverlayWidget(tw, mykey_LEFT|mykey_RIGHT, SUB_SCREEN);
	if(okCallback!=0) {
		tw->registerOkCallback(okCallback);
	}
	if(cancelCallback != 0) {
		tw->registerCancelCallback(cancelCallback);
	}
	typewriter_active = true;
	tw->reveal();
}


void showTypewriterForFilename(void) {
	showTypewriter("filename", labelFilename->getCaption(), handleTypewriterFilenameOk, deleteTypewriter);
}

void handleTypewriterNewFolderOk(void)
{
	char *text = tw->getText();
	if(text[0] != '\0' && strchr(text, '/') == NULL && strchr(text, ':') == NULL)
	{
		mkdir(text, 0777);
		// TODO: Enter directory after creating it?
		updateFilesystemState(true);
	}
	deleteTypewriter();
}

void showTypewriterForNewFolder(void) {
	showTypewriter("dir name", "", handleTypewriterNewFolderOk, deleteTypewriter);
}

void handleTypewriterInstnameOk(void)
{
	song->getInstrument(lbinstruments->getidx())->setName(tw->getText());
	lbinstruments->set( lbinstruments->getidx(), tw->getText() );

	deleteTypewriter();
}


void showTypewriterForInstRename(void)
{
	Instrument *inst = song->getInstrument(lbinstruments->getidx());
	if(inst==NULL) {
		return;
	}

	showTypewriter("inst name", lbinstruments->get(lbinstruments->getidx()), handleTypewriterInstnameOk, deleteTypewriter);
}

void handleTypewriterSongnameOk(void)
{
	song->setName(tw->getText());
	labelsongname->setCaption(song->getName());
	deleteTypewriter();
}

void showTypewriterForSongRename(void)
{
	if(!state->playing) {
		showTypewriter("song name", song->getName(), handleTypewriterSongnameOk, deleteTypewriter);
	}
}

void handleTypewriterSampleOk(void)
{
	song->getInstrument(lbinstruments->getidx())->getSample(lbsamples->getidx())->setName(tw->getText());
	lbsamples->set( lbsamples->getidx(), tw->getText() );

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
	if(on)
	{
		drawSampleNumbers();
		kb->showKeyLabels();
		tbmultisample->setCaption("-");
		lbinstruments->resize(114, 67);
		buttonrenamesample->show();
		lbsamples->show();
		tbmapsamples->show();
	}
	else
	{
		kb->hideKeyLabels();
		tbmultisample->setCaption("+");
		buttonrenamesample->hide();
		lbsamples->hide();
		lbinstruments->resize(114, 89);
		tbmapsamples->hide();
	}
}

void showTypewriterForSampleRename(void)
{
	Instrument *inst = song->getInstrument(lbinstruments->getidx());
	if(inst == 0)
		return;

	Sample *sample = inst->getSample(lbsamples->getidx());
	if(sample == 0)
		return;

	showTypewriter("sample name", lbsamples->get(lbsamples->getidx()), handleTypewriterSampleOk, deleteTypewriter);
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

	lbsamples->set(state->sample, smp->getName());

	volEnvSetInst(inst);

	cbvolenvenabled->setChecked(inst->getVolEnvEnabled());

	sampleChange(smp);
	updateKeyLabels();
	redrawSubScreen();
}

void handleRecordSampleCancel(void)
{
	// Kill record box
	gui->unregisterOverlayWidget();
	delete recordbox;

	// Turn off the mic
	CommandMicOff();

	redrawSubScreen();
}

// OMG FUCKING BEST FEATURE111
void handleRecordSample(void)
{
	// Check RAM first!
	void *testbuf = malloc(RECORDBOX_SOUNDDATA_SIZE * 2);
	if(testbuf == 0)
	{
		showMessage("not enough ram free!", true);
		return;
	}

	free(testbuf);

	// Turn on the mic
	CommandMicOn();

	// Get sample
	Sample *smp = 0;
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst != 0)
		smp = inst->getSample(state->sample);

	// Show record box

	recordbox = new RecordBox(&sub_vram, handleRecordSampleOK, handleRecordSampleCancel, smp, inst, state->sample);

	gui->registerOverlayWidget(recordbox, KEY_A | KEY_B, SUB_SCREEN);

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

	normalizeBox = new NormalizeBox(&sub_vram, handleNormalizeOK, handleNormalizeCancel);
	gui->registerOverlayWidget(normalizeBox, 0, SUB_SCREEN);
	normalizeBox->reveal();
}

void swapControls(Handedness handedness)
{
	if(handedness == LEFT_HANDED)
	{
		mykey_UP = KEY_X;
		mykey_DOWN = KEY_B;
		mykey_LEFT = KEY_Y;
		mykey_RIGHT = KEY_A;
		mykey_L = KEY_R;
		mykey_R = KEY_L;
		mykey_A = KEY_RIGHT;
		mykey_B = KEY_DOWN;
		mykey_X = KEY_UP;
		mykey_Y = KEY_LEFT;
		keys_that_are_repeated = KEY_A | KEY_B | KEY_X | KEY_Y;
	}
	else
	{
		mykey_UP = KEY_UP;
		mykey_DOWN = KEY_DOWN;
		mykey_LEFT = KEY_LEFT;
		mykey_RIGHT = KEY_RIGHT;
		mykey_L = KEY_L;
		mykey_R = KEY_R;
		mykey_A = KEY_A;
		mykey_B = KEY_B;
		mykey_X = KEY_X;
		mykey_Y = KEY_Y;
		keys_that_are_repeated = KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT;
	}
}

void swapPatternButtons(Handedness handedness)
{
	u8 x, y;
	pv->getPos(&x, &y, NULL, NULL);

	Handedness current_handedness = x == 30 ? LEFT_HANDED : RIGHT_HANDED;
	if (current_handedness == handedness) return;

	std::vector<Widget*> widgets = gui->getWidgets(MAIN_SCREEN);
	int offset = (handedness == LEFT_HANDED) ? -225 : 225;

	for (Widget* widget : widgets) {
		widget->getPos(&x, &y, NULL, NULL);
		widget->setPos(x + offset, y);
	}
	pv->setPos(handedness == LEFT_HANDED ? 30 : 0, 0);

	redraw_main_requested = true;
}

void handleHandednessChange(u8 handedness)
{
	clearMainScreen();

	if(handedness == 0)
	{
		settings->setHandedness(LEFT_HANDED);
		swapControls(LEFT_HANDED);
		swapPatternButtons(LEFT_HANDED);
	}
	else
	{
		settings->setHandedness(RIGHT_HANDED);
		swapControls(RIGHT_HANDED);
		swapPatternButtons(RIGHT_HANDED);
	}
}

void handleOutputModeChange(u8 outputMode)
{
	settings->setStereoOutput(outputMode != 0);
	CommandSetStereoOutput(outputMode != 0);
	stopPlay();
}

void handleOutputFreqChange(u8 freq)
{
	settings->setFreq47kHz(freq != 0);
	soundExtSetFrequency(freq ? 47 : 32);
}

void switchScreens(void)
{
	lcdSwap();
	gui->switchScreens();
	pv->clearSelection();
	redraw_main_requested = false;
	drawMainScreen();
}


// Create the song and do other init stuff yet to be determined.
void setupSong(void) {
	song = new Song(6, 125);
	action_buffer = new ActionBuffer(isDSiMode() ? 1024 : 256);
	DC_FlushAll();
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

	mb = new MessageBox(&sub_vram, "really exit", 2, "yes", requestExit, "no", deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
	mb->pleaseDraw();
}

void showMessage(const char *msg, bool error)
{
	mb = new MessageBox(&sub_vram, msg, 1, error ? "doh!" : "yay!", deleteMessageBox);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->reveal();
}

void showAboutBox(void)
{
	char msg[256];
	sniprintf(msg, 256, "NitrousTracker " VERSION " (" GIT_HASH ")");
	mb = new MessageBox(&sub_vram, msg, 2, "track on!", deleteMessageBox, "exit", showExitBox);
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
		action_buffer->add(song, new MultipleCellSetAction(state, state->channel, state->getCursorRow(), clipboard, true));
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
	DC_FlushAll();
}

void handleSamplePanningChange(s32 newpanning)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	u8 pan = newpanning * 2;

	smp->setPanning(pan);
	smp->setBasePanning();
	DC_FlushAll();
}

void handleSampleRelNoteChange(s32 newnote)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	DC_FlushAll();

	smp->setRelNote(newnote);
}

void handleSampleFineTuneChange(s32 newfinetune)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;

	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;

	DC_FlushAll();

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

	DC_FlushAll();

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

	DC_FlushAll();

	sampledisplay->setSample(smp);
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

	DC_FlushAll();

	sampledisplay->setSample(smp);
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

	DC_FlushAll();

	sampledisplay->setSample(smp);
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
	mb = new MessageBox(&sub_vram, "connecting ...", 0);
	gui->registerOverlayWidget(mb, 0, SUB_SCREEN);
	mb->show();
	mb->pleaseDraw();

	int res = dsmi_connect();
	deleteMessageBox();

	if(res == 0) {
		showMessage("Sorry, couldn't connect.", true);
		state->dsmi_connected = false;
	} else {
		debugprintf("YAY, connected!\n");
		btndsmwtoggleconnect->setCaption("disconnect");
        btndsmwtoggleconnect->pleaseDraw();
        state->dsmi_connected = true;
	}
}

void dsmiDisconnect(void)
{
	dsmi_disconnect();

	btndsmwtoggleconnect->setCaption("connect");
	btndsmwtoggleconnect->pleaseDraw();

	state->dsmi_connected = false;
}

void dsmiToggleConnect(void)
{
	if(!state->dsmi_connected)
		dsmiConnect();
	else
		dsmiDisconnect();
}

void handleDsmiSendToggled(bool is_active)
{
	state->dsmi_send = is_active;
}

void handleDsmiRecvToggled(bool is_active)
{
	state->dsmi_recv = is_active;
}
#endif

void saveConfig(void)
{
	if (settings->writeIfChanged())
		showMessage("config saved!", false);
}

void toggleMapSamples(bool is_active)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == NULL)
		return;

	if(is_active)
	{
		if(tbmultisample->getState() == false)
			tbmultisample->setState(true);
	}

	state->map_samples = is_active;
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

	DC_FlushAll();
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

	DC_FlushAll();
}

void handleSnapTo0XingToggled(bool on)
{
	sampledisplay->setSnapToZeroCrossing(on);
}

void envZoomIn(void)
{
	volenvedit->zoomIn();
}

void envZoomOut(void)
{
	volenvedit->zoomOut();
}

void volEnvPointsChanged(void)
{
	Instrument *inst = song->getInstrument(state->instrument);
	if(inst == 0)
		return;

	u16 *xs, *ys;
	u8 n_points = volenvedit->getPoints(&xs, &ys);

	inst->setVolumeEnvelopePoints(xs, ys, n_points);

	toggleVolEnvEnabled(n_points != 0 && inst->getVolEnvEnabled());
	volenvedit->pleaseDraw();

	DC_FlushAll();
}

void volEnvDrawFinish(void)
{
	cbvolenvenabled->setChecked(true);
	toggleVolEnvEnabled(true);
	DC_FlushAll();
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

	DC_FlushAll();
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
}

void sampleDrawToggle(bool on)
{
	sampledisplay->setDrawMode(on);
}

void setupGUI(bool dldi_enabled)
{
	gui = new GUI();
	gui->setTheme(settings->getTheme(), settings->getTheme()->col_dark_bg);

	kb = new Piano(0, 152, 224, 40, (uint16*)CHAR_BASE_BLOCK_SUB(0), (uint16*)SCREEN_BASE_BLOCK_SUB(1/*8*/), &sub_vram);
	kb->registerNoteCallback(handleNoteStroke);
	kb->registerReleaseCallback(handleNoteRelease);

	pixmaplogo = new GradientIcon(98, 1, 80, 17, settings->getTheme()->col_light_ctrl, settings->getTheme()->col_dark_ctrl,
		(const u32*) nitrotracker_logo_raw, &sub_vram);
	pixmaplogo->registerPushCallback(showAboutBox);

	tabbox = new TabBox(1, 1, 139, 151, &sub_vram, TABBOX_ORIENTATION_TOP, 16);
	tabbox->setTheme(settings->getTheme(), settings->getTheme()->col_dark_bg);
	tabbox->addTab(icon_song_raw, 0);
	if (dldi_enabled)
		tabbox->addTab(icon_disk_raw, 1);
	tabbox->addTab(icon_sample_raw, 2);
	tabbox->addTab(icon_trumpet_raw, 3);
	tabbox->addTab(icon_wrench_raw, 4);

	// <Disk OP GUI>
		fileselector = new FileSelector(38, 21, 100, 111, &sub_vram);

		std::vector<std::string> samplefilter;
		samplefilter.push_back("wav");

		fileselector->addFilter("sample", samplefilter);

		std::vector<std::string> songfilter;
		songfilter.push_back("xm");
		fileselector->addFilter("song", songfilter);
		std::vector<std::string> instfilter;
		instfilter.push_back("xi");
		fileselector->addFilter("instrument", instfilter);
		fileselector->selectFilter("song");
		fileselector->registerFileSelectCallback(handleFileChange);
		fileselector->registerDirChangeCallback(handleDirChange);

		rbgdiskop = new RadioButton::RadioButtonGroup();

		rbsong   = new RadioButton(2, 21, 36, 14, &sub_vram, rbgdiskop);

		rbsong->setCaption("sng");

		rbsample = new RadioButton(2, 36, 36, 14, &sub_vram, rbgdiskop);

		rbsample->setCaption("smp");

		//rbinst   = new RadioButton(2, 51, 36, 14, &sub_vram, "ins", rbgdiskop);
		rbgdiskop->setActive(0);

		rbgdiskop->registerChangeCallback(handleDiskOPChangeFileType);

		memoryiindicator_disk = new MemoryIndicator(3, 51, 34, 8, &sub_vram, true);

		labelramusage_disk = new Label(8, 59, 34, 10, &sub_vram, false);
		labelramusage_disk->setCaption("ram");

		cbsamplepreview = new CheckBox(4, 70, 34, 14, &sub_vram, false, true);
		cbsamplepreview->setCaption("pre");
		cbsamplepreview->registerToggleCallback(handleSamplePreviewToggled);

		buttonload = new Button(3, 86, 34, 14, &sub_vram);
		buttonload->setCaption("load");
		buttonload->registerPushCallback(handleLoad);

		buttonsave = new Button(3, 102, 34, 14, &sub_vram);
		buttonsave->setCaption("save");
		buttonsave->registerPushCallback(handleSave);

		buttondelfile = new Button(3, 118, 34, 14, &sub_vram);
		buttondelfile->setCaption("del");
		buttondelfile->registerPushCallback(handleDelfile);

		labelFilename = new Label(3, 134, 97, 14, &sub_vram);
		labelFilename->setCaption("");
		labelFilename->registerPushCallback(showTypewriterForFilename);

		buttonchangefilename = new Button(101, 134, 22, 14, &sub_vram);
		buttonchangefilename->setCaption("...");
		buttonchangefilename->registerPushCallback(showTypewriterForFilename);

		buttonnewfolder = new BitButton(124, 134, 14, 14, &sub_vram, icon_new_folder_raw, 8, 8, 3, 3);
		buttonnewfolder->registerPushCallback(showTypewriterForNewFolder);

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
		lbpot = new ListBox(4, 21, 50, 78, &sub_vram, 1, true);
		lbpot->set(0," 0");
		lbpot->registerChangeCallback(handlePotPosChangeFromUser);
		buttonpotup = new Button(70, 47, 14, 12, &sub_vram);
		buttonpotup->setCaption(">");
		buttonpotup->registerPushCallback(handlePotInc);
		buttonpotdown = new Button(55, 47, 14, 12, &sub_vram);
		buttonpotdown->setCaption("<");
		buttonpotdown->registerPushCallback(handlePotDec);
		buttonins = new Button(55, 21, 29, 12, &sub_vram);
		buttonins->setCaption("ins");
		buttonins->registerPushCallback(handlePotIns);
		buttondel = new Button(55, 60, 29, 12, &sub_vram);
		buttondel->setCaption("del");
		buttondel->registerPushCallback(handlePotDel);
		buttoncloneptn = new Button(55, 34, 29, 12, &sub_vram);
		buttoncloneptn->setCaption("cln");
		buttoncloneptn->registerPushCallback(handlePtnClone);
		tbqueuelock = new ToggleButton(55, 74, 29, 12, &sub_vram, true);
		tbqueuelock->setCaption("lock");
		tbqueuelock->registerToggleCallback(toggleQueueLock);
		tbpotloop = new ToggleButton(55, 87, 29, 12, &sub_vram, true);
		tbpotloop->setCaption("loop");
		tbpotloop->registerToggleCallback(handleLoopToggle);

		labelptnlen = new Label(87, 48, 50, 12, &sub_vram, false);
		labelptnlen->setCaption("ptn len:");
		nsptnlen = new NumberSlider(105, 60, 32, 17, &sub_vram, DEFAULT_PATTERN_LENGTH, 1, 256, true);
		nsptnlen->registerChangeCallback(handlePtnLengthChange);

		labelchannels = new Label(87, 22, 48, 12, &sub_vram, false);
		labelchannels->setCaption("chn:  4");
		buttonlesschannels = new Button(112, 34, 12, 12, &sub_vram);
		buttonlesschannels->setCaption("-");
		buttonlesschannels->registerPushCallback(handleChannelDel);
		buttonmorechannels = new Button(125, 34, 12, 12, &sub_vram);
		buttonmorechannels->setCaption("+");
		buttonmorechannels->registerPushCallback(handleChannelAdd);

		labeltempo = new Label(4, 103, 32, 12, &sub_vram, false);
		labeltempo->setCaption("tmp");
		labelbpm = new Label(38, 103, 32, 12, &sub_vram, false);
		labelbpm->setCaption("bpm");
		labelrestartpos = new Label(72, 103, 46, 12, &sub_vram, false);
		labelrestartpos->setCaption("restart");
		nbtempo = new NumberBox(4, 115, 32, 17, &sub_vram, 1, 1, 31);
		nbtempo->registerChangeCallback(handleTempoChange);
#ifdef DEBUG
		nsbpm = new NumberSlider(38, 115, 32, 17, &sub_vram, 120, 1, 255);
#else
		nsbpm = new NumberSlider(38, 115, 32, 17, &sub_vram, 120, 32, 255);
#endif
		nsbpm->registerChangeCallback(handleBpmChange);
		nsrestartpos = new NumberSlider(72, 115, 32, 17, &sub_vram, 0, 0, 255, true);
		nsrestartpos->registerChangeCallback(handleRestartPosChange);

		labelsongname = new Label(4, 134, 113, 14, &sub_vram, true);
		labelsongname->setCaption("unnamed");
		labelsongname->registerPushCallback(showTypewriterForSongRename);

		buttonrenamesong = new Button(118, 134, 20, 14, &sub_vram);
		buttonrenamesong->setCaption("...");
		buttonrenamesong->registerPushCallback(showTypewriterForSongRename);

		buttonzap = new Button(107, 116, 30, 14, &sub_vram);
		buttonzap->setCaption("zap!");
		buttonzap->registerPushCallback(handleZap);

		labelramusage = new Label(87, 78, 52, 12, &sub_vram, false);
		labelramusage->setCaption("ram use");

		memoryiindicator = new MemoryIndicator(87, 90, 50, 8, &sub_vram);

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
	// </Song gui>

	// <Sample Gui>

	sampledisplay = new SampleDisplay(4, 23, 131, 70, &sub_vram);
	sampledisplay->setActive();

	sampletabbox = new TabBox(3, 94, 132, 55, &sub_vram, TABBOX_ORIENTATION_LEFT, 11);
	sampletabbox->setTheme(settings->getTheme(), settings->getTheme()->col_dark_bg);
	sampletabbox->addTab(sampleedit_wave_icon_raw, 0);
	sampletabbox->addTab(sampleedit_draw_small_raw, 1);
	sampletabbox->addTab(sampleedit_control_icon_raw, 2);
	sampletabbox->addTab(sampleedit_loop_icon_raw, 3);


	//sampletabbox->addTab(sampleedit_chip_icon);
	sampletabbox->registerTabChangeCallback(sampleTabBoxChage);

	// <Sample editing>
		labelsampleedit_record = new Label(18, 99, 21, 30, &sub_vram, true);
		labelsampleedit_record->setCaption("rec");

		buttonrecord = new BitButton(20, 110, 17, 17, &sub_vram, sampleedit_record_raw);
		buttonrecord->registerPushCallback(handleRecordSample);

		labelsampleedit_select = new Label(38, 99, 39, 30, &sub_vram, true);
		labelsampleedit_select->setCaption("select");

		buttonsmpselall = new BitButton(40, 110, 17, 17, &sub_vram, sampleedit_all_raw);
		buttonsmpselall->registerPushCallback(sample_select_all);

		buttonsmpselnone = new BitButton(58, 110, 17, 17, &sub_vram, sampleedit_none_raw);
		buttonsmpselnone->registerPushCallback(sample_clear_selection);

		labelsampleedit_edit = new Label(76, 99, 57, 48, &sub_vram, true);
		labelsampleedit_edit->setCaption("edit");

		buttonsmpfadein = new BitButton(78, 110, 17, 17, &sub_vram, sampleedit_fadein_raw);
		buttonsmpfadein->registerPushCallback(sample_fade_in);

		buttonsmpfadeout = new BitButton(96, 110, 17, 17, &sub_vram, sampleedit_fadeout_raw);
		buttonsmpfadeout->registerPushCallback(sample_fade_out);

		buttonsmpreverse = new BitButton(78, 128, 17, 17, &sub_vram, sampleedit_reverse_raw);
		buttonsmpreverse->registerPushCallback(sample_reverse);

		buttonsmpseldel = new BitButton(96, 128, 17, 17, &sub_vram, sampleedit_del_raw);
		buttonsmpseldel->registerPushCallback(sample_del_selection);

		buttonsmpnormalize = new BitButton(114, 110, 17, 17, &sub_vram, sampleedit_normalize_raw);
		buttonsmpnormalize->registerPushCallback(sample_show_normalize_window);

		sampletabbox->registerWidget(buttonrecord, 0, 0);
		sampletabbox->registerWidget(buttonsmpselall, 0, 0);
		sampletabbox->registerWidget(buttonsmpselnone, 0, 0);
		sampletabbox->registerWidget(buttonsmpseldel, 0, 0);
		sampletabbox->registerWidget(buttonsmpfadein, 0, 0);
		sampletabbox->registerWidget(buttonsmpfadeout, 0, 0);
		sampletabbox->registerWidget(buttonsmpreverse, 0, 0);
		sampletabbox->registerWidget(buttonsmpnormalize, 0, 0);
		sampletabbox->registerWidget(labelsampleedit_edit, 0, 0);
		sampletabbox->registerWidget(labelsampleedit_select, 0, 0);
		sampletabbox->registerWidget(labelsampleedit_record, 0, 0);
	// </Sample editing>

	// <Drawing and Generating>
		buttonsmpdraw = new ToggleButton(18, 96, 17, 17, &sub_vram);
		buttonsmpdraw->setBitmap(sampleedit_draw_raw);
		buttonsmpdraw->registerToggleCallback(sampleDrawToggle);

		sampletabbox->registerWidget(buttonsmpdraw, 0, 1);
	// </Drawing and Generating>

	// <Sample settings>
		labelsamplevolume = new Label(22, 108, 25, 10, &sub_vram, false);
		labelsamplevolume->setCaption("vol");

		labelpanning = new Label(19, 127, 25, 10, &sub_vram, false);
		labelpanning->setCaption("pan");

		labelrelnote = new Label(79, 108, 25, 10, &sub_vram, false);
		labelrelnote->setCaption("rel");

		labelfinetune = new Label(75, 127, 30, 10, &sub_vram, false);
		labelfinetune->setCaption("tun");

		nssamplevolume = new NumberSlider(40, 103, 32, 17, &sub_vram, 64, 0, 64);
		nssamplevolume->registerChangeCallback(handleSampleVolumeChange);

		nspanning = new NumberSlider(40, 122, 32, 17, &sub_vram, 64, 0, 127, false);
		nspanning->registerChangeCallback(handleSamplePanningChange);

		nsrelnote = new NumberSliderRelNote(94, 103, 38, 17, &sub_vram, 0);
		nsrelnote->registerChangeCallback(handleSampleRelNoteChange);

		nsfinetune = new NumberSlider(94, 122, 38, 17, &sub_vram, 0, -128, 127);
		nsfinetune->registerChangeCallback(handleSampleFineTuneChange);

		sampletabbox->registerWidget(nssamplevolume, 0, 2);
		sampletabbox->registerWidget(nspanning, 0, 2);
		sampletabbox->registerWidget(nsrelnote, 0, 2);
		sampletabbox->registerWidget(nsfinetune, 0, 2);
		sampletabbox->registerWidget(labelfinetune, 0, 2);
		sampletabbox->registerWidget(labelrelnote, 0, 2);
		sampletabbox->registerWidget(labelsamplevolume, 0, 2);
		sampletabbox->registerWidget(labelpanning, 0, 2);
	// </Sample settings>

	// <Looping>
		gbsampleloop = new GroupBox(19, 99, 110, 32, &sub_vram);
		gbsampleloop->setText("loop type");

		rbg_sampleloop = new RadioButton::RadioButtonGroup();

		rbloop_none     = new RadioButton(21, 110, 40, 10, &sub_vram, rbg_sampleloop);
		rbloop_forward  = new RadioButton(68, 110, 40, 10, &sub_vram, rbg_sampleloop);
		rbloop_pingpong = new RadioButton(21, 120, 40, 10, &sub_vram, rbg_sampleloop);

		rbloop_none->setCaption("none");
		rbloop_forward->setCaption("forward");
		rbloop_pingpong->setCaption("ping-pong");

		rbloop_none->setActive(true);

		rbg_sampleloop->registerChangeCallback(handleSampleLoopChanged);

		cbsnapto0xing = new CheckBox(50, 134, 77, 10, &sub_vram, true, true);
		cbsnapto0xing->setCaption("snap");
		cbsnapto0xing->registerToggleCallback(handleSnapTo0XingToggled);

		sampletabbox->registerWidget(rbloop_none, 0, 3);
		sampletabbox->registerWidget(rbloop_forward, 0, 3);
		sampletabbox->registerWidget(rbloop_pingpong, 0, 3);
		sampletabbox->registerWidget(gbsampleloop, 0, 3);
		sampletabbox->registerWidget(cbsnapto0xing, 0, 3);
	// </Looping>

	tabbox->registerWidget(sampledisplay, 0, 2);
	tabbox->registerWidget(sampletabbox, 0, 2);
	// </Sample Gui>

	// <Instruments Gui>
		volenvedit = new EnvelopeEditor(5, 24, 130, 72, &sub_vram, MAX_ENV_X, MAX_ENV_Y, MAX_ENV_POINTS);
		volenvedit->registerPointsChangeCallback(volEnvPointsChanged);
		volenvedit->registerDrawFinishCallback(volEnvDrawFinish);

		cbvolenvenabled = new CheckBox(6, 98, 60, 10, &sub_vram, true, false);
		cbvolenvenabled->setCaption("env on");
		cbvolenvenabled->registerToggleCallback(toggleVolEnvEnabled);

		btnaddenvpoint = new Button(72, 100, 30, 10, &sub_vram);
		btnaddenvpoint->setCaption("add");
		btnaddenvpoint->registerPushCallback(addEnvPoint);

		btndelenvpoint = new Button(104, 100, 30, 10, &sub_vram);
		btndelenvpoint->setCaption("del");
		btndelenvpoint->registerPushCallback(delEnvPoint);

		btnenvzoomin = new Button(72, 112, 30, 10, &sub_vram);
		btnenvzoomin->setCaption("+");
		btnenvzoomin->registerPushCallback(envZoomIn);

		btnenvzoomout = new Button(104, 112, 30, 10, &sub_vram);
		btnenvzoomout->setCaption("-");
		btnenvzoomout->registerPushCallback(envZoomOut);

		btnenvdrawmode = new Button(6, 112, 60, 10, &sub_vram);
		btnenvdrawmode->setCaption("draw env");
		btnenvdrawmode->registerPushCallback(envStartDrawMode);
    
    btnenvsetsuspoint = new Button(6, 122, 60, 10, &sub_vram);
    btnenvsetsuspoint->setCaption("set sus");
    btnenvsetsuspoint->registerPushCallback(envSetSustainPoint);
    
    cbsusenabled = new CheckBox(6, 132, 60, 10, &sub_vram, true, false);
    cbsusenabled->setCaption("sus on");
    cbsusenabled->registerToggleCallback(envToggleSustainEnabled);
    
		tbmapsamples = new ToggleButton(5, 125, 80, 12, &sub_vram, false);
		tbmapsamples->setCaption("map samples");
		tbmapsamples->registerToggleCallback(toggleMapSamples);

		tabbox->registerWidget(btnaddenvpoint, 0, 3);
		tabbox->registerWidget(btndelenvpoint, 0, 3);
		tabbox->registerWidget(btnenvzoomin, 0, 3);
		tabbox->registerWidget(btnenvzoomout, 0, 3);
		tabbox->registerWidget(btnenvdrawmode, 0, 3);
		tabbox->registerWidget(btnenvsetsuspoint, 0, 3);
		tabbox->registerWidget(cbsusenabled, 0, 3);
		tabbox->registerWidget(cbvolenvenabled, 0, 3);
		tabbox->registerWidget(volenvedit, 0, 3);
		tabbox->registerWidget(tbmapsamples, 0, 3);
	// </Instruments Gui>

	// <Settings Gui>
		gbhandedness = new GroupBox(5, 23, 80, 25, &sub_vram);
		gbhandedness->setText("handedness");

		rbghandedness = new RadioButton::RadioButtonGroup();
		rblefthanded  = new RadioButton(7 , 35, 35, 14, &sub_vram, rbghandedness);
		rblefthanded->setCaption("left");
		rbrighthanded = new RadioButton(42, 35, 35, 14, &sub_vram, rbghandedness);
		rbrighthanded->setCaption("right");
		rbghandedness->setActive(1);
		rbghandedness->registerChangeCallback(handleHandednessChange);

#if defined(MIDI) || defined(SHOW_ALL_SETTINGS)
		gbdsmw = new GroupBox(5, 55, 80, 54, &sub_vram);
		gbdsmw->setText("dsmidi");

		btndsmwtoggleconnect = new Button(10, 67, 71, 14, &sub_vram);
		btndsmwtoggleconnect->setCaption("connect");

		cbdsmwsend = new CheckBox(7, 83, 40, 14, &sub_vram, true, true);
		cbdsmwsend->setCaption("send");

		cbdsmwrecv = new CheckBox(7, 97, 40, 14, &sub_vram, true, true);
		cbdsmwrecv->setCaption("receive");
#endif

		gboutput = new GroupBox(89, 23, 40, 34, &sub_vram);
		gboutput->setText("out");

		rbgoutput = new RadioButton::RadioButtonGroup();
		rboutputmono = new RadioButton(91, 33, 36, 14, &sub_vram, rbgoutput);
		rboutputmono->setCaption("1ch");
		rboutputstereo = new RadioButton(91, 47, 36, 14, &sub_vram, rbgoutput);
		rboutputstereo->setCaption("2ch");
		rbgoutput->setActive(1);
		rbgoutput->registerChangeCallback(handleOutputModeChange);

		gblinesbeat = new GroupBox(89, 62, 40, 28, &sub_vram);
		gblinesbeat->setText("l/b");
		nblinesbeat = new NumberBox(93, 72, 32, 17, &sub_vram, settings->getLinesPerBeat(), 1, 64);
		nblinesbeat->registerChangeCallback(handleLinesBeatChange);

#if !defined(SHOW_ALL_SETTINGS)
		if (isDSiMode())
#endif
		{
			gbfreq = new GroupBox(89, 95, 40, 34, &sub_vram);
			gbfreq->setText("freq");

			rbgfreq = new RadioButton::RadioButtonGroup();
			rbfreq32 = new RadioButton(91, 105, 36, 14, &sub_vram, rbgfreq);
			rbfreq32->setCaption("32k");
			rbfreq47 = new RadioButton(91, 119, 36, 14, &sub_vram, rbgfreq);
			rbfreq47->setCaption("47k");
			rbgfreq->setActive(1);
			rbgfreq->registerChangeCallback(handleOutputFreqChange);
		}

		btnconfigsave = new Button(97, 135, 40, 14, &sub_vram);
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
		tabbox->registerWidget(rboutputmono, 0, 4);
		tabbox->registerWidget(rboutputstereo, 0, 4);
		tabbox->registerWidget(gboutput, 0, 4);
		tabbox->registerWidget(nblinesbeat, 0, 4);
		tabbox->registerWidget(gblinesbeat, 0, 4);
#if !defined(SHOW_ALL_SETTINGS)
		if (isDSiMode())
#endif
		{
			tabbox->registerWidget(rbfreq32, 0, 4);
			tabbox->registerWidget(rbfreq47, 0, 4);
			tabbox->registerWidget(gbfreq, 0, 4);
		}
	// </Settings Gui>

	lbinstruments = new ListBox(141, 33, 114, 89, &sub_vram, MAX_INSTRUMENTS, true, true, false);

	lbsamples = new ListBox(141, 101, 114, 23, &sub_vram, MAX_INSTRUMENT_SAMPLES, true, false, true);

	buttonswitchsub    = new BitButton(233, 1  , 21, 21, &sub_vram, icon_flp_raw, 18, 18);
	buttonplay         = new BitButton(180, 4  , 23, 15, &sub_vram, icon_play_raw, 12, 12, 5, 0, true);
	buttonpause        = new BitButton(180, 4  , 23, 15, &sub_vram, icon_pause_raw, 12, 12, 5, 0, false);
	buttonstop         = new BitButton(204, 4  , 23, 15, &sub_vram, icon_stop_raw, 12, 12, 5, 0);

	buttonundo         = new BitButton(226, 127, 14, 12, &sub_vram, icon_undo_raw, 8, 8, 3, 2);
	buttonredo         = new BitButton(226 + 15, 127, 14, 12, &sub_vram, icon_redo_raw, 8, 8, 3, 2);
	buttoninsnote2     = new Button(225, 140, 30, 12, &sub_vram);
	buttondelnote2     = new Button(225, 153, 30, 12, &sub_vram);
	buttonemptynote    = new Button(225, 166, 30, 12, &sub_vram);
	buttonstopnote     = new Button(225, 179, 30, 12, &sub_vram);
	buttonrenamesample = new Button(141, 125 , 23, 12, &sub_vram, false);
	buttonrenameinst   = new Button(141, 20 , 23, 12, &sub_vram);

	tbmultisample      = new ToggleButton(165, 21, 10, 10, &sub_vram);

	buttonundo->registerPushCallback(undoOp);
	buttonredo->registerPushCallback(redoOp);

	cbscrolllock = new CheckBox(178, 19, 30, 12, &sub_vram, true, false, true);
	cbscrolllock->setCaption("scrl lock");
	cbscrolllock->registerToggleCallback(handleToggleScrollLock);

	tbrecord = new ToggleButton(140, 136, 16, 16, &sub_vram);
	tbrecord->setBitmap(icon_record_raw, 12, 12);
	tbrecord->registerToggleCallback(setRecordMode);
	tbrecord->setColorOff(RGB15(18, 0, 0) | BIT(15));

	labeladd = new Label(182, 126, 22, 12, &sub_vram, false, true);
	labeladd->setCaption("add");
	labeloct = new Label(206, 126, 25, 12, &sub_vram, false, true);
	labeloct->setCaption("oct");
	numberboxadd    = new NumberBox(185, 135, 18, 17, &sub_vram, state->add, 0, 8, 1);
	numberboxoctave = new NumberBox(206, 135, 18, 17, &sub_vram, state->basenote/12, 0, 6, 1);

	buttonswitchsub->registerPushCallback(switchScreens);
	buttonplay->registerPushCallback(startPlay);
	buttonstop->registerPushCallback(stopPlay);
	buttonpause->registerPushCallback(pausePlay);

	buttoninsnote2->registerPushCallback(insNote);
	buttondelnote2->registerPushCallback(delNote);
	buttonemptynote->registerPushCallback(emptyNoteStroke);
	buttonstopnote->registerPushCallback(stopNoteStroke);
	buttonrenameinst->registerPushCallback(showTypewriterForInstRename);
	buttonrenamesample->registerPushCallback(showTypewriterForSampleRename);

	tbmultisample->registerToggleCallback(handleToggleMultiSample);

	numberboxadd->registerChangeCallback(changeAdd);
	numberboxoctave->registerChangeCallback(changeOctave);

	lbinstruments->registerChangeCallback(handleInstChange);
	lbsamples->registerChangeCallback(handleSampleChange);

	buttoninsnote2->setCaption("ins");
	buttondelnote2->setCaption("del");
	buttonemptynote->setCaption("clr");
	buttonstopnote->setCaption("--");
	buttonrenameinst->setCaption("ren");
	buttonrenamesample->setCaption("ren");

	tbmultisample->setCaption("+");

	// <Main Screen>
		buttonswitchmain = new BitButton(233, 1  , 21, 21, &main_vram_back, icon_flp_raw, 18, 18);
		buttonswitchmain->registerPushCallback(switchScreens);

		labelmute = new Label(226, 23, 32, 8, &main_vram_back, false, true);
		labelmute->setCaption("mute");

		buttonunmuteall = new Button(225, 32, 30, 12, &main_vram_back);
		buttonunmuteall->setCaption("none");

		labelnotevol = new Label(230, 44, 23, 10, &main_vram_back, false, true);
		labelnotevol->setCaption("vol");

		nsnotevolume	 = new NumberSlider(225, 54, 30, 17, &main_vram_back, 127, 0, 127, true, true);
		nsnotevolume->registerPostChangeCallback(handleNoteVolumeChanged);

		buttonsetnotevol = new Button(225, 70, 30, 12, &main_vram_back);
		buttonsetnotevol->setCaption("set");
		buttonsetnotevol->registerPushCallback(handleSetNoteVol);

#ifdef ENABLE_EFFECT_MENU
		cbtoggleeffects = new CheckBox(195, 32, 30, 12, &main_vram_back, true, true, true);
		cbtoggleeffects->setCaption("fx");
		cbtoggleeffects->registerToggleCallback(handleToggleEffectsVisibility);

		labeleffectcmd = new Label(200, 44, 23, 10, &main_vram_back, false, true);
		labeleffectcmd->setCaption("cmd");

		nseffectcmd	= new NumberSlider(196, 54, 28, 17, &main_vram_back, 0, -1, 26, true, true);
		nseffectcmd->registerPostChangeCallback(handleEffectCommandChanged);

		buttonseteffectcmd = new Button(196, 70, 28, 12, &main_vram_back);
		buttonseteffectcmd->setCaption("set");
		buttonseteffectcmd->registerPushCallback(handleSetEffectCommand);

		labeleffectpar = new Label(200, 84, 23, 10, &main_vram_back, false, true);
		labeleffectpar->setCaption("val");

		nseffectpar	= new NumberSlider(196, 94, 28, 17, &main_vram_back, 0, 0, 255, true, true);
		nseffectpar->registerPostChangeCallback(handleEffectParamChanged);

		buttonseteffectpar = new Button(196, 110, 28, 12, &main_vram_back);
		buttonseteffectpar->setCaption("set");
		buttonseteffectpar->registerPushCallback(handleSetEffectParam);
#endif

		//buttoncut         = new BitButton(232,  52, 22, 21, &main_vram_back, icon_cut_raw, 16, 16, 3, 2);
		//buttoncopy        = new BitButton(232,  74, 22, 21, &main_vram_back, icon_copy_raw, 16, 16, 3, 3);
		//buttonpaste       = new BitButton(232,  96, 22, 21, &main_vram_back, icon_paste_raw, 16, 16, 3, 3);

		buttoncut         = new Button(225,  86, 30, 12, &main_vram_back);
		buttoncopy        = new Button(225,  99, 30, 12, &main_vram_back);
		buttonpaste       = new Button(225, 112, 30, 12, &main_vram_back);

		buttoncut->setCaption("cut");
		buttoncopy->setCaption("cp");
		buttonpaste->setCaption("pst");

		buttoncolselect   = new Button(225, 125, 30, 12, &main_vram_back);
		buttoninsnote     = new Button(225, 140, 30, 12, &main_vram_back);
		buttondelnote     = new Button(225, 153, 30, 12, &main_vram_back);
		buttonemptynote2  = new Button(225, 166, 30, 12, &main_vram_back);
		buttonstopnote2   = new Button(225, 179, 30, 12, &main_vram_back);

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

		pv = new PatternView(0, 0, 200, 192, &main_vram_back, state);
		pv->setSong(song);
		pv->registerMuteCallback(handleMuteChannelsChanged);

		gui->registerWidget(labelmute, 0, MAIN_SCREEN);
		gui->registerWidget(buttonunmuteall, 0, MAIN_SCREEN);
		gui->registerWidget(buttonswitchmain, 0, MAIN_SCREEN);
		gui->registerWidget(labelnotevol, 0, MAIN_SCREEN);
		gui->registerWidget(nsnotevolume, 0, MAIN_SCREEN);
		gui->registerWidget(buttonsetnotevol, 0, MAIN_SCREEN);
#ifdef ENABLE_EFFECT_MENU
		gui->registerWidget(cbtoggleeffects, 0, MAIN_SCREEN);
		gui->registerWidget(labeleffectcmd, 0, MAIN_SCREEN);
		gui->registerWidget(nseffectcmd, 0, MAIN_SCREEN);
		gui->registerWidget(buttonseteffectcmd, 0, MAIN_SCREEN);
		gui->registerWidget(labeleffectpar, 0, MAIN_SCREEN);
		gui->registerWidget(nseffectpar, 0, MAIN_SCREEN);
		gui->registerWidget(buttonseteffectpar, 0, MAIN_SCREEN);
#else
		pv->toggleEffectsVisibility(false);
#endif
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

	gui->registerWidget(buttonswitchsub, 0, SUB_SCREEN);
	gui->registerWidget(buttonplay, 0, SUB_SCREEN);
	gui->registerWidget(buttonstop, 0, SUB_SCREEN);
	gui->registerWidget(buttonpause, 0, SUB_SCREEN);
	gui->registerWidget(buttonemptynote, 0, SUB_SCREEN);
	gui->registerWidget(buttonundo, 0, SUB_SCREEN);
	gui->registerWidget(buttonredo, 0, SUB_SCREEN);
	gui->registerWidget(buttoninsnote2, 0, SUB_SCREEN);
	gui->registerWidget(buttondelnote2, 0, SUB_SCREEN);
	gui->registerWidget(buttonrenameinst, 0, SUB_SCREEN);
	gui->registerWidget(buttonrenamesample, 0, SUB_SCREEN);
	gui->registerWidget(tbmultisample, 0, SUB_SCREEN);
	gui->registerWidget(cbscrolllock, 0, SUB_SCREEN);
	gui->registerWidget(numberboxadd, 0, SUB_SCREEN);
	gui->registerWidget(numberboxoctave, 0, SUB_SCREEN);
	gui->registerWidget(labeladd, 0, SUB_SCREEN);
	gui->registerWidget(labeloct, 0, SUB_SCREEN);
	gui->registerWidget(kb, 0, SUB_SCREEN);
	gui->registerWidget(buttonstopnote, 0, SUB_SCREEN);
	gui->registerWidget(tbrecord, 0, SUB_SCREEN);
	gui->registerWidget(pixmaplogo, 0, SUB_SCREEN);
	gui->registerWidget(tabbox, 0, SUB_SCREEN);
	gui->registerWidget(lbinstruments, 0, SUB_SCREEN);
	gui->registerWidget(lbsamples, 0, SUB_SCREEN);

	gui->revealAll();


	actionBufferChangeCallback();
	updateTempoAndBpm();
	handleLinesBeatChange(settings->getLinesPerBeat());


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

// Update the state for certain keypresses
void handleButtons(u16 buttons, u16 buttonsheld)
{
	u16 ptnlen = song->getPatternLength(song->getPotEntry(state->potpos));

	if(!(buttonsheld & mykey_R))
	{
		if(buttons & mykey_UP)
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
			redraw_main_requested = true;

		}
		else if(buttons & mykey_DOWN)
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
			redraw_main_requested = true;
		}
	}

	if((buttons & mykey_LEFT)&&(!typewriter_active))
	{
		if(state->channel>0) {
			state->channel--;
			pv->updateSelection();
			redraw_main_requested = true;
		}
	}
	else if((buttons & mykey_RIGHT)&&(!typewriter_active))
	{
		if(state->channel < song->getChannels()-1)
		{
			state->channel++;
			pv->updateSelection();
			redraw_main_requested = true;
		}
	}
	else if(buttons & KEY_START)
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
	else if(buttons & KEY_SELECT)
	{
#ifdef DEBUG
		PrintFreeMem();
		printMallInfo();
#else
		stopPlay();
#endif
	}
#ifdef DEBUG
	/*else if(buttons & mykey_Y) {
		saveScreenshot();
	} else if(buttons & mykey_R) {
		dumpSample();
	}
	*/
#endif
}

void VblankHandler(void)
{
	// Check input
	scanKeys();
	u16 keysdown = keysDown() | (keysDownRepeat() & keys_that_are_repeated);
	u16 keysup = keysUp();
	u16 keysheld = keysHeld();
	touchRead(&touch);

	if(keysdown & KEY_TOUCH)
	{
		gui->penDown(touch.px, touch.py);
		redraw_main_requested = true;
	}

	if(keysup & KEY_TOUCH)
	{
		gui->penUp(touch.px, touch.py);
		lastx = -255;
		lasty = -255;
	}

	if( (keysheld & KEY_TOUCH) && ( (abs(touch.px - lastx)>0) || (abs(touch.py - lasty)>0) ) ) // PenMove
	{
		gui->penMove(touch.px, touch.py);
		lastx = touch.px;
		lasty = touch.py;
		if(gui->getActiveScreen() == MAIN_SCREEN)
			redraw_main_requested = true;
	}

	if(keysheld & mykey_R)
	{
		if(keysheld & mykey_DOWN)
			move_to_bottom();
		else if(keysheld & mykey_UP)
			move_to_top();
	}

	if(keysdown & ~KEY_TOUCH)
	{
		if((keysdown & mykey_X)||(keysdown & mykey_L)) {
			switchScreens();
		}

		if(keysdown & mykey_B) {
			fastscroll = true;
		}

		gui->buttonPress(keysdown);
		handleButtons(keysdown, keysheld);
		pv->pleaseDraw();
	}

	if(keysup)
	{
		gui->buttonRelease(keysup);

		if(keysup & mykey_B)
			fastscroll = false;
	}

	// Easy Piano pak handling logic
	if (pianoIsInserted())
	{
		pianoScanKeys();
		u16 piano_down = pianoKeysDown();

		for (u16 i = 0; i < 15; i++) {
			if (i > 10 && i < 13)
				continue;
			
			u16 note_val = (i >= 13) ? (i - 2) : i;

			if (piano_down & (1 << i)) {
				handleNoteStroke(note_val);
				handleNoteRelease(note_val, false);
			}
 		}
	}

	// Constantly update pattern view while playing
	if(redraw_main_requested)
	{
		redraw_main_requested = false;
		drawMainScreen();
	}

	frame = (frame + 1) % 2;
}

extern "C" void debug_print_stub(char *string)
{
	iprintf(string);
}

void fadeIn(void)
{
	for(int i=-16; i <= 0; ++i)
	{
	    setBrightness(3, i);
		swiWaitForVBlank();
	}
}

#ifdef DEBUG
void saveScreenshot(void)
{
	debugprintf("Saving screenshot\n");
	u8 *screenbuf = (u8*)malloc(256*192*3*2);
	u8 *screenptr = screenbuf;

	u16 col;
	for(u32 i=0;i<192*256;++i) {
		col = main_vram_front[i];
		*(screenptr++) = (col & 0x1F) << 3;
		col >>= 5;
		*(screenptr++) = (col & 0x1F) << 3;
		col >>= 5;
		*(screenptr++) = (col & 0x1F) << 3;
	}
	for(u32 i=0;i<192*256;++i) {
		col = sub_vram[i];
		*(screenptr++) = (col & 0x1F) << 3;
		col >>= 5;
		*(screenptr++) = (col & 0x1F) << 3;
		col >>= 5;
		*(screenptr++) = (col & 0x1F) << 3;
	}

	static u8 filenr = 0;
	char filename[255] = {0};
	siprintf(filename, "scr%02d.rgb", filenr);

	FILE *fileh;
	fileh = fopen(filename, "w");
	fwrite(screenbuf, 256*192*3*2, 1, fileh);
	fclose(fileh);

	free(screenbuf);
	debugprintf("saved\n");

	filenr++;
}

void dumpSample(void)
{
	static u8 smpfilenr = 0;
	char filename[255] = {0};
	siprintf(filename, "smp%02d.raw", smpfilenr);

	Instrument *inst = song->getInstrument(state->instrument);
	if(inst==0) return;
	Sample *smp = inst->getSample(state->sample);
	if(smp==0) return;
	void *data = smp->getData();
	u32 size = smp->getSize();

	debugprintf("saving sample\n");

	FILE *fileh;
	fileh = fopen(filename, "w");
	fwrite(data, size, 1, fileh);
	fclose(fileh);

	debugprintf("saved\n");
}
#endif

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

	// Hide everything
#ifndef DEBUG
	setBrightness(3, 16);
#endif

	powerOn(POWER_ALL_2D);

	// Adjust screens so that the main screen is the top screen
	lcdMainOnTop();

	// Main screen: Text and double buffer ERB
	videoSetMode(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);

	// Sub screen: Keyboard tiles, Typewriter tiles and ERB
	videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE | DISPLAY_BG2_ACTIVE);

	vramSetPrimaryBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000,
	           VRAM_C_SUB_BG_0x06200000 , VRAM_D_LCD);

	// SUB_BG0 for Piano Tiles
	videoBgEnableSub(0);
	int piano_bg = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 1, 0);
	bgSetScroll(piano_bg, 0, 0);
	bgSetPriority(piano_bg, 2);

	// SUB_BG1 for Typewriter Tiles
	videoBgEnableSub(1);
	int typewriter_bg = bgInitSub(1, BgType_Text4bpp, BgSize_T_256x256, 12, 1);
	bgSetPriority(typewriter_bg, 0);

#ifdef DEBUG
	u8 text_priority = 0;
	u8 bg_priority = 1;
#else
	u8 text_priority = 1;
	u8 bg_priority = 0;
#endif

	// Pattern view
	int ptn_bg = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 2, 0);
	bgSetPriority(ptn_bg, bg_priority);

	// Sub screen framebuffer
	int sub_bg = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 2, 0);
	bgSetPriority(sub_bg, 0);

	// Special effects
	REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG2 | BLEND_SRC_BG0;
	REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG2 | BLEND_SRC_BG0;

	// Setup text
	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 4, 0, true, true);
	bgSetPriority(0, text_priority);

	bgUpdate();

	// Set draw loactions for the ERBs
	main_vram_front = (uint16*)BG_BMP_RAM(2);
	main_vram_back = (uint16*)BG_BMP_RAM(8);
	sub_vram  = (uint16*)BG_BMP_RAM_SUB(2);

	// Clear tile mem
	dmaFillWords(0, BG_BMP_RAM_SUB(0), 32*1024);

#ifdef USE_FAT
	bool fat_success = fatInitDefault();
#else
    bool fat_success = false;
#endif

	// parse argv[0], if present
	if (argc >= 1 && argv != NULL && argv[0] != NULL) {
		char *path_split = strrchr(argv[0], '/');
		if (path_split != NULL && (path_split - argv[0]) >= 1) {
			int launch_path_len = path_split - argv[0];
			launch_path = (char*) malloc(launch_path_len + 1);
			strncpy(launch_path, argv[0], launch_path_len);
			launch_path[launch_path_len] = '\0';

			if (!dirExists(launch_path)) {
				free(launch_path);
				launch_path = NULL;
			}
		}
	}

	state = new State();

	settings = new Settings(launch_path, fat_success);

	clearMainScreen();
	clearSubScreen();

	// Key repeat handling: enable repeat
	keysSetRepeat(REPEAT_START_DELAY, 60 / REPEAT_FREQ);

	// Init interprocessor communication
	CommandInit();
	RegisterRowCallback(handleRowChangeFromSong);
	RegisterStopCallback(handleStop);
	RegisterPlaySampleFinishedCallback(handlePreviewSampleFinished);
	RegisterPotPosChangeCallback(handlePotPosChangeFromSong);

	setupSong();

	CommandSetSong(song);

	setupGUI(fat_success);
	action_buffer->register_change_callback(actionBufferChangeCallback);

	applySettings();
	setSong(song);

#ifndef DEBUG
	fadeIn();
#endif

#ifdef USE_FAT
	if(!fat_success)
		showMessage("dldi init failed", true);
#endif

#ifdef DEBUG
	debugprintf("NitroTracker debug build.\nBuilt %s %s\n<Start> clears messages.\n", __DATE__, __TIME__);
#endif

	while(!exit_requested)
	{
		VblankHandler();

#ifdef MIDI
		if( state->dsmi_connected ) {
			handleDSMWRecv();
			dsmi_task();
		}
#endif

#ifdef DEBUG
        if(keysHeld() == (KEY_START | KEY_SELECT | KEY_L | KEY_R)) {
            exit_requested = true;
        }
#endif
		swiWaitForVBlank();
	}

	if (launch_path) free(launch_path);

	return 0;
}
