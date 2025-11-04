/*====================================================================
Copyright 2006 Tobias Weyand

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
======================================================================*/

#include "tobkit/theme.h"
#include <stdio.h>
#include <string.h>

using namespace tobkit;

Theme::Theme(char* themepath, bool use_fat) : 
	// Set default colors
	col_bg(RGB15(4, 6, 15) | BIT(15)),
	col_dark_bg(col_bg),
	col_medium_bg(RGB15(9, 11, 17) | BIT(15)),
	col_light_bg(RGB15(16, 18, 24) | BIT(15)),
	col_lighter_bg(RGB15(23, 25, 31) | BIT(15)),
	col_light_ctrl(RGB15(31, 31, 0) | BIT(15)), // RGB15(26,26,26)|BIT(15)
	col_dark_ctrl(RGB15(31, 18, 0) | BIT(15)),
	col_light_ctrl_disabled(col_light_bg),
	col_dark_ctrl_disabled(col_medium_bg),
	col_list_highlight1(RGB15(28, 15, 0) | BIT(15)),
	col_list_highlight2(RGB15(28, 28, 0) | BIT(15)),
	col_outline(RGB15(0, 0, 0) | BIT(15)),
	col_tab_outline(col_outline),
	col_sepline(RGB15(31, 31, 0) | BIT(15)),
	col_icon(RGB15(0, 0, 0) | BIT(15)),
	col_text(RGB15(0, 0, 0) | BIT(15)),
	col_signal(RGB15(31, 0, 0) | BIT(15)),
	col_signal_off(RGB15(18, 0, 0) | BIT(15)),
	col_piano_label(RGB15(0, 0, 0)),
	col_piano_label_inv(RGB15(31, 31, 31)),
	col_loop(RGB15(7, 25, 5) | BIT(15)),
	col_env_sustain(RGB15(0, 31, 0) | BIT(15)),
	col_mem_ok(RGB15(17, 24, 16) | BIT(15)),
	col_mem_warn(RGB15(31, 31, 0) | BIT(15)),
	col_mem_alert(RGB15(31, 0, 0) | BIT(15)),
	col_typewriter_cursor(RGB15(0, 0, 0) | BIT(15)),
	col_pv_lines(col_light_bg),
	col_pv_sublines(RGB15(7, 9, 17) | BIT(15)),
	col_pv_lines_record(col_dark_ctrl),
	col_pv_cb_col1(col_medium_bg),
	col_pv_cb_col2(col_light_bg),
	col_pv_cb_col1_highlight(col_list_highlight1),
	col_pv_cb_col2_highlight(col_list_highlight2),
	col_pv_left_numbers(col_list_highlight1),
	col_pv_notes(RGB15(9, 15, 31) | BIT(15)),
	col_pv_notes_dark(RGB15(0, 6, 26) | BIT(15)),
	col_pv_instr(RGB15(31, 11, 0) | BIT(15)),
	col_pv_instr_dark(RGB15(20, 6, 0) | BIT(15)),
	col_pv_volume(RGB15(0, 27, 0) | BIT(15)),
	col_pv_volume_dark(RGB15(0, 16, 0) | BIT(15)),
	col_pv_effect(RGB15(31, 12, 29) | BIT(15)),
	col_pv_effect_dark(RGB15(12, 6, 18) | BIT(15)),
	col_pv_effect_param(RGB15(30, 26, 8) | BIT(15)),
	col_pv_effect_param_dark(RGB15(9, 8, 5) | BIT(15)),
	col_pv_cb_sel_highlight(RGB15(31, 24, 0) | BIT(15)), fat(use_fat)
{
	if (fat == true && themepath != NULL)
	{
		FILE* conf = fopen(themepath, "r");
		debugprintf("loading theme '%s'\n", themepath);

		if (conf != NULL)  {
			fseek(conf, 0, SEEK_END);
			u32 conf_filesize = ftell(conf);
			fseek(conf, 0, SEEK_SET);

			char* confstr = (char*)calloc(1, conf_filesize);
			fread(confstr, conf_filesize, 1, conf);
			fclose(conf);

			u16* colscheme = (u16*)calloc(NUM_COLORS, sizeof(u16));
			bool result = parseThemeConf(confstr, colscheme);
			if (result) {
				col_bg 						= colscheme[0] | BIT(15);
				col_dark_bg 				= colscheme[1] | BIT(15);
				col_medium_bg 				= colscheme[2] | BIT(15);
				col_light_bg 				= colscheme[3] | BIT(15);
				col_lighter_bg 				= colscheme[4] | BIT(15);
				col_light_ctrl 				= colscheme[5] | BIT(15);
				col_dark_ctrl 				= colscheme[6] | BIT(15);
				col_light_ctrl_disabled 	= colscheme[7] | BIT(15);
				col_dark_ctrl_disabled 		= colscheme[8] | BIT(15);
				col_list_highlight1 		= colscheme[9] | BIT(15);
				col_list_highlight2 		= colscheme[10] | BIT(15);
				col_outline 				= colscheme[11] | BIT(15);
				col_tab_outline 			= colscheme[12] | BIT(15);
				col_sepline 				= colscheme[13] | BIT(15);
				col_icon 					= colscheme[14] | BIT(15);
				col_text 					= colscheme[15] | BIT(15);
				col_signal 					= colscheme[16] | BIT(15);
				col_signal_off 				= colscheme[17] | BIT(15);
				col_piano_label 			= colscheme[18];
				col_piano_label_inv 		= colscheme[19];
				col_loop 					= colscheme[20] | BIT(15);
				col_env_sustain 			= colscheme[21] | BIT(15);
				col_mem_ok 					= colscheme[22] | BIT(15);
				col_mem_warn 				= colscheme[23] | BIT(15);
				col_mem_alert 				= colscheme[24] | BIT(15);
				col_typewriter_cursor 		= colscheme[25] | BIT(15);
				col_pv_lines 				= colscheme[26] | BIT(15);
				col_pv_sublines 			= colscheme[27] | BIT(15);
				col_pv_lines_record 		= colscheme[28] | BIT(15);
				col_pv_cb_col1 				= colscheme[29] | BIT(15);
				col_pv_cb_col2 				= colscheme[30] | BIT(15);
				col_pv_cb_col1_highlight 	= colscheme[31] | BIT(15);
				col_pv_cb_col2_highlight 	= colscheme[32] | BIT(15);
				col_pv_left_numbers 		= colscheme[33] | BIT(15);
				col_pv_notes 				= colscheme[34] | BIT(15);
				col_pv_instr 				= colscheme[35] | BIT(15);
				col_pv_volume 				= colscheme[36] | BIT(15);
				col_pv_effect 				= colscheme[37] | BIT(15);
				col_pv_effect_param 		= colscheme[38] | BIT(15);
				col_pv_notes_dark 			= colscheme[39] | BIT(15);
				col_pv_instr_dark 			= colscheme[40] | BIT(15);
				col_pv_volume_dark 			= colscheme[41] | BIT(15);
				col_pv_effect_dark 			= colscheme[42] | BIT(15);
				col_pv_effect_param_dark 	= colscheme[43] | BIT(15);
				col_pv_cb_sel_highlight 	= colscheme[44] | BIT(15);

				debugprintf("loaded theme '%s'\n", themepath);
			}

			else
				debugprintf("failed to parse theme at %s, using default\n", themepath);

			free(colscheme);
			free(confstr);
		} else
			debugprintf("no theme found at %s, using default\n", themepath);
	}
}

/* ===================== PRIVATE ===================== */


bool Theme::stringToRGB15(char* str, u16* col)
{
	if (str == NULL)
		return false;

	int r, g, b;
	// can't allow '#' prepended as parser reads exactly 6 chars for each colour
	// int res = sscanf(str, str[0] == '#' ? "#%02x%02x%02x" : "%02x%02x%02x", &r, &g, &b);
	int res = sscanf(str, "%02x%02x%02x", &r, &g, &b);
	if (res < 3)
		return false;
	*col = (u16)RGB15(r / 8, g / 8, b / 8);
	return true;
}

// not needed as we are not writing themes currently
// void Theme::RGB15ToString(u16 col, char* str)
// {
// 	sprintf(str, "%02x%02x%02x", (col & 0x1f) * 8, ((col >> 5) & 0x1f) * 8, (col >> 10) & 0x1f * 8);
// }


bool Theme::parseThemeConf(char* str, u16* theme_cols) {
	if (str == NULL || theme_cols == NULL)
		return false;
		
	char* valstart = str;
	size_t rem = strlen(str);
	char* valend = str + rem;
	char* nextline = 0;
	for (int i = 0; i < NUM_COLORS; i++) {
		if (rem < 6)
			return false;
		size_t valsize = 0;
		nextline = strchr(valstart, '\n');
		valsize = nextline == NULL ?
			valend - valstart : nextline - valstart;
		if (valsize >= 6) {
			char val[7];
			strncpy(val, valstart, 6);
			val[6] = 0;
			u16 c;
			bool result = stringToRGB15(val, &c);
			if (!result)
				return false;

			theme_cols[i] = c;
		}
		else
			return false;

		if (nextline != NULL) {
			rem -= valsize + 1;
			valstart = nextline + 1;
		}
		else
			break;
	}

	return true; // parsed exactly NUM_COLORS successfully
}
