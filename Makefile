#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET := HBMenuDeluxe
export TOPDIR := $(CURDIR)

# GMAE_ICON is the image used to create the game icon, leave blank to use default rule
GAME_ICON :=

# specify a directory which contains the nitro filesystem
# this is relative to the Makefile
NITRO_FILES :=

# These set the information text in the nds file
#GAME_TITLE     := My Wonderful Homebrew
#GAME_SUBTITLE1 := built with devkitARM
#GAME_SUBTITLE2 := http://devitpro.org

include $(DEVKITARM)/ds_rules

.PHONY: data bootloader clean

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: $(TARGET).nds

#---------------------------------------------------------------------------------
checkarm7:
	$(MAKE) -C arm7

#---------------------------------------------------------------------------------
checkarm9:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
$(TARGET).nds : $(NITRO_FILES) arm7/$(TARGET).elf arm9/$(TARGET).elf
	@ndstool	-c $@ -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf \
	-g HBDX 01 "HBMENU DX" -z 80040407 -u 00030004 -a 00000138 -p 0001 \
	-t banner.bin
	$(_ADDFILES)
	@cp $(TARGET).nds 00000000.app

#	-g HBDX 01 "HBMENU DX" -z 80040407 -u 00030004 -a 00000138 -p 0001 \

data:
	@mkdir -p data

bootloader: data
	$(MAKE) -C bootloader LOADBIN=$(TOPDIR)/data/load.bin

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	$(MAKE) -C arm7

#---------------------------------------------------------------------------------
arm9/$(TARGET).elf: bootloader
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	$(MAKE) -C bootloader clean
	rm -rf data
	rm -f $(TARGET).nds
	rm -f 00000000.app
