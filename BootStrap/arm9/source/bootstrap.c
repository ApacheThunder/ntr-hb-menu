/*-----------------------------------------------------------------

 Copyright (C) 2010  Dave "WinterMute" Murphy

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
#include <fat.h>

#include <stdio.h>

#include "nds_loader_arm9.h"


void DisplayError(bool fatInitFailed) {
	consoleDemoInit();
	printf("hbmenu bootstrap ...\n\n");
	if (fatInitFailed) {
		printf("FAT init failed!\n");
	} else {
		printf("Load NDS file Error!\n");
	}
}

int main(int argc, char **argv) {
	if (fatInitDefault()) {
		if (runNdsFile("/BOOT.NDS", 0, NULL) != 0)DisplayError(true);
	} else {
		DisplayError(false);
	}
	while(1)swiWaitForVBlank();
}

