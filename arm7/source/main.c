#include <nds.h>

#include <maxmod7.h>

volatile bool exitflag = false;

void powerButtonCB() { exitflag = true; }

void VblankHandler() { }
void VcountHandler() { inputGetAndSend(); }

int main(void) {
	readUserSettings();
	ledBlink(0);
	
	irqInit();
	
	initClockIRQ();
	fifoInit();
	// touchInit();
	
	mmInstall(FIFO_MAXMOD);
	
	SetYtrigger(80);
	
	installSoundFIFO();
	installSystemFIFO();
	
	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);
	
	irqEnable(IRQ_VBLANK | IRQ_VCOUNT);
	
	setPowerButtonCB(powerButtonCB);
	
	i2cWriteRegister(0x4A, 0x12, 0x00);	// Press power-button for auto-reset
	i2cWriteRegister(0x4A, 0x70, 0x01);	// Bootflag = Warmboot/SkipHealthSafety
	
	while(1)swiWaitForVBlank();
	return 0;
}

