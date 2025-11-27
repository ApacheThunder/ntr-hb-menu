# HBMenu Deluxe

This is a test build of an enhanced version of HBMenu with DS Firwmare style graphics/sounds. This version is able to navigate two different devices via the shoulder buttons.

This is exclusively a DSi/3DS app and is not intended to be used on DS/DS lite.

Currently this is setup to auto boot _picoboot.nds from flashcart's storage if available. Otherwise it will display the filebrowser. Shoulder buttons are used to switch filesystems from DSi SD to flashcart's storage.

Only R4 and DSTT is setup though currently for some reason the bootloader hangs trying to launch anything from R4. Booting things from DSTT/DSTTi (and their clones) appears to work however.

Holding B brings up the file menu instead of autobooting. I setup an autoboot bool in main.cpp that you can set to false and recompile if you want a more generalized normal version of HBMenu that isn't setup to autoboot anything.

# License
Note: While the GPL license allows you to distribute modified versions of this program it would be appreciated if any improvements are contributed to devkitPro. Ultimately the community as a whole is better served by having a single official source for tools, applications and libraries.

The latest sources may be obtained from devkitPro git using the command: `git clone git@github.com:devkitPro/nds-hb-menu.git`

```
 Copyright (C) 2005 - 2017
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy

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
 ```
