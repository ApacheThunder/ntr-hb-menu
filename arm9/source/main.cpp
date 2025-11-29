/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/
#include <nds.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>

#include "args.h"
#include "file_browse.h"
#include "font.h"
#include "hbmenu_consolebg.h"

#include "iconTitle.h"
#include "nds_loader_arm9.h"
#include "read_card.h"
#include "ndsheaderbanner.h"
#include "dldi_tools.h"
#include "dldi_binaries.h"
#include "launcherData.h"

using namespace std;

volatile int err = 0;
volatile bool GUIINIT = false;
volatile bool usingSD = false;
volatile bool slot1Available = false;
volatile bool dsiSDAvailable = false;

const bool autoBoot = true;

void InitGUI (void) {
	if (GUIINIT)return;
	GUIINIT = true;
	iconTitleInit();
	videoSetModeSub(MODE_4_2D);
	vramSetBankC(VRAM_C_SUB_BG);
	int bgSub = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 1, 0);
	PrintConsole *console = consoleInit(0, 0, BgType_Text4bpp, BgSize_T_256x256, 4, 6, false, false);
	dmaCopy(hbmenu_consolebgBitmap, bgGetGfxPtr(bgSub), 256*256);
	ConsoleFont font;
	font.gfx = (u16*)fontTiles;
	font.pal = (u16*)fontPal;
	font.numChars = 95;
	font.numColors = (fontPalLen / 2);
	font.bpp = 4;
	font.asciiOffset = 32;
	font.convertSingleColor = true;
	consoleSetFont(console, &font);
	dmaCopy(hbmenu_consolebgPal, BG_PALETTE_SUB, 256*2);
	BG_PALETTE_SUB[255] = RGB15(31,31,31);
	keysSetRepeat(25,5);
	consoleSetWindow(console, 1, 1, 30, 22);
}


int exitProgram(void) {
	if (!GUIINIT)InitGUI();
	if (err != 0)iprintf("Bootloader returned error %d\n", err);
	// iprintf("Press A to power off.");
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		if (!keysHeld())break;
	}
	while (1) {
		swiWaitForVBlank();
		scanKeys();
		if (keysDown() != 0)break;
	}
	systemShutDown();
	return 0;
}

int FileBrowser() {
	InitGUI();
	consoleClear();
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		if (!keysHeld())break;
	}
		
	if (slot1Available) {
		if (access("fat:/", F_OK) == 0)chdir("fat:/");
	} else {
		if (access("sd:/", F_OK) == 0)chdir("sd:/");	
	}
	
	vector<string> extensionList = argsGetExtensionList();
	
	while(1) {
		string filename = browseForFile(extensionList);
		// Construct a command line
		vector<string> argarray;
		if (!argsFillArray(filename, argarray)) {
			printf("Invalid NDS or arg file selected\n");
		} else {
			iprintf("Running %s with %d parameters\n", argarray[0].c_str(), argarray.size());
			// Make a copy of argarray using C strings, for the sake of runNdsFile
			vector<const char*> c_args;
			for (const auto& arg: argarray)c_args.push_back(arg.c_str());
			// Try to run the NDS file with the given arguments
			int err = runNdsFile(c_args[0], c_args.size(), &c_args[0]);
			iprintf("Start failed. Error %i\n", err);
		}
		argarray.clear();
	}
	return 0;
}


bool InitSlot1DLDI() {
	if (REG_SCFG_MC == 0x11)return false;
	
	bool CardReset = false;
	
	sNDSHeaderExt ntrHeader;
	
	if (REG_SCFG_MC == 0x10) {
		cardInit(&ntrHeader); // Original R4 needs card init for some cursed reason.
		for (int i = 0; i < 30; i++)swiWaitForVBlank();
		CardReset = true;
	}
		
	if (access("sd:/slot1.dldi", F_OK) == 0) {
		if (!CardReset)swiWaitForVBlank();
		myDldiLoadFromFile("sd:/slot1.dldi");
		return true;
	}
	
	sNDSHeaderExt* cartHeader = (sNDSHeaderExt*)InitialCartHeaderTWL;
		
	if (!memcmp(cartHeader->gameCode, "ASMA", 4)) {
		if (!memcmp(cartHeader->gameTitle, "MEDIAPLAYER", 11)) { dldiLoadFromBin(gmtf_dldi); } else { dldiLoadFromBin(r4tf_dldi); }
		return true;
	} else if (!memcmp(cartHeader->gameCode, "ACEK", 4) || !memcmp(cartHeader->gameCode, "YCEP", 4) || !memcmp(cartHeader->gameCode, "AHZH", 4) || 
			   !memcmp(cartHeader->gameCode, "CHPJ", 4) || !memcmp(cartHeader->gameCode, "ADLP", 4) ||
			   !memcmp(cartHeader->gameTitle, "QMATETRIAL", 10) || !memcmp(cartHeader->gameTitle, "R4DSULTRA", 9) // R4iDSN/R4 Ultra
	) {
		dldiLoadFromBin(ak2_dldi);
		return true;
	} else if (!memcmp(cartHeader->gameCode, "AMFE", 4)) {
		dldiLoadFromBin(m3ds_dldi);
		return true;
	} else if (!memcmp(cartHeader->gameCode, "ABJJ", 4)) {
		dldiLoadFromBin(ez5n_dldi);
		return true;
	} else /*if (!memcmp(cartHeader->gameCode, "TTDS", 4))*/ {
		if (!CardReset) {
			for (int i = 0; i < 10; i++)swiWaitForVBlank();
		}
		dldiLoadFromBin(ttio_dldi);
		return true;
	}
	
	/*return false;*/
}


int main(void) {
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;
	
	defaultExceptionHandler();
	
	if (!isDSiMode()) {
		InitGUI();
		printf ("\n\n\n\n\n\n\n\n\n\n      Unsupported Console!\n");
		return exitProgram();
	}
	
	sysSetCardOwner(BUS_OWNER_ARM9);
	
	if (InitSlot1DLDI()) {
		if (fatMountSimple("fat", dldiGet()))slot1Available = true;
	}
	
	dsiSDAvailable = fatMountSimple("sd", get_io_dsisd());
	
	
	if (!slot1Available && !dsiSDAvailable) {
		InitGUI();
		printf ("\n\n\n\n\n\n\n\n\n\n     No filesystems found!\n");
		return exitProgram();
	}
	
	if (autoBoot && slot1Available) {
		scanKeys();
		swiWaitForVBlank();
		u32 KeysDown = keysDown();
		switch (KeysDown) {
			case 0: {
				if(access("fat:/_picoboot.nds", F_OK) == 0) {
					usingSD = false;
					const char *argarray[1] = { "fat:/_picoboot.nds" };
					err = runNdsFile("fat:/_picoboot.nds", 1, argarray);
					return exitProgram();
				}
			}
			default: {
				err = FileBrowser();
				return exitProgram();
			} break;
		}
	}
	err = FileBrowser();
	return exitProgram();
}

