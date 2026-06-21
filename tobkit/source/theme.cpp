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

#include "tobkit/platform.h"
#include "tobkit/theme.h"
#include <stdio.h>
#include <string.h>

using namespace tobkit;

ColorScheme::ColorScheme() {
	col_bg = RGB5A1(4, 6, 15, 1);
	col_env_bg = col_bg;
	col_medium_bg = RGB5A1(9, 11, 17, 1);
	col_light_bg = RGB5A1(16, 18, 24, 1);
	col_lighter_bg = RGB5A1(23, 25, 31, 1);
	col_light_ctrl = RGB5A1(31, 31, 0, 1); // RGB5A1(26,26,26,1)
	col_dark_ctrl = RGB5A1(31, 18, 0, 1);
	col_light_ctrl_disabled = col_light_bg;
	col_dark_ctrl_disabled = col_medium_bg;
	col_selected_tab = col_light_bg;
	col_unselected_tab = col_medium_bg;
	col_list_1 = col_medium_bg;
	col_list_2 = col_light_bg;
	col_list_highlight1 = RGB5A1(28, 15, 0, 1);
	col_list_highlight2 = RGB5A1(28, 28, 0, 1);
	col_scrollbar_bg1 = col_medium_bg;
	col_scrollbar_bg2 = col_light_bg;
	col_scrollbar_inactive = col_dark_ctrl;
	col_scrollbar_active = col_light_ctrl;
	col_scrollbar_arr_bg1 = col_dark_ctrl;
	col_scrollbar_arr_bg2 = col_light_ctrl;
	col_outline = RGB5A1(0, 0, 0, 1);
	col_tab_outline = col_outline;
	col_sepline = RGB5A1(31, 31, 0, 1);
	col_tab_icon = RGB5A1(0, 0, 0, 1);
	col_icon_bt = col_tab_icon;
	col_checkmark = col_tab_icon;
	col_text = RGB5A1(0, 0, 0, 1);
	col_text_light = col_light_bg;
	col_text_bt = col_text;
	col_text_value = col_text;
	col_text_lb = col_text;
	col_text_lb_highlight = col_text;
	col_signal = RGB5A1(31, 0, 0, 1);
	col_signal_off = RGB5A1(18, 0, 0, 1);
	col_piano_label = RGB5A1(0, 0, 0, 1);
	col_piano_label_inv = RGB5A1(31, 31, 31, 1);
	col_loop = RGB5A1(7, 25, 5, 1);
	col_env_sustain = RGB5A1(0, 31, 0, 1);
	col_env_line = col_dark_ctrl;
	col_env_pt = col_light_ctrl;
	col_env_pt_border = col_outline;
	col_env_pt_border_active = col_signal;
	col_mem_ok = RGB5A1(17, 24, 16, 1);
	col_mem_warn = RGB5A1(31, 31, 0, 1);
	col_mem_alert = RGB5A1(31, 0, 0, 1);
	col_typewriter_cursor = RGB5A1(0, 0, 0, 1);
	col_smp_bg = col_bg;
	col_smp_bg_sel = col_light_ctrl;
	col_smp_waveform = RGB5A1(31, 19, 0, 1);
	col_smp_waveform_sel = RGB5A1(0, 0, 0, 1);
	col_pv_bg = col_bg;
	col_pv_chn = col_light_bg;
	col_pv_lines = col_light_bg;
	col_pv_sublines = RGB5A1(7, 9, 17, 1);
	col_pv_lines_record = col_dark_ctrl;
	col_pv_cb_col1 = col_medium_bg;
	col_pv_cb_col2 = col_light_bg;
	col_pv_cb_col1_highlight = col_list_highlight1;
	col_pv_cb_col2_highlight = col_list_highlight2;
	col_pv_left_numbers = col_list_highlight1;
	col_pv_notes = RGB5A1(9, 15, 31, 1);
	col_pv_notes_dark = RGB5A1(0, 6, 26, 1);
	col_pv_instr = RGB5A1(31, 11, 0, 1);
	col_pv_instr_dark = RGB5A1(20, 6, 0, 1);
	col_pv_volume = RGB5A1(0, 27, 0, 1);
	col_pv_volume_dark = RGB5A1(0, 16, 0, 1);
	col_pv_effect = RGB5A1(31, 12, 29, 1);
	col_pv_effect_dark = RGB5A1(12, 6, 18, 1);
	col_pv_effect_param = RGB5A1(30, 26, 8, 1);
	col_pv_effect_param_dark = RGB5A1(9, 8, 5, 1);
	col_pv_cb_sel_highlight = RGB5A1(31, 24, 0, 1);
	col_pv_pb = col_outline;
	col_pv_pb_cell = col_outline;
	col_pv_mutesolo_text = col_text;
	col_pv_mutesolo_col1 = col_pv_cb_col1;
	col_pv_mutesolo_col2 = col_pv_cb_col2;
	col_pv_mutesolo_col1_highlight = col_pv_cb_col1_highlight;
	col_pv_mutesolo_col2_highlight = col_pv_cb_col2_highlight;
	col_pv_left_numbers_highlight = col_pv_left_numbers;
	col_list_sep_vertical = col_sepline;
	col_tb_bg_off_col1 = col_dark_ctrl;
	col_tb_fg_off = col_text_bt;
	col_tb_fg_on = col_light_ctrl;
	col_piano_full_col1 = RGB5A1(31, 31, 31, 1);
	col_piano_full_col2 = RGB5A1(25, 25, 25, 1);
	col_piano_half_col1 = RGB5A1(0, 0, 0, 1);
	col_piano_half_col2 = RGB5A1(8, 8, 8, 1);
	col_piano_full_highlight_col1 = RGB5A1(24, 24, 30, 1);
	col_piano_full_highlight_col2 = RGB5A1(20, 20, 26, 1);
	col_piano_half_highlight_col1 = RGB5A1(20, 8, 8, 1);
	col_piano_half_highlight_col2 = RGB5A1(13, 0, 0, 1);
	col_piano_outline = RGB5A1(0, 0, 0, 1);
	col_typewriter_bg = RGB5A1(31, 31, 31, 1);
	col_typewriter_key = RGB5A1(22, 22, 28, 1);
	col_typewriter_key_label = RGB5A1(0, 0, 0, 1);
	col_typewriter_mod_key = RGB5A1(17, 17, 26, 1);
	col_typewriter_pressed_key = col_typewriter_bg;
	col_typewriter_mod_key_label = col_typewriter_key_label;
	col_smp_zoom = col_light_bg;
	col_messagebox_title_col1 = col_list_highlight1;
	col_messagebox_title_col2 = col_list_highlight2;
	col_messagebox_title_text = col_text;
	col_fxkeyboard_col1 = col_piano_full_col1;
	col_fxkeyboard_col2 = col_piano_full_col2;
	col_fxkeyboard_col1_disabled = col_light_ctrl_disabled;	
	col_fxkeyboard_col2_disabled = col_dark_ctrl_disabled;
	col_fxkeyboard_btn_label = col_piano_label;
	col_fxkeyboard_cmd_desc = col_text_light;	
	col_fxkeyboard_cmd_desc_disabled = col_dark_ctrl_disabled;	
	col_fxkeyboard_minilabel_x = col_pv_notes &~ RGB5A1_ALPHA_BIT;	
	col_fxkeyboard_minilabel_y = col_pv_effect &~ RGB5A1_ALPHA_BIT;
	col_typewriter_disabled_key = RGB5A1(25, 25, 25, 1);
	col_light_ctrl_pressed = col_dark_ctrl;
	col_dark_ctrl_pressed = col_light_ctrl;
	col_text_bt_pressed = col_text_bt;
	col_icon_bt_pressed = col_icon_bt;
	col_tb_bg_on_col1 = col_tb_bg_off_col1;
	col_tb_bg_on_col2 = col_tb_bg_off_col1;
	col_tb_bg_off_col2 = col_tb_bg_off_col1;
	col_pv_mutesolo_text_highlight = col_text;
	col_tab_icon_highlight = col_tab_icon;
	col_smp_offset_guide = col_list_sep_vertical;
}

Theme::Theme(char* themepath, bool use_fat)
{
	if (use_fat && themepath != NULL)
	{
		loadTheme(themepath);
	}
}

bool Theme::loadTheme(const char* themefile) {
	FILE* themedef = fopen(themefile, "r");
	if (themedef != NULL) {
		tobkit::ColorScheme scheme;
		bool result = parseTheme(themedef, scheme.data);
		fclose(themedef);

		if (result) {
			memcpy(data, scheme.data, sizeof(data));
			col_piano_label &= ~RGB5A1_ALPHA_BIT;
			col_piano_label_inv &= ~RGB5A1_ALPHA_BIT;
			col_fxkeyboard_btn_label &= ~RGB5A1_ALPHA_BIT;
			col_fxkeyboard_minilabel_x &= ~RGB5A1_ALPHA_BIT;
			col_fxkeyboard_minilabel_y &= ~RGB5A1_ALPHA_BIT;

			debugprintf("loaded theme '%s'\n", themefile);
			return true;
		}
		else
			debugprintf("failed to parse theme at '%s', using builtin\n", themefile);
	}
	else
		debugprintf("no theme found at '%s', using builtin\n", themefile);
	return false;
}

void Theme::loadDefault(void) {
	tobkit::ColorScheme scheme;

	memcpy(data, scheme.data, sizeof(data));
	col_piano_label &= ~RGB5A1_ALPHA_BIT;
	col_piano_label_inv &= ~RGB5A1_ALPHA_BIT;
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
	*col = (u16)(RGB5A1(r >> 3, g >> 3, b >> 3, 1));
	return true;
}

// not needed as we are not writing themes currently
void Theme::RGB15ToString(u16 col, char* str)
{
	sprintf(str, "%02x%02x%02x", (col & 0x1f) << 3, ((col >> 5) & 0x1f) << 3, (col >> 10 & 0x1f) << 3);
}


bool Theme::parseTheme(FILE* theme_, u16* theme_cols) {
	if (theme_ == NULL || theme_cols == NULL)
		return false;

	int r, g, b, k, l, parsed;

	bool theme_has_key[NUM_COLORS] = { 0 };

	for (l = 0;;++l) {
		parsed = fscanf(theme_, "%d=%02x%02x%02x%*[^\n]\n", &k, &r, &g, &b);
		if (parsed == EOF)
			break;
			
		if (parsed != 4 || k < 0) {
			debugprintf("theme parse error on line %d\n", l + 1);
			return false;
		}
		else if (k > NUM_COLORS - 1) {
			debugprintf("theme parse error on line %d (key out of bounds, max %d)  \n", l + 1, NUM_COLORS - 1);
			return false;
		}
		theme_cols[k] = RGB5A1(r >> 3, g >> 3, b >> 3, 1);
		theme_has_key[k] = true;
	}

	if (l == 0) {
		ntxm_dprintf("ignoring empty theme\n");
		return false;
	}
	
	// check if specific colours were specified by the theme
	// if not replace them with their previous colour from the same theme
	if (!theme_has_key[99]) theme_cols[99] = theme_cols[3];		// Sample editor zoom buttons
	if (!theme_has_key[100]) theme_cols[100] = theme_cols[13];	// Message box title gradient col1
	if (!theme_has_key[101]) theme_cols[101] = theme_cols[14];	// Message box title gradient col2
	if (!theme_has_key[102]) theme_cols[102] = theme_cols[27];	// Message box title text

	if (!theme_has_key[103]) theme_cols[103] = theme_cols[84];	// Fxkb button gradient 1
	if (!theme_has_key[104]) theme_cols[104] = theme_cols[85];	// Fxkb button gradient 2
	if (!theme_has_key[105]) theme_cols[105] = theme_cols[7];	// Fxkb disabled button gradient 1 
	if (!theme_has_key[106]) theme_cols[106] = theme_cols[8];	// Fxkb disabled button gradient 2
	if (!theme_has_key[107]) theme_cols[107] = theme_cols[35];	// Fxkb big button label
	if (!theme_has_key[108]) theme_cols[108] = theme_cols[28];	// Fxkb command desc label
	if (!theme_has_key[109]) theme_cols[109] = theme_cols[8];	// Fxkb disabled command desc label
	if (!theme_has_key[110]) theme_cols[110] = theme_cols[61];	// Fxkb button param label 'X'
	if (!theme_has_key[111]) theme_cols[111] = theme_cols[67];	// Fxkb button param label 'Y'
	if (!theme_has_key[112]) theme_cols[112] = theme_cols[93];	// Typewriter disabled key

	if (!theme_has_key[113]) theme_cols[113] = theme_cols[6];   // Light button gradient (pressed)
	if (!theme_has_key[114]) theme_cols[114] = theme_cols[5];   // Dark button gradient (pressed)
	if (!theme_has_key[115]) theme_cols[115] = theme_cols[29];  // Button text (pressed)
	if (!theme_has_key[116]) theme_cols[116] = theme_cols[25];  // Button icon (pressed)
	if (!theme_has_key[117]) theme_cols[117] = theme_cols[81];  // Togglebutton background (on) 1
	if (!theme_has_key[118]) theme_cols[118] = theme_cols[81];  // Togglebutton background (on) 2
	if (!theme_has_key[119]) theme_cols[119] = theme_cols[81];  // Togglebutton background (off) 2
	if (!theme_has_key[120]) theme_cols[120] = theme_cols[74];  // Mute/solo pressed button text
	if (!theme_has_key[121]) theme_cols[121] = theme_cols[24];  // Selected tab icon
	if (!theme_has_key[122]) theme_cols[122] = theme_cols[80];  // Sample display offset preview



	return true;
}
