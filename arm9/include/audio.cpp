/*
    NitroHax -- Cheat tool for the Nintendo DS
    Copyright (C) 2008  Michael "Chishm" Chisholm

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stddef.h>
#include <nds.h>
#include <maxmod9.h>

#include "soundbank.h"
#include "soundbank_bin.h"

static mm_sound_effect dsboot;
static mm_sound_effect dsiboot;
static mm_sound_effect sfxBack;
static mm_sound_effect sfxLaunch;
static mm_sound_effect sfxSelect;
static mm_sound_effect sfxWrong;

static bool audioReady = false;

void InitAudio() {
	mmInitDefaultMem((mm_addr)soundbank_bin);
	
	mmLoadEffect( SFX_DSBOOT );
	mmLoadEffect( SFX_DSIBOOT );
	mmLoadEffect( SFX_BACK );
	mmLoadEffect( SFX_LAUNCH );
	mmLoadEffect( SFX_SELECT );
	mmLoadEffect( SFX_WRONG );
	
	dsiboot = {
		{ SFX_DSIBOOT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	
	dsboot = {
		{ SFX_DSBOOT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	
	sfxBack = {
		{ SFX_BACK } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	
	sfxLaunch = {
		{ SFX_LAUNCH } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	
	sfxSelect = {
		{ SFX_SELECT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	
	sfxWrong = {
		{ SFX_WRONG } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	audioReady = true;
}

void PlayBootJingle() {
	if (!audioReady)InitAudio();
	mmEffectEx(&dsboot);
}

void PlayBootJingleDSi() {
	if (!audioReady)InitAudio();
	mmEffectEx(&dsiboot);
}

void PlayLaunchSFX() {
	if (!audioReady)InitAudio();
	mmEffectEx(&sfxLaunch);
}

void PlayWrongSFX() {
	if (!audioReady)InitAudio();
	mmEffectEx(&sfxWrong);
}

void PlaySelectSFX() {
	if (!audioReady)InitAudio();
	mmEffectEx(&sfxSelect);
}

void PlayBackSFX() {
	if (!audioReady)InitAudio();
	mmEffectEx(&sfxBack);
}

