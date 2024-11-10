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

#include "settings.h"

#include <stdio.h>
#include <string.h>

#include "tools.h"

#define SETTINGS_DEFAULT_DATA_DIR "/data/NitroTracker"
#define SETTINGS_CONFIG_FILENAME "NitroTracker.conf"

/* ===================== PUBLIC ===================== */

Settings::Settings(char *launch_path, bool use_fat)
: handedness(RIGHT_HANDED),
sample_preview(true),
stereo_output(true),
theme(new Theme()),
fat(use_fat), changed(false)
{
	songpath[SETTINGS_FILENAME_LEN] = '\0';
	samplepath[SETTINGS_FILENAME_LEN] = '\0';

	// might never have snprintf called
	configpath[0] = '\0';
	configpath[SETTINGS_FILENAME_LEN] = '\0';

	sniprintf(songpath, SETTINGS_FILENAME_LEN, "%s/", launch_path != NULL ? launch_path : "");
	sniprintf(samplepath, SETTINGS_FILENAME_LEN, "%s/", launch_path != NULL ? launch_path : "");

	if(fat == true)
	{
		if (launch_path == NULL)
		{
			dirCreate("/data");
			dirCreate("/data/NitroTracker");
		}

		sniprintf(configpath, SETTINGS_FILENAME_LEN, "%s/%s",
			launch_path != NULL ? launch_path : SETTINGS_DEFAULT_DATA_DIR,
			SETTINGS_CONFIG_FILENAME);

		// Check if the config file exists and, if not, create it
		FILE *conf = fopen(configpath, "r");
		if(conf == NULL)
		{
			write();
		}
		else
		{
			char hstring[20], prevstring[20];

			fseek(conf, 0, SEEK_END);
			u32 conf_filesize = ftell(conf);
			fseek(conf, 0, SEEK_SET);

			char *confstr = (char*)calloc(1, conf_filesize);
			fread(confstr, conf_filesize, 1, conf);
			fclose(conf);

			getConfigValue(confstr, "Samplepath", samplepath, SETTINGS_FILENAME_LEN, NULL);
			getConfigValue(confstr, "Songpath", songpath, SETTINGS_FILENAME_LEN, NULL);

			getConfigValue(confstr, "Handedness", hstring, 20, "Right");
			handedness = stringToHandedness(hstring);

			getConfigValue(confstr, "Sample Preview", prevstring, 20, "True");
			sample_preview = stringToBool(prevstring);

			getConfigValue(confstr, "Stereo Output", prevstring, 20, "True");
			stereo_output = stringToBool(prevstring);

			getConfigValue(confstr, "47kHz Output", prevstring, 20, "False");
			freq_47khz = stringToBool(prevstring);

			getConfigValue(confstr, "Lines Per Beat", prevstring, 20, "8");
			lines_per_beat = strtoul(prevstring, NULL, 10);
			if (!lines_per_beat || lines_per_beat > 64)
				lines_per_beat = 8;

			free(confstr);
		}
	}
}

Handedness Settings::getHandedness(void)
{
	return handedness;
}

void Settings::setHandedness(Handedness handedness_)
{
	handedness = handedness_;
	changed = true;
}

bool Settings::getSamplePreview(void)
{
	return sample_preview;
}

void Settings::setSamplePreview(bool sample_preview_)
{
	sample_preview =  sample_preview_;
	changed = true;
}

bool Settings::getStereoOutput(void)
{
	return stereo_output;
}

void Settings::setStereoOutput(bool stereo_output_)
{
	stereo_output =  stereo_output_;
	changed = true;
}

bool Settings::getFreq47kHz(void)
{
	return freq_47khz;
}

void Settings::setFreq47kHz(bool freq_47khz_)
{
	freq_47khz =  freq_47khz_;
	changed = true;
}

u8 Settings::getLinesPerBeat(void)
{
	return lines_per_beat;
}

void Settings::setLinesPerBeat(u8 lines_per_beat_)
{
	lines_per_beat =  lines_per_beat_;
	changed = true;
}

Theme *Settings::getTheme(void)
{
	return theme;
}

void Settings::setTheme(Theme *theme_)
{
	theme = theme_;
	changed = true;
}

char *Settings::getSongPath(void)
{
    if(!dirExists(songpath)) {
        strncpy(songpath, "/", SETTINGS_FILENAME_LEN);
    }
    return songpath;
}

void Settings::setSongPath(const char* songpath_)
{
	strncpy(songpath, songpath_, SETTINGS_FILENAME_LEN);
	songpath[SETTINGS_FILENAME_LEN] = '\0';
	changed = true;
}

char *Settings::getSamplePath(void)
{
    if(!dirExists(samplepath)) {
        strncpy(samplepath, "/", SETTINGS_FILENAME_LEN);
    }
	return samplepath;
}

void Settings::setSamplePath(const char* samplepath_)
{
	strncpy(samplepath, samplepath_, SETTINGS_FILENAME_LEN);
	samplepath[SETTINGS_FILENAME_LEN] = '\0';
	changed = true;
}

bool Settings::writeIfChanged(void)
{
	if (!changed)
		return false;
	changed = false;
	return write();
}

/* ===================== PRIVATE ===================== */

bool Settings::write(void)
{
	if (strlen(configpath) <= 0)
		return false;

	FILE *conf = fopen(configpath, "w");
	if(conf == NULL)
	{
		debugprintf("error opening config for writing\n");
		return false;
	}

	char hstring[20], prevstring[20], stereostring[20], freqstring[20];
	handednessToString(hstring);
	boolToString(sample_preview, prevstring);
	boolToString(stereo_output, stereostring);
	boolToString(freq_47khz, freqstring);
	fiprintf(conf, "Samplepath = %s\nSongpath = %s\nHandedness = %s\nSample Preview = %s\nStereo Output = %s\n47kHz Output = %s\nLines Per Beat = %d\n",
			samplepath, songpath, hstring, prevstring, stereostring, freqstring, lines_per_beat);
	fclose(conf);
	return true;
}

void Settings::handednessToString(char *str)
{
	if(handedness == LEFT_HANDED)
		strcpy(str, "Left");
	else if(handedness == RIGHT_HANDED)
		strcpy(str, "Right");
}

Handedness Settings::stringToHandedness(char *str)
{
	if(strcasecmp(str, "Left") == 0)
		return LEFT_HANDED;
	else
		return RIGHT_HANDED;
}

void Settings::boolToString(bool b, char *str)
{
	if(b == true)
		strcpy(str, "True");
	else
		strcpy(str, "False");
}

bool Settings::stringToBool(char *str)
{
	if(strcasecmp(str, "True") == 0)
		return true;
	else
		return false;
}

static bool setDefaultConfigValue(char *value, const char *defvalue, size_t maxlen)
{
	if (defvalue != NULL)
	{
		strncpy(value, defvalue, maxlen);
		value[maxlen - 1] = 0;
	}
	return false;
}

bool Settings::getConfigValue(char *config, const char *attribute, char *value, size_t maxlen, const char *defvalue)
{
	// Oh goodness, this is going to be some badass C code
	char *attrptr = 0;
	char *valstart = 0;
	char *valend = 0;

	// Find the attribute string
	attrptr = strstr(config, attribute);
	if(attrptr == NULL)
		return setDefaultConfigValue(value, defvalue, maxlen);

	// Find the '=' sign after that
	valstart = strchr(attrptr, '=');
	if(valstart == NULL)
		return setDefaultConfigValue(value, defvalue, maxlen);

	// Skip forward to the next non-space character
	valstart++;
	while(*valstart == ' ')
		valstart++;

	// Find the end of the line
	valend = strchr(attrptr, '\n');
	if(valend == NULL)
		return setDefaultConfigValue(value, defvalue, maxlen);

	// Skip backward to the last non-space character
	valend--;
	while(*valend == ' ')
		valend--;

	size_t vallen = valend - valstart + 1;
	size_t len = std::min(maxlen-1, vallen);
	strncpy(value, valstart, len);
	value[len] = 0;

	//debugprintf("'%s' : '%s'\n", attribute, value);

	return true;
}
