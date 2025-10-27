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

using namespace tobkit;

Theme::Theme(void)
{
	// Set default colors
	col_bg                  = RGB15(4,6,15)|BIT(15);
	col_dark_bg             = col_bg;
	col_medium_bg           = RGB15(9,11,17)|BIT(15);
	col_light_bg            = RGB15(16,18,24)|BIT(15);
	col_lighter_bg          = RGB15(23,25,31) | BIT(15);
	col_light_ctrl          = RGB15(31,31,0)|BIT(15); // RGB15(26,26,26)|BIT(15)
	col_dark_ctrl           = RGB15(31,18,0)|BIT(15); // RGB15(31,31,31)|BIT(15)
	col_light_ctrl_disabled = col_light_bg;
	col_dark_ctrl_disabled  = col_medium_bg;
	col_list_highlight1     = RGB15(28,15,0)|BIT(15);
	col_list_highlight2     = RGB15(28,28,0)|BIT(15);
	col_outline             = RGB15(0,0,0)|BIT(15);
	col_tab_outline			= col_outline;
	col_sepline             = RGB15(31,31,0)|BIT(15);
	col_icon                = RGB15(0,0,0)|BIT(15);
	col_text                = RGB15(0,0,0)|BIT(15);
	col_signal              = RGB15(31,0,0)|BIT(15);
	col_signal_off			= RGB15(18, 0, 0) | BIT(15);
	col_piano_label			= RGB15(0, 0, 0);
	col_piano_label_inv		= RGB15(31,31,31);
	col_loop				= RGB15(7,25,5)|BIT(15);
	col_env_sustain			= RGB15(0,31,0) | BIT(15);

	col_mem_ok				= RGB15(17,24,16) | BIT(15);
	col_mem_warn			= RGB15(31,31,0) | BIT(15);
	col_mem_alert			= RGB15(31,0,0) | BIT(15);
	col_typewriter_cursor   = RGB15(0,0,0)|BIT(15);
	col_pv_lines			= col_light_bg;
	col_pv_sublines			= RGB15(7,9,17)|BIT(15);
	col_pv_lines_record		= col_dark_ctrl;
	col_pv_cb_col1			= col_medium_bg;
	col_pv_cb_col2			= col_light_bg;
	col_pv_cb_col1_highlight= col_list_highlight1;
	col_pv_cb_col2_highlight= col_list_highlight2;
	col_pv_left_numbers		= col_list_highlight1;
	col_pv_notes			= RGB15(9,15,31)|BIT(15);
	col_pv_notes_dark		= RGB15(0,6,26)|BIT(15);
	col_pv_instr			= RGB15(31,11,0)|BIT(15);
	col_pv_instr_dark		= RGB15(20,6,0)|BIT(15);
	col_pv_volume			= RGB15(0,27,0)|BIT(15);
	col_pv_volume_dark		= RGB15(0,16,0)|BIT(15);
	col_pv_effect			= RGB15(31,12,29)|BIT(15);
	col_pv_effect_dark		= RGB15(12,6,18)|BIT(15);
	col_pv_effect_param		= RGB15(30,26,8)|BIT(15);
	col_pv_effect_param_dark= RGB15(9,8,5)|BIT(15);
	col_pv_cb_sel_highlight	= RGB15(31,24,0)|BIT(15);

}
