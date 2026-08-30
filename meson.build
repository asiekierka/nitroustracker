project(
    'nitroustracker', ['c', 'cpp'],
    version: '0.7.0b4',
    default_options: ['buildtype=debugoptimized', 'debug=true', 'optimization=2', 'c_std=gnu2x', 'cpp_std=gnu++20', 'warning_level=3']
)
cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
windows_build = target_machine.system() == 'windows'
if windows_build
	add_global_link_arguments(['-static-libgcc', '-static-libstdc++'], language: 'cpp')
endif

conf_data = configuration_data()
conf_data.set_quoted('VERSION', meson.project_version())

libntxm_sources = [
  'libntxm/libntxm/source/common/demokit.c',
  'libntxm/libntxm/source/common/instrument.cpp',
  'libntxm/libntxm/source/common/ntxmtools.c',
  'libntxm/libntxm/source/common/sample.cpp',
  'libntxm/libntxm/source/common/song.cpp',
  'libntxm/libntxm/source/common/tables.c',
  'libntxm/libntxm/source/player/player.cpp',
  'libntxm/libntxm/source/single/fifocommand.cpp',
  'libntxm/libntxm/source/single/ntxmsound.cpp',
  'libntxm/libntxm/source/single/platform_sdl3.cpp',
  'libntxm/libntxm/source/transport/format_transport.cpp',
  'libntxm/libntxm/source/transport/mod_transport.cpp',
  'libntxm/libntxm/source/transport/wav.cpp',
  'libntxm/libntxm/source/transport/xm_transport.cpp'
]

nitroustracker_sources = [
  'tobkit/source/bitbutton.cpp',
  'tobkit/source/button.cpp',
  'tobkit/source/checkbox.cpp',
  'tobkit/source/containerwidget.cpp',
  'tobkit/source/digitbox.cpp',
  'tobkit/source/fileselector.cpp',
  'tobkit/source/gradienticon.cpp',
  'tobkit/source/groupbox.cpp',
  'tobkit/source/gui.cpp',
  'tobkit/source/label.cpp',
  'tobkit/source/listbox.cpp',
  'tobkit/source/memoryindicator.cpp',
  'tobkit/source/messagebox.cpp',
  'tobkit/source/numberbox.cpp',
  'tobkit/source/numberslider.cpp',
  'tobkit/source/piano.cpp',
  'tobkit/source/piano_hit.c',
  'tobkit/source/piano_pal.c',
  'tobkit/source/pixmap.cpp',
  'tobkit/source/radiobutton.cpp',
  'tobkit/source/radiobuttongroup.cpp',
  'tobkit/source/rowbox.cpp',
  'tobkit/source/screen.cpp',
  'tobkit/source/tabbox.cpp',
  'tobkit/source/theme.cpp',
  'tobkit/source/themeselectorbox.cpp',
  'tobkit/source/togglebutton.cpp',
  'tobkit/source/twotabbox.cpp',
  'tobkit/source/typewriter.cpp',
  'tobkit/source/typewriter_hit.c',
  'tobkit/source/widget.cpp',
  'common/source/action.cpp',
  'common/source/cell_array.cpp',
  'common/source/debug_helpers.cpp',
  'common/source/dsmidi_handler.cpp',
  'common/source/main.cpp',
  'common/source/settings.cpp',
  'common/source/state.cpp',
  'common/source/libnds/trig.c',
  'common/source/tobkit/envelope_editor.cpp',
  'common/source/tobkit/fxkeyboard.cpp',
  'common/source/tobkit/normalizebox.cpp',
  'common/source/tobkit/numbersliderrelnote.cpp',
  'common/source/tobkit/patternview.cpp',
  'common/source/tobkit/recordbox.cpp',
  'common/source/tobkit/sampledisplay.cpp',
  'common/source/tools/strlcat.c',
  'common/source/tools/strlcpy.c',
  'common/source/tools/tools.cpp',
  'sdl/source/display_manager.cpp',
  'sdl/source/platform.cpp'
]

bin2c_prog = find_program('tools/bin2c.py')
font_3x5_pack_prog = find_program('tools/font_3x5_pack.py')
font_pack_prog = find_program('tools/font_pack.py')
icon_pack_prog = find_program('tools/icon_pack.py')

icon_pngs = [
  'tobkit/bitmaps/haken.png',
  'common/bitmaps/icon_copy.png',
  'common/bitmaps/icon_cut.png',
  'common/bitmaps/icon_disk.png',
  'common/bitmaps/icon_disk_unsaved.png',
  'common/bitmaps/icon_flp.png',
  'common/bitmaps/icon_fx.png',
  'common/bitmaps/icon_new_folder.png',
  'common/bitmaps/icon_paste.png',
  'common/bitmaps/icon_pause.png',
  'common/bitmaps/icon_play.png',
  'common/bitmaps/icon_record.png',
  'common/bitmaps/icon_redo.png',
  'common/bitmaps/icon_sample.png',
  'common/bitmaps/icon_song.png',
  'common/bitmaps/icon_stop.png',
  'common/bitmaps/icon_trumpet.png',
  'common/bitmaps/icon_undo.png',
  'common/bitmaps/icon_wrench.png',
  'common/bitmaps/nitrotracker_logo.png',
  'common/bitmaps/instedit_panenv.png',
  'common/bitmaps/instedit_vibrato.png',
  'common/bitmaps/instedit_volenv.png',
  'common/bitmaps/sampleedit_all.png',
  'common/bitmaps/sampleedit_chip_icon.png',
  'common/bitmaps/sampleedit_control_icon.png',
  'common/bitmaps/sampleedit_del.png',
  'common/bitmaps/sampleedit_draw.png',
  'common/bitmaps/sampleedit_draw_small.png',
  'common/bitmaps/sampleedit_fadein.png',
  'common/bitmaps/sampleedit_fadeout.png',
  'common/bitmaps/sampleedit_loop_icon.png',
  'common/bitmaps/sampleedit_none.png',
  'common/bitmaps/sampleedit_normalize.png',
  'common/bitmaps/sampleedit_record.png',
  'common/bitmaps/sampleedit_reverse.png',
  'common/bitmaps/sampleedit_trim.png',
  'common/bitmaps/sampleedit_wave_icon.png',
  'common/bitmaps/settings_icon_midi.png',
  'common/bitmaps/settings_icon_sound.png',
  'common/bitmaps/settings_icon_visual.png',
  'common/bitmaps/vibrato_saw.png',
  'common/bitmaps/vibrato_saw2.png',
  'common/bitmaps/vibrato_sine.png',
  'common/bitmaps/vibrato_square.png'
]

conf_data.set('HAVE_DIRENT_D_TYPE', cc.has_member('struct dirent', 'd_type', prefix : '#include <dirent.h>'))
conf_data.set('HAVE_STRLCAT', cc.has_function('strlcat', prefix : '#include <string.h>'))
conf_data.set('HAVE_STRLCPY', cc.has_function('strlcpy', prefix : '#include <string.h>'))

sdl3_dep = dependency('sdl3', required: true)
add_project_arguments(
	cc.get_supported_arguments([
		'-Wno-unused-parameter'
	]),
	language: 'c')
add_project_arguments(
	cxx.get_supported_arguments([
		'-Wno-unused-parameter'
	]),
	language: 'cpp')
add_project_arguments('-DNT_PLATFORM_SDL3', language: 'c')
add_project_arguments('-DNT_PLATFORM_SDL3', language: 'cpp')
add_project_arguments('-DGIT_HASH="WIP"', language: 'cpp')
add_project_arguments('-DDEBUG', language: 'cpp')
  
configure_file(output : 'config.h', configuration : conf_data)

libntxm = static_library('ntxm', libntxm_sources,
  dependencies: [sdl3_dep],
  include_directories: include_directories('libntxm/libntxm/include'))

font_3x5_pack = generator(font_3x5_pack_prog, output: '@BASENAME@.raw', arguments: ['@INPUT@', '@OUTPUT@'])
font_pack = generator(font_pack_prog, output: '@BASENAME@.raw', arguments: ['@INPUT@', '@OUTPUT@'])
icon_pack = generator(icon_pack_prog, output: '@BASENAME@.raw', arguments: ['@INPUT@', '@OUTPUT@'])
bin2c_raw = generator(bin2c_prog, output: '@BASENAME@_raw.c', arguments: ['--field_name', '@BASENAME@_raw', '@OUTPUT@', '@INPUT@'])
icon_raws = icon_pack.process(icon_pngs)
font_raws = font_pack.process('tobkit/fonts/font_8x11.png')
font_3x5_raws = font_3x5_pack.process('tobkit/fonts/font_3x5.png')
nitroustracker_sources += bin2c_raw.process(icon_raws)
nitroustracker_sources += bin2c_raw.process(font_raws)
nitroustracker_sources += bin2c_raw.process(font_3x5_raws)

nitroustracker_dependencies = [sdl3_dep]

if windows_build
	windows = import('windows')
	nitroustracker_sources += windows.compile_resources('assets/mingw/resources.rc')
endif

nitroustracker = executable('nitroustracker', nitroustracker_sources,
  dependencies: nitroustracker_dependencies,
  include_directories: include_directories('libntxm/libntxm/include', 'tobkit/include', 'common/source', 'sdl/source'),
  link_with: [libntxm])
