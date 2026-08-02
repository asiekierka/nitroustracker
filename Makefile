# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2023

BLOCKSDS	?= /opt/blocksds/core
BLOCKSDSEXT	?= /opt/blocksds/external
WONDERFUL_TOOLCHAIN	?= /opt/wonderful

# User config
# ===========

NAME		:= nitroustracker
export VERSION		?= 0.7.0b3
export GIT_HASH		:= $(shell git rev-parse --short HEAD)

GAME_TITLE	:= NitrousTracker
GAME_VERSION	:= v$(VERSION)

GAME_ICON	:= assets/icon.bmp
GAME_ICON_DEBUG	:= assets/icon.debug.bmp

GAME_FULL_TITLE       := $(GAME_TITLE);$(GAME_VERSION)
GAME_FULL_TITLE_MIDI  := $(GAME_TITLE);$(GAME_VERSION);(compatible with MIDI)
GAME_FULL_TITLE_DEBUG := $(GAME_TITLE);$(GAME_VERSION);(debug build)

# Source code paths
# -----------------

NITROFSDIR	:= romfs

# Tools
# -----

MAKE		:= make
RM		:= rm -rf

# Verbose flag
# ------------

ifeq ($(VERBOSE),1)
V		:=
else
V		:= @
endif

# Directories
# -----------

ARM9DIR		:= arm9
ARM7DIR		:= arm7

# Build artfacts
# --------------

NITROFAT_IMG	:= build/nitrofat.bin
ROM		:= $(NAME).midi.nds
ROM_DEBUG	:= $(NAME).debug.nds
ROM_NOMIDI	:= $(NAME).nds

# Targets
# -------

.PHONY: all clean arm9d arm9 arm9l arm7d arm7 arm7l libntxm tobkit tobkit-debug libdsmi format

all: $(ROM) $(ROM_DEBUG) $(ROM_NOMIDI)

clean:
	@echo "  CLEAN"
	$(V)$(MAKE) -f Makefile.arm9 clean --no-print-directory DEBUG=true MIDI=true
	$(V)$(MAKE) -f Makefile.arm9 clean --no-print-directory DEBUG=false
	$(V)$(MAKE) -f Makefile.arm9 clean --no-print-directory DEBUG=false MIDI=true
	$(V)$(MAKE) -f Makefile.arm7 clean --no-print-directory DEBUG=true MIDI=true
	$(V)$(MAKE) -f Makefile.arm7 clean --no-print-directory DEBUG=false
	$(V)$(MAKE) -f Makefile.arm7 clean --no-print-directory DEBUG=false MIDI=true
	$(V)$(MAKE) -C tobkit clean DEBUG=true
	$(V)$(MAKE) -C tobkit clean DEBUG=false
	$(V)$(MAKE) -C dsmi/ds/libdsmi -f Makefile.blocks clean
	$(V)$(MAKE) -C libntxm/libntxm clean
	$(V)$(RM) $(ROM) $(ROM_DEBUG) $(ROM_NOMIDI) build

libntxm:
	@make -C libntxm/libntxm

tobkit:
	@make -C tobkit

tobkit-debug:
	@make -C tobkit DEBUG=true

libdsmi:
	@make -C dsmi/ds/libdsmi -f Makefile.blocks

arm9d: tobkit-debug libntxm libdsmi
	$(V)+$(MAKE) -f Makefile.arm9 --no-print-directory DEBUG=true MIDI=true

arm9: tobkit libntxm libdsmi
	$(V)+$(MAKE) -f Makefile.arm9 --no-print-directory DEBUG=false MIDI=true

arm9l: tobkit libntxm libdsmi
	$(V)+$(MAKE) -f Makefile.arm9 --no-print-directory DEBUG=false

arm7d: libntxm
	$(V)+$(MAKE) -f Makefile.arm7 --no-print-directory DEBUG=true MIDI=true

arm7: libntxm
	$(V)+$(MAKE) -f Makefile.arm7 --no-print-directory DEBUG=false MIDI=true

arm7l: libntxm
	$(V)+$(MAKE) -f Makefile.arm7 --no-print-directory DEBUG=false

ifneq ($(strip $(NITROFSDIR)),)
# Additional arguments for ndstool
NDSTOOL_FAT	:= -d $(NITROFSDIR)

# Make the NDS ROM depend on the filesystem image only if it is needed
$(ROM): $(NITROFSDIR)
$(ROM_DEBUG): $(NITROFSDIR)
$(ROM_NOMIDI): $(NITROFSDIR)
endif

$(ROM): arm9 arm7
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 build/arm7.elf -9 build/arm9.elf \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE_MIDI)" \
		$(NDSTOOL_FAT)

$(ROM_DEBUG): arm9d arm7d
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 build/arm7d.elf -9 build/arm9d.elf \
		-b $(GAME_ICON_DEBUG) "$(GAME_FULL_TITLE_DEBUG)" \
		$(NDSTOOL_FAT)

$(ROM_NOMIDI): arm9l arm7l
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 build/arm7l.elf -9 build/arm9l.elf \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		$(NDSTOOL_FAT)

format:
	@find common/source n3ds/source nds/arm7/source nds/arm9/source sdl/include sdl/source tobkit/include tobkit/source -iname '*.h' -o -iname '*.cpp' -o -iname '*.c' | xargs clang-format -i
