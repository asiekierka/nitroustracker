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

#ifndef _THEME_H_
#define _THEME_H_

#include <nds.h>

namespace tobkit {

class Theme
{
	public:
		Theme();
		
		u16 col_bg;
		u16 col_dark_bg;
		u16 col_medium_bg;
		u16 col_light_bg;
		u16 col_lighter_bg;
		u16 col_light_ctrl;
		u16 col_dark_ctrl;
		u16 col_light_ctrl_disabled;
		u16 col_dark_ctrl_disabled;
		u16 col_list_highlight1;
		u16 col_list_highlight2;
		u16 col_outline;
		u16 col_tab_outline;
		u16 col_sepline;
		u16 col_icon;
		u16 col_text;
		u16 col_signal;
		u16 col_signal_off;
		u16 col_piano_label;
		u16 col_piano_label_inv;
		u16 col_loop;
		u16 col_env_sustain;
		u16 col_mem_ok;
		u16 col_mem_warn;
		u16 col_mem_alert;
		u16 col_typewriter_cursor;
		u16 col_pv_lines;
		u16	col_pv_sublines;
		u16	col_pv_lines_record;
		u16 col_pv_cb_col1;
		u16 col_pv_cb_col2;
		u16	col_pv_cb_col1_highlight;
		u16	col_pv_cb_col2_highlight;
		u16 col_pv_left_numbers;
		u16 col_pv_notes;
		u16 col_pv_instr;
		u16 col_pv_volume;
		u16 col_pv_effect;
		u16 col_pv_effect_param;
		u16 col_pv_notes_dark;
		u16 col_pv_instr_dark;
		u16 col_pv_volume_dark;
		u16 col_pv_effect_dark;
		u16 col_pv_effect_param_dark;
		u16 col_pv_cb_sel_highlight;
};

};

#endif
