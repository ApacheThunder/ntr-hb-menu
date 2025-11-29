#include <nds/arm9/dldi.h>

void dldiLoadFromBin (const u8 dldiAddr[]);
void myDldiLoadFromFile (const char* filepath);

void ntrCardReset();

const DISC_INTERFACE *dldiGet(void) {
	if(io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA)sysSetCartOwner(BUS_OWNER_ARM9);
	if(io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS)sysSetCardOwner(BUS_OWNER_ARM9);
	return &io_dldi_data->ioInterface;
}

