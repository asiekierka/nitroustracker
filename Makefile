# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2023

BLOCKSDS	?= /opt/blocksds/core
BLOCKSDSEXT	?= /opt/blocksds/external

# User config
# ===========

NAME		:= nitroustracker

GAME_TITLE	:= NitrousTracker
GAME_SUBTITLE	:= v0.4.9
GAME_ICON	:= icon.bmp

# DLDI and internal SD slot of DSi
# --------------------------------

# Root folder of the SD image
SDROOT		:= sdroot
# Name of the generated image it "DSi-1.sd" for no$gba in DSi mode
SDIMAGE		:= image.bin

# Source code paths
# -----------------

NITROFATDIR	:=

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
ROM		:= $(NAME).nds
ROM_DEBUG	:= $(NAME).debug.nds
ROM_NOWIFI	:= $(NAME).nowifi.nds

# Targets
# -------

.PHONY: all clean arm9d arm9 arm9l arm7d arm7 arm7l libntxm tobkit tobkit-debug libdsmi sdimage

all: $(ROM) $(ROM_DEBUG) $(ROM_NOWIFI)

clean:
	@echo "  CLEAN"
	$(V)$(MAKE) -f Makefile.arm9 clean --no-print-directory DEBUG=true WIFI=true
	$(V)$(MAKE) -f Makefile.arm9 clean --no-print-directory DEBUG=false
	$(V)$(MAKE) -f Makefile.arm9 clean --no-print-directory DEBUG=false WIFI=true
	$(V)$(MAKE) -f Makefile.arm7 clean --no-print-directory DEBUG=true WIFI=true
	$(V)$(MAKE) -f Makefile.arm7 clean --no-print-directory DEBUG=false
	$(V)$(MAKE) -f Makefile.arm7 clean --no-print-directory DEBUG=false WIFI=true
	$(V)$(MAKE) -C tobkit clean DEBUG=true
	$(V)$(MAKE) -C tobkit clean DEBUG=false
	$(V)$(MAKE) -C dsmi/ds/libdsmi -f Makefile.blocks clean
	$(V)$(MAKE) -C libntxm/libntxm -f Makefile.blocks clean
	$(V)$(RM) $(ROM) build $(SDIMAGE)

libntxm:
	@make -C libntxm/libntxm -f Makefile.blocks

tobkit:
	@make -C tobkit

tobkit-debug:
	@make -C tobkit DEBUG=true

libdsmi:
	@make -C dsmi/ds/libdsmi -f Makefile.blocks

arm9d: tobkit-debug libntxm libdsmi
	$(V)+$(MAKE) -f Makefile.arm9 --no-print-directory DEBUG=true WIFI=true

arm9: tobkit libntxm libdsmi
	$(V)+$(MAKE) -f Makefile.arm9 --no-print-directory DEBUG=false WIFI=true

arm9l: tobkit libntxm libdsmi
	$(V)+$(MAKE) -f Makefile.arm9 --no-print-directory DEBUG=false

arm7d: libntxm
	$(V)+$(MAKE) -f Makefile.arm7 --no-print-directory DEBUG=true WIFI=true

arm7: libntxm
	$(V)+$(MAKE) -f Makefile.arm7 --no-print-directory DEBUG=false WIFI=true

arm7l: libntxm
	$(V)+$(MAKE) -f Makefile.arm7 --no-print-directory DEBUG=false

ifneq ($(strip $(NITROFATDIR)),)
# Additional arguments for ndstool
NDSTOOL_FAT	:= -F $(NITROFAT_IMG)

$(NITROFAT_IMG): $(NITROFATDIR)
	@echo "  MKFATIMG $@ $(NITROFATDIR)"
	$(V)$(BLOCKSDS)/tools/mkfatimg/mkfatimg -t $(NITROFATDIR) $@ 0

# Make the NDS ROM depend on the filesystem image only if it is needed
$(ROM): $(NITROFAT_IMG)
$(ROM_DEBUG): $(NITROFAT_IMG)
endif

# Combine the title strings
ifeq ($(strip $(GAME_SUBTITLE)),)
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_AUTHOR)
else
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_SUBTITLE);$(GAME_AUTHOR)
endif

$(ROM): arm9 arm7
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 build/arm7.elf -9 build/arm9.elf \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		$(NDSTOOL_FAT)

$(ROM_DEBUG): arm9d arm7d
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 build/arm7d.elf -9 build/arm9d.elf \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		$(NDSTOOL_FAT)

$(ROM_NOWIFI): arm9l arm7l
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 build/arm7l.elf -9 build/arm9l.elf \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		$(NDSTOOL_FAT)

sdimage:
	@echo "  MKFATIMG $(SDIMAGE) $(SDROOT)"
	$(V)$(BLOCKSDS)/tools/mkfatimg/mkfatimg -t $(SDROOT) $(SDIMAGE) 0
