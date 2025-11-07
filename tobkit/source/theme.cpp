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
	col_env_bg(col_bg),
	col_medium_bg(RGB15(9, 11, 17) | BIT(15)),
	col_light_bg(RGB15(16, 18, 24) | BIT(15)),
	col_lighter_bg(RGB15(23, 25, 31) | BIT(15)),
	col_light_ctrl(RGB15(31, 31, 0) | BIT(15)), // RGB15(26,26,26)|BIT(15)
	col_dark_ctrl(RGB15(31, 18, 0) | BIT(15)),
	col_light_ctrl_disabled(col_light_bg),
	col_dark_ctrl_disabled(col_medium_bg),
	col_selected_tab(col_light_bg),
	col_unselected_tab(col_medium_bg),
	col_list_1(col_medium_bg),
	col_list_2(col_light_bg),
	col_list_highlight1(RGB15(28, 15, 0) | BIT(15)),
	col_list_highlight2(RGB15(28, 28, 0) | BIT(15)),
	col_scrollbar_bg1(col_medium_bg),
	col_scrollbar_bg2(col_light_bg),
	col_scrollbar_inactive(col_dark_ctrl),
	col_scrollbar_active(col_light_ctrl),
	col_scrollbar_arr_bg1(col_dark_ctrl),
	col_scrollbar_arr_bg2(col_light_ctrl),
	col_outline(RGB15(0, 0, 0) | BIT(15)),
	col_tab_outline(col_outline),
	col_sepline(RGB15(31, 31, 0) | BIT(15)),
	col_icon(RGB15(0, 0, 0) | BIT(15)),
	col_icon_bt(col_icon),
	col_checkmark(col_icon),
	col_text(RGB15(0, 0, 0) | BIT(15)),
	col_text_light(col_light_bg),
	col_text_bt(col_text),
	col_text_value(col_text),
	col_text_lb(col_text),
	col_text_lb_highlight(col_text),
	col_signal(RGB15(31, 0, 0) | BIT(15)),
	col_signal_off(RGB15(18, 0, 0) | BIT(15)),
	col_piano_label(RGB15(0, 0, 0)),
	col_piano_label_inv(RGB15(31, 31, 31)),
	col_loop(RGB15(7, 25, 5) | BIT(15)),
	col_env_sustain(RGB15(0, 31, 0) | BIT(15)),
	col_env_line(col_dark_ctrl),
	col_env_pt(col_light_ctrl),
	col_env_pt_border(col_outline),
	col_env_pt_border_active(col_signal),
	col_mem_ok(RGB15(17, 24, 16) | BIT(15)),
	col_mem_warn(RGB15(31, 31, 0) | BIT(15)),
	col_mem_alert(RGB15(31, 0, 0) | BIT(15)),
	col_typewriter_cursor(RGB15(0, 0, 0) | BIT(15)),
	col_smp_bg(col_bg),
	col_smp_bg_sel(col_light_ctrl),
	col_smp_waveform(RGB15(31, 19, 0) | BIT(15)),
	col_smp_waveform_sel(RGB15(0, 0, 0) | BIT(15)),
	col_pv_bg(col_bg),
	col_pv_chn(col_light_bg),
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
	col_pv_cb_sel_highlight(RGB15(31, 24, 0) | BIT(15)), 
	col_pv_pb(col_outline),
	col_pv_pb_cell(col_outline),
	col_pv_mutesolo_text(col_text),
	col_pv_mutesolo_col1(col_pv_cb_col1),
	col_pv_mutesolo_col2(col_pv_cb_col2),
	col_pv_mutesolo_col1_highlight(col_pv_cb_col1_highlight),
	col_pv_mutesolo_col2_highlight(col_pv_cb_col2_highlight),
	
	fat(use_fat)
{
	if (fat == true && themepath != NULL)
	{
		FILE* themedef = fopen(themepath, "r");
		// debugprintf("loading theme '%s'\n", themepath);

		if (themedef != NULL)  {
			u16* colscheme = (u16*)calloc(NUM_COLORS, sizeof(u16));
			bool result = parseTheme(themedef, colscheme);

			fclose(themedef);

			if (result) {
				col_bg							= colscheme[0] | BIT(15);
				col_env_bg 						= colscheme[1] | BIT(15);
				col_medium_bg 					= colscheme[2] | BIT(15);
				col_light_bg 					= colscheme[3] | BIT(15);
				col_lighter_bg 					= colscheme[4] | BIT(15);
				col_light_ctrl 					= colscheme[5] | BIT(15);
				col_dark_ctrl 					= colscheme[6] | BIT(15);
				col_light_ctrl_disabled 		= colscheme[7] | BIT(15);
				col_dark_ctrl_disabled 			= colscheme[8] | BIT(15);
				col_selected_tab 				= colscheme[9] | BIT(15);
				col_unselected_tab 				= colscheme[10] | BIT(15);
				col_list_1 						= colscheme[11] | BIT(15);
				col_list_2 						= colscheme[12] | BIT(15);
				col_list_highlight1 			= colscheme[13] | BIT(15);
				col_list_highlight2 			= colscheme[14] | BIT(15);
				col_scrollbar_bg1 				= colscheme[15] | BIT(15);
				col_scrollbar_bg2 				= colscheme[16] | BIT(15);
				col_scrollbar_inactive 			= colscheme[17] | BIT(15);
				col_scrollbar_active 			= colscheme[18] | BIT(15);
				col_scrollbar_arr_bg1 			= colscheme[19] | BIT(15);
				col_scrollbar_arr_bg2 			= colscheme[20] | BIT(15);
				col_outline 					= colscheme[21] | BIT(15);
				col_tab_outline 				= colscheme[22] | BIT(15);
				col_sepline 					= colscheme[23] | BIT(15);
				col_icon 						= colscheme[24] | BIT(15);
				col_icon_bt 					= colscheme[25] | BIT(15);
				col_checkmark 					= colscheme[26] | BIT(15);
				col_text 						= colscheme[27] | BIT(15);
				col_text_light 					= colscheme[28] | BIT(15);
				col_text_bt 					= colscheme[29] | BIT(15);
				col_text_value 					= colscheme[30] | BIT(15);
				col_text_lb 					= colscheme[31] | BIT(15);
				col_text_lb_highlight 			= colscheme[32] | BIT(15);
				col_signal 						= colscheme[33] | BIT(15);
				col_signal_off 					= colscheme[34] | BIT(15);
				col_piano_label 				= colscheme[35];
				col_piano_label_inv 			= colscheme[36];
				col_loop 						= colscheme[37] | BIT(15);
				col_env_sustain 				= colscheme[38] | BIT(15);
				col_env_line 					= colscheme[39] | BIT(15);
				col_env_pt 						= colscheme[40] | BIT(15);
				col_env_pt_border 				= colscheme[41] | BIT(15);
				col_env_pt_border_active 		= colscheme[42] | BIT(15);
				col_mem_ok 						= colscheme[43] | BIT(15);
				col_mem_warn 					= colscheme[44] | BIT(15);
				col_mem_alert 					= colscheme[45] | BIT(15);
				col_typewriter_cursor 			= colscheme[46] | BIT(15);
				col_smp_bg 						= colscheme[47] | BIT(15);
				col_smp_bg_sel 					= colscheme[48] | BIT(15);
				col_smp_waveform 				= colscheme[49] | BIT(15);
				col_smp_waveform_sel 			= colscheme[50] | BIT(15);
				col_pv_bg 						= colscheme[51] | BIT(15);
				col_pv_chn 						= colscheme[52] | BIT(15);
				col_pv_lines 					= colscheme[53] | BIT(15);
				col_pv_sublines 				= colscheme[54] | BIT(15);
				col_pv_lines_record 			= colscheme[55] | BIT(15);
				col_pv_cb_col1 					= colscheme[56] | BIT(15);
				col_pv_cb_col2 					= colscheme[57] | BIT(15);
				col_pv_cb_col1_highlight 		= colscheme[58] | BIT(15);
				col_pv_cb_col2_highlight 		= colscheme[59] | BIT(15);
				col_pv_left_numbers 			= colscheme[60] | BIT(15);
				col_pv_notes 					= colscheme[61] | BIT(15);
				col_pv_notes_dark 				= colscheme[62] | BIT(15);
				col_pv_instr 					= colscheme[63] | BIT(15);
				col_pv_instr_dark 				= colscheme[64] | BIT(15);
				col_pv_volume 					= colscheme[65] | BIT(15);
				col_pv_volume_dark 				= colscheme[66] | BIT(15);
				col_pv_effect 					= colscheme[67] | BIT(15);
				col_pv_effect_dark 				= colscheme[68] | BIT(15);
				col_pv_effect_param 			= colscheme[69] | BIT(15);
				col_pv_effect_param_dark 		= colscheme[70] | BIT(15);
				col_pv_cb_sel_highlight 		= colscheme[71] | BIT(15);
				col_pv_pb 						= colscheme[72] | BIT(15);
				col_pv_pb_cell					= colscheme[73] | BIT(15);
				col_pv_mutesolo_text 			= colscheme[74] | BIT(15);
				col_pv_mutesolo_col1 			= colscheme[75] | BIT(15);
				col_pv_mutesolo_col2 			= colscheme[76] | BIT(15);
				col_pv_mutesolo_col1_highlight 	= colscheme[77] | BIT(15);
				col_pv_mutesolo_col2_highlight 	= colscheme[78] | BIT(15);

				debugprintf("loaded theme '%s'\n", themepath);
			}

			else
				debugprintf("failed to parse theme at '%s', using builtin\n", themepath);

			free(colscheme);
		} else
			debugprintf("no theme found at '%s', using builtin\n", themepath);
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
	*col = (u16)RGB15(r * 31 / 255, g * 31 / 255, b * 31 / 255);
	return true;
}

// not needed as we are not writing themes currently
void Theme::RGB15ToString(u16 col, char* str)
{
	sprintf(str, "%02x%02x%02x", (col & 0x1f) * 255 / 31, ((col >> 5) & 0x1f) * 255 / 31, (col >> 10 & 0x1f) * 255 / 31);
}


bool Theme::parseTheme(FILE *theme_, u16 *theme_cols) {
	if (theme_ == NULL || theme_cols == NULL)
		return false;

	int theme_i = 0;
	int r, g, b, k, parsed;

	for (int l = 0; l < NUM_COLORS; ++l) {
		parsed = fscanf(theme_, "%d=%02x%02x%02x%*[^\n]\n", &k, &r, &g, &b);
		
		if (parsed != 4 || k < 0) {
			debugprintf("theme parse error on line %d\n", l + 1);
			return false;
		} else if ( k > NUM_COLORS - 1 ) {
			debugprintf("theme parse error on line %d (key out of bounds, max %d)  \n", l + 1, NUM_COLORS - 1);
			return false;
		}
		theme_cols[k] = RGB15(r * 31 / 255, g * 31 / 255, b * 31 / 255);
		theme_i++;
	} 

	if (theme_i < NUM_COLORS - 1) {
		debugprintf("theme only specifies %u colors, need %u\n", theme_i, NUM_COLORS);
		return false;
	}
		
	return true;
}
