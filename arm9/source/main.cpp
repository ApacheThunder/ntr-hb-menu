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
#include "tonccpy.h"

/*#define NDS_HEADER			0x02FFFE00
#define NDS_HEADER_POKEMON	0x02FFF000
#define TWL_HEADER			0x02FFE000*/

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

ITCM_CODE const char* getRomTid(const tNDSHeader* ndsHeader) {
	static char romTid[5];
	strncpy(romTid, ndsHeader->gameCode, 4);
	romTid[4] = '\0';
	return romTid;
}

/*ITCM_CODE void setMemoryAddress(const tNDSHeader* ndsHeader, u32 ChipID) {
	if (ndsHeader->unitCode & BIT(1)) {
		tonccpy((u32*)0x027FFA80, (u32*)ndsHeader, 0x160);	// Make a duplicate of DS header

		*(vu32*)(0x027FA680) = 0x02FD4D80;
		*(vu32*)(0x027FA684) = 0x00000000;
		*(vu32*)(0x027FA688) = 0x00001980;

		*(vu32*)(0x027FF00C) = 0x0000007F;
		*(vu32*)(0x027FF010) = 0x550E25B8;
		*(vu32*)(0x027FF014) = 0x02FF4000;

		// Set region flag
		if (strncmp(getRomTid(ndsHeader)+3, "J", 1) == 0) {
			*(vu8*)(0x027FFD70) = 0;
		} else if (strncmp(getRomTid(ndsHeader)+3, "E", 1) == 0) {
			*(vu8*)(0x027FFD70) = 1;
		} else if (strncmp(getRomTid(ndsHeader)+3, "P", 1) == 0) {
			*(vu8*)(0x027FFD70) = 2;
		} else if (strncmp(getRomTid(ndsHeader)+3, "U", 1) == 0) {
			*(vu8*)(0x027FFD70) = 3;
		} else if (strncmp(getRomTid(ndsHeader)+3, "C", 1) == 0) {
			*(vu8*)(0x027FFD70) = 4;
		} else if (strncmp(getRomTid(ndsHeader)+3, "K", 1) == 0) {
			*(vu8*)(0x027FFD70) = 5;
		}
	}
	
    // Set memory values expected by loaded NDS
    // from NitroHax, thanks to Chism
	*((vu32*)0x027FF800) = ChipID;					// CurrentCardID
	*((vu32*)0x027FF804) = ChipID;					// Command10CardID
	*((vu16*)0x027FF808) = ndsHeader->headerCRC16;	// Header Checksum, CRC-16 of [000h-15Dh]
	*((vu16*)0x027FF80A) = ndsHeader->secureCRC16;	// Secure Area Checksum, CRC-16 of [ [20h]..7FFFh]
	*((vu16*)0x027FF850) = 0x5835;
	// Copies of above
	*((vu32*)0x027FFC00) = ChipID;					// CurrentCardID
	*((vu32*)0x027FFC04) = ChipID;					// Command10CardID
	*((vu16*)0x027FFC08) = ndsHeader->headerCRC16;	// Header Checksum, CRC-16 of [000h-15Dh]
	*((vu16*)0x027FFC0A) = ndsHeader->secureCRC16;	// Secure Area Checksum, CRC-16 of [ [20h]..7FFFh]
	*((vu16*)0x027FFC10) = 0x5835;
	*((vu16*)0x027FFC40) = 0x01;						// Boot Indicator -- EXTREMELY IMPORTANT!!! Thanks to cReDiAr
	
	tonccpy((u32*)0x027FC000, (u32*)InitialCartHeaderTWL, 0x1000);
	tonccpy((u32*)0x027FF000, (u32*)NDS_HEADER_POKEMON, 0x170);
	tonccpy((u32*)0x027FFE00, (u32*)NDS_HEADER, 0x160);
	tonccpy((u32*)0x027FE000, (u32*)TWL_HEADER, 0x1000);
		
	tonccpy((u32*)0x027FF830, (u32*)0x02FFF830, 0x20);
	tonccpy((u32*)0x027FFC80, (u32*)0x02FFFC80, 0x70);
	tonccpy((u32*)0x027FFD80, (u32*)0x02FFFD80, 0x70);
}*/

/*void MaybeCardInit(bool doInit) {
	if (doInit) {
		// *(u32*)0x02000010 = 0xFFFFFFFF;
		swiWaitForVBlank();
		// Arm7 needs to do it's card init first
		fifoSendValue32(FIFO_USER_01, 1);
		swiWaitForVBlank();
		fifoWaitValue32(FIFO_USER_02);
		swiWaitForVBlank();
		cardInit((sNDSHeaderExt*)InitialCartHeaderTWL);
	} else {
		// fifoSendValue32(FIFO_USER_03, 1);
		// *(u32*)0x02000010 = 0;
		swiWaitForVBlank();
		fifoSendValue32(FIFO_USER_01, 1);
		swiWaitForVBlank();
		fifoWaitValue32(FIFO_USER_02);
		swiWaitForVBlank();
	}
}*/


bool InitSlot1DLDI() {
	sysSetCardOwner(BUS_OWNER_ARM9);
	
	if (REG_SCFG_MC == 0x11)return false;
	
	bool CardReset = false;
	
	// if (REG_SCFG_MC == 0x10) {
		// MaybeCardInit(true);
		// CardReset = true;
	// }
	
		
	if (access("sd:/slot1.dldi", F_OK) == 0) {
		if (!CardReset) {
			for (int i = 0; i < 10; i++)swiWaitForVBlank();
		}
		myDldiLoadFromFile("sd:/slot1.dldi");
		return true;
	}
	
	sNDSHeaderExt* cartHeader = (sNDSHeaderExt*)InitialCartHeaderTWL;
	
	// setMemoryAddress((tNDSHeader*)cartHeader, *(vu32*)InitialCartChipID);
		
	if (!memcmp(cartHeader->gameCode, "ASMA", 4)) {
		cardInit((sNDSHeaderExt*)InitialCartHeaderTWL);
		if (!memcmp(cartHeader->gameTitle, "MEDIAPLAYER", 11)) { dldiLoadFromBin(gmtf_dldi); } else { dldiLoadFromBin(r4tf_dldi); }
		return true;
	} else if (!memcmp(cartHeader->gameCode, "ASME", 4)) {
		cardInit((sNDSHeaderExt*)InitialCartHeaderTWL);
		dldiLoadFromBin(cyclods_dldi);
		return true;
	} else if (!memcmp(cartHeader->gameCode, "ACEK", 4) || !memcmp(cartHeader->gameCode, "YCEP", 4) || !memcmp(cartHeader->gameCode, "AHZH", 4) || 
			   !memcmp(cartHeader->gameCode, "CHPJ", 4) || !memcmp(cartHeader->gameCode, "ADLP", 4) ||
			   !memcmp(cartHeader->gameTitle, "QMATETRIAL", 10) || !memcmp(cartHeader->gameTitle, "R4DSULTRA", 9) // R4iDSN/R4 Ultra
	) {
		// if (!CardReset) { MaybeCardInit(true); } else { MaybeCardInit(false); }
		dldiLoadFromBin(ak2_dldi);
		return true;
	} else if (!memcmp(cartHeader->gameCode, "AMFE", 4)) {
		// MaybeCardInit(false);
		dldiLoadFromBin(m3ds_dldi);
		return true;
	} else if (!memcmp(cartHeader->gameCode, "ABJJ", 4)) {
		// MaybeCardInit(false);
		dldiLoadFromBin(ez5n_dldi);
		return true;
	} else if (!memcmp(cartHeader->gameCode, "AL3E", 4)) {
		// MaybeCardInit(false);
		dldiLoadFromBin(acep_dldi);
		return true;
	} else if (!memcmp(cartHeader->gameCode, "ALXX", 4)) {
		// if (!CardReset) { MaybeCardInit(true); } else { MaybeCardInit(false); }
		dldiLoadFromBin(ds2_dldi);
		return true;
	// } else if (!memcmp(cartHeader->gameCode, "TTDS", 4)) {
	} else {
		if (!CardReset) {
			for (int i = 0; i < 10; i++)swiWaitForVBlank();
		}
		// MaybeCardInit(false);
		dldiLoadFromBin(ttio_dldi);
		return true;
	}
}


int main(void) {
	/*extern u64 *fake_heap_end;
	*fake_heap_end = 0;
		
	
	*(u32*)0x02000010 = 0;*/
	defaultExceptionHandler();
	
	fifoWaitValue32(FIFO_USER_01);
	
	bool skipSlot1Init = false;
	
	u32 currentButton = 0;
	
	if (!isDSiMode()) {
		InitGUI();
		printf ("\n\n\n\n\n\n\n\n\n\n      Unsupported Console!\n");
		return exitProgram();
	}
	
	/*scanKeys();
	swiWaitForVBlank();
	currentButton = keysDown();
	switch (currentButton) {
		case KEY_X: { skipSlot1Init = true; } break;
	}*/
	
	if (!skipSlot1Init && InitSlot1DLDI())slot1Available = fatMountSimple("fat", dldiGet());

	if (autoBoot && slot1Available) {
		scanKeys();
		swiWaitForVBlank();
		currentButton = keysDown();
		switch (currentButton) {
			case 0: {
				if(access("fat:/_picoboot.nds", F_OK) == 0) {
					usingSD = false;
					const char *argarray[1] = { "fat:/_picoboot.nds" };
					err = runNdsFile("fat:/_picoboot.nds", 1, argarray);
					return exitProgram();
				}
			}
		}
	}
	
	dsiSDAvailable = fatMountSimple("sd", get_io_dsisd());
	
	if (!slot1Available && !dsiSDAvailable) {
		InitGUI();
		printf ("\n\n\n\n\n\n\n\n\n\n     No filesystems found!\n");
		return exitProgram();
	}
	
	err = FileBrowser();
	return exitProgram();
}

