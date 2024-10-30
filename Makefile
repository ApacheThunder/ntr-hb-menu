#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET := hbmenu
export TOPDIR := $(CURDIR)

export HBMENU_MAJOR	:= 0
export HBMENU_MINOR	:= 9
export HBMENU_PATCH	:= 3


VERSION	:=	$(HBMENU_MAJOR).$(HBMENU_MINOR).$(HBMENU_PATCH)

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

# .PHONY: data ndsbootloader bootstub BootStrap exceptionstub clean
.PHONY: data ndsbootloader bootstub BootStrap clean

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
# all: ndsbootloader bootstub exceptionstub $(TARGET).nds BootStrap
all: ndsbootloader bootstub $(TARGET).nds BootStrap

cia:
	$(MAKE) -C BootStrap bootstrap.cia

dist:	all
	rm	-fr	hbmenu
	mkdir -p hbmenu/nds
	ndstool	-c boot.nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf \
			-b $(CURDIR)/icon.bmp "hbmenu;$(VERSION);http://devkitpro.org" \
			-g #### 01 "HBMENU" -z 80040407 -u 00030015 -a 001FFFFF -p 0001
	cp boot.nds hbmenu/BOOT.NDS
	cp BootStrap/_BOOT_MP.NDS BootStrap/TTMENU.DAT BootStrap/_ds_menu.dat BootStrap/ez5sys.bin BootStrap/akmenu4.nds BootStrap/ismat.dat hbmenu
	cp -r BootStrap/ACE3DS hbmenu
ifneq (,$(wildcard BootStrap/bootstrap.cia))
	cp "BootStrap/bootstrap.cia" hbmenu
endif
	cp testfiles/* hbmenu/nds
	zip -9r hbmenu-$(VERSION).zip hbmenu README.md COPYING
#---------------------------------------------------------------------------------
checkarm7:
	$(MAKE) -C arm7

#---------------------------------------------------------------------------------
checkarm9:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
$(TARGET).nds : $(NITRO_FILES) arm7/$(TARGET).elf arm9/$(TARGET).elf
	ndstool	-c $(TARGET).nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf \
			-b $(CURDIR)/icon.bmp "hbmenu;$(VERSION);http://devkitpro.org" \
			-g HOME 01 "HBMENU" -z 80040407 -u 00030015 -a 001FFFFF -p 0001
	@cp $(TARGET).nds 00000000.app

data:
	@mkdir -p data

ndsbootloader: data
	$(MAKE) -C ndsbootloader LOADBIN=$(CURDIR)/data/load.bin
	
# exceptionstub: data
#	$(MAKE) -C exception-stub STUBBIN=$(CURDIR)/data/exceptionstub.bin

bootstub: data
	$(MAKE) -C bootstub
	
BootStrap: data
	$(MAKE) -C BootStrap

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	$(MAKE) -C arm7

#---------------------------------------------------------------------------------
arm9/$(TARGET).elf: ndsbootloader
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	$(MAKE) -C ndsbootloader clean
	$(MAKE) -C bootstub clean
	$(MAKE) -C BootStrap clean
#	$(MAKE) -C exception-stub clean
	rm -rf data
	rm -rf hbmenu
	rm -f $(TARGET).nds
	rm -f boot.nds
	rm -f 00000000.app

