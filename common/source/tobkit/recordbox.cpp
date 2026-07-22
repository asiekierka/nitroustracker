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

#include "recordbox.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ntxm/fifocommand.h"
#include "ntxm/instrument.h"
#include "ntxm/ntxmtools.h"
#include "tools.h"

using namespace tobkit;

/* ===================== PUBLIC ===================== */

// Constructor sets base variables
RecordBox::RecordBox(Screen *_screen, void (*_onOk)(void),
                     void (*_onCancel)(void), Sample *_sample,
                     Instrument *_instrument, u8 _smpidx)
    : Widget((_screen->getWidth() - RECORDBOX_WIDTH) / 2,
             (_screen->getHeight() - RECORDBOX_HEIGHT) / 2, RECORDBOX_WIDTH,
             RECORDBOX_HEIGHT, _screen, true),
      recording(false), btndown(false), onOk(_onOk), onCancel(_onCancel),
      sample(_sample), instrument(_instrument), smpidx(_smpidx),
      sound_data(NULL)
{
	title = "sample recorder";

	const char *msg = "hold down a";
	u8 msgwidth = getStringWidth(msg);
	labelmsg = new Label(x + (RECORDBOX_WIDTH - msgwidth) / 2, y + 18,
	                     msgwidth + 5, 12, _screen, false);
	labelmsg->setCaption(msg);

	msg = "or press b";
	msgwidth = getStringWidth(msg);
	labelmsg2 = new Label(x + (RECORDBOX_WIDTH - msgwidth) / 2, y + 30,
	                      msgwidth + 5, 12, _screen, false);
	labelmsg2->setCaption(msg);

	msg = "recording";
	msgwidth = getStringWidth(msg);
	labelrec = new Label(x + (RECORDBOX_WIDTH - msgwidth) / 2, y + 33,
	                     msgwidth + 5, 12, _screen, false, false, true);
	labelrec->setCaption(msg);
	labelrec->hide();

	buttoncancel =
	    new Button(x + (RECORDBOX_WIDTH - 50) / 2, y + 44, 50, 14, _screen);
	buttoncancel->setCaption("cancel");
	buttoncancel->registerPushCallback(_onCancel);

#ifdef NT_PLATFORM_3DS
	sound_data = (u16 *)ntxm_cmemalign(0x1000, RECORDBOX_SOUNDDATA_SIZE);
	micInit((u8 *)sound_data, RECORDBOX_SOUNDDATA_SIZE);
#endif
}

RecordBox::~RecordBox(void)
{
#ifdef NT_PLATFORM_3DS
	micExit();
#endif
	if (sound_data)
		ntxm_free(sound_data);

	delete labelmsg;
	delete labelmsg2;
	delete labelrec;
	delete buttoncancel;
}

// Drawing request
void RecordBox::pleaseDraw(void)
{
	draw();
}

// Event calls
void RecordBox::penDown(u16 px, u16 py)
{
	u16 bx, by, bw, bh;

	buttoncancel->getPos(&bx, &by, &bw, &bh);
	if ((px >= bx) && (px <= bx + bw) && (py >= by) && (py <= by + bh)) {
		buttoncancel->penDown(px, py);
		btndown = true;
	}
}

void RecordBox::penUp(u16 px, u16 py)
{
	if (btndown == true) {
		btndown = false;
		buttoncancel->penUp(px, py);
	}
}

void RecordBox::buttonPress(u16 button)
{
	if (button & KEY_A)
		startRecording();
	else if (button & KEY_B) {
		if (!recording) {
			startRecording();
		} else {
			stopRecording();
		}
	}
}

void RecordBox::buttonRelease(u16 button)
{
	if ((button & KEY_A) && recording)
		stopRecording();
}

Sample *RecordBox::getSample(void)
{
	return sample;
}

void RecordBox::setTheme(Theme *theme_, u16 bgcolor_)
{
	theme = theme_;
	bgcolor = bgcolor_;
	labelmsg->setTheme(theme, theme->col_light_bg);
	labelmsg2->setTheme(theme, theme->col_light_bg);
	labelrec->setTheme(theme, theme->col_light_bg);
	buttoncancel->setTheme(theme, theme->col_light_bg);
	labelmsg->reveal();
	labelmsg2->reveal();
	labelrec->reveal();
	buttoncancel->reveal();
}

/* ===================== PRIVATE ===================== */

void RecordBox::draw(void)
{
	drawGradient(theme->col_dark_ctrl, theme->col_light_ctrl, 1, 1, width - 2,
	             15);
	drawHLine(1, 16, width - 2, theme->col_outline);
	if (recording == true) {
		drawFullBox(1, 17, width - 2, RECORDBOX_HEIGHT - 18, theme->col_signal);
		labelmsg->setTheme(theme, theme->col_signal);
		labelmsg2->setTheme(theme, theme->col_signal);
		buttoncancel->setTheme(theme, theme->col_signal);
		labelrec->show();
		labelmsg->hide();
		labelmsg2->hide();
		buttoncancel->hide();
	} else {
		drawFullBox(1, 17, width - 2, RECORDBOX_HEIGHT - 18,
		            theme->col_light_bg);
		labelrec->hide();
		labelmsg->show();
		labelmsg2->show();
		buttoncancel->show();
		buttoncancel->pleaseDraw();
	}
	drawBorder(theme->col_outline);

	u8 titlewidth = getStringWidth(title) + 5;
	drawString(title, (RECORDBOX_WIDTH - titlewidth) / 2, 2, theme->col_text,
	           titlewidth + 5);

	labelmsg->pleaseDraw();
	labelmsg2->pleaseDraw();
	labelrec->pleaseDraw();
}

bool RecordBox::startRecording(void)
{
	if (!recording) {
		// Kill and recreate the sample
		if (sample != NULL)
			instrument->setSample(smpidx, NULL); // Deletes the sample

#ifdef NT_PLATFORM_3DS
		if (R_FAILED(MICU_StartSampling(MICU_ENCODING_PCM16_SIGNED,
		                                MICU_SAMPLE_RATE_16360, 0,
		                                micGetSampleDataSize(), false)))
			return false;
#else
		if (sound_data)
			ntxm_free(sound_data);
		sound_data = (u16 *)ntxm_cmalloc(RECORDBOX_SOUNDDATA_SIZE);
		if (!sound_data)
			return false;

		// Start recording
		ntxm_flush_dcache();
#endif
		CommandStartRecording(sound_data, RECORDBOX_SOUNDDATA_SIZE);
		recording = true;

		draw();
	}
	return true;
}

void RecordBox::stopRecording()
{
	int size = CommandStopRecording();
#ifdef NT_PLATFORM_3DS
	size = micGetLastSampleOffset();
	MICU_StopSampling();
#endif
#ifdef NT_PLATFORM_NDS
	DC_InvalidateRange(sound_data, size);
#endif

	debugprintf("orig sample size %lu @ %d Hz\n", size / 2,
	            RECORDBOX_SAMPLING_FREQ);

	// Security check
	if (size < RECORDBOX_CROP_SAMPLES_END + RECORDBOX_CROP_SAMPLES_START) {
#ifndef NT_PLATFORM_3DS
		if (sound_data)
			ntxm_free(sound_data);
		sound_data = NULL;
#endif
		sample = NULL;
		onCancel();
		debugprintf("recorded data too small\n");
		return;
	}

	// Get pointer to sound data and shrink it beautiful
	s32 newsize =
	    size -
	    RECORDBOX_CROP_SAMPLES_END *
	        2; // Crop the end because it contains the clicking of the button
	if (newsize < 0)
		newsize = 0;
#ifdef NT_PLATFORM_3DS
	u16 *sample_data = (u16 *)ntxm_cmalloc(newsize);
	memcpy(sample_data, sound_data, newsize);
#else
	u16 *sample_data = (u16 *)ntxm_crealloc(sound_data, newsize);
	sound_data = NULL;
#endif

	//Cut the first few samples
	if (RECORDBOX_CROP_SAMPLES_START < newsize) {
		memmove(sample_data, sample_data + RECORDBOX_CROP_SAMPLES_START * 2,
		        newsize - RECORDBOX_CROP_SAMPLES_START * 2);
		newsize -= RECORDBOX_CROP_SAMPLES_START * 2;
		sample_data = (u16 *)ntxm_crealloc(sample_data, newsize);
	}

	// takes ownership of sound_data
	sample = new Sample(sample_data, newsize / 2, RECORDBOX_SAMPLING_FREQ);
	debugprintf("cut sample size %lu @ %d Hz\n", newsize / 2,
	            RECORDBOX_SAMPLING_FREQ);

	sample->setName("rec");

	//smp->cutSilence(); // Cut silence in the beginning (experiMENTAL!)

	recording = false;

	ntxm_flush_dcache();

	onOk();
}
