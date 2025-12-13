#include <nds.h>
#include <nds/memory.h>

#include <maxmod7.h>

#include "read_card.h"

#define InitialCartHeaderTWL 0x02FFC000 // System Menu keeps cart's header here (if cart is present) on initial boot of any DSiWare!

volatile bool exitflag = false;

void powerButtonCB() { exitflag = true; }

void VblankHandler() { }
void VcountHandler() { inputGetAndSend(); }

/*int WaitForArm9Check() {
	fifoWaitValue32(FIFO_USER_01);
	swiWaitForVBlank();
	// if (fifoCheckValue32(FIFO_USER_03))cardInit((sNDSHeaderExt*)InitialCartHeaderTWL);
	// if (*(u32*)0x02000010 == 0xFFFFFFFF)cardInit((sNDSHeaderExt*)InitialCartHeaderTWL);
	cardInit((sNDSHeaderExt*)InitialCartHeaderTWL);
	fifoSendValue32(FIFO_USER_02, 1);
	swiWaitForVBlank();
	while(1)swiWaitForVBlank();
	return 0;
}*/

int main(void) {
	readUserSettings();
	ledBlink(0);
	
	irqInit();
	
	initClockIRQ();
	fifoInit();
	touchInit();
	
	mmInstall(FIFO_MAXMOD);
	
	SetYtrigger(80);
	
	installSoundFIFO();
	installSystemFIFO();
	
	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);
	
	irqEnable(IRQ_VBLANK | IRQ_VCOUNT);
	
	setPowerButtonCB(powerButtonCB);
	
	if (isDSiMode()) {
		i2cWriteRegister(0x4A, 0x12, 0x00);	// Press power-button for auto-reset
		i2cWriteRegister(0x4A, 0x70, 0x01);	// Bootflag = Warmboot/SkipHealthSafety
	}
	
	cardInit((sNDSHeaderExt*)InitialCartHeaderTWL);

	fifoSendValue32(FIFO_USER_01, 1);
	
	swiWaitForVBlank();
	// return WaitForArm9Check();
	while(1)swiWaitForVBlank();
	return 0;
}

