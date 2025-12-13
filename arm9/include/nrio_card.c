#include <nds.h>
#include <nds/ndstypes.h>
#include <nds/system.h>
#include <nds/card.h>
#include <stdio.h>

#include "tonccpy.h"
#include "read_card.h"

#define INITBUFFER  0x02000000

#define NANDREADPAGE_D2 0xD2 // NAND Read Page Command
#define CARDD2FLAGS (u32)0xB918027E
#define CARDB7FLAGS (u32)0xB9180000

volatile u32 CARD_CR2_D2 = 0;


// Final C1 command now reads from 0x08 location of header like origional stage1 main rom does instead of hardcoding.
// Some different capacity cards are known to use different values. 1080 = what most 16g carts use. Some 2G carts also use this value

ITCM_CODE u32 CalcCARD_CR2_D2() {
	u32 da,db,dc,ncr2;
	da = CARDD2FLAGS; // Originally obtained from 0x27FFE60?
	db = CARDD2FLAGS;
	da = (da & 0x00001FFF);
	db = (db & 0x003F0000);
	db = (db >> 16);
	dc = (da + db);
	dc = (dc/2)-1;
	ncr2 = CARDD2FLAGS;
	ncr2 &=~ 0x003F1FFF;
	ncr2 |= dc;
	ncr2 = ((u32)((ncr2) & (~0x07000000)) | (3<<24));
	return ncr2;
}

ITCM_CODE void InitCartNandReadMode(u32 CardType) {
	// Only bricked carts would present values like these. Hard Code 1083 if this is the case
	// FYI only other known value that can exist here is 0x10430000 which is used by 1G N-Cards? Only seen it once with a 1g N-Card setup as a bootleg game.
	// Because of how rare 1G cards would be the default value will be 0x10830000 which 2G, 8G, and 16G cards are known to use.
	// In theory a 32G card could end up using 0x10C30000. Interestingly enough using this value on a 16G cart does not break nrioTools ability to return correct data.
	// (using 0x10430000 however does result in just FF data being returned from a 16G cart)
	switch (CardType) {
		case 0x00000000: { CardType = 0x10830000; }break;
		case 0xFFFFFFFF: { CardType = 0x10830000; }break; 
	}
	
	// Original init sequence from stage1 main rom before it goes to read stage2/retail game from nand as defined by a block stbale stored at 0x8000
	cardParamCommand (0x66, 0, CARD_ACTIVATE | CARD_nRESET | CARD_BLK_SIZE(7) | CARD_SEC_CMD | BIT(20) | BIT(19) | BIT(14) | BIT(13), (u32*)INITBUFFER, 128);
	cardParamCommand (0xC1, 0x0D210000, CARD_ACTIVATE | CARD_nRESET | CARD_BLK_SIZE(7) | BIT(17), (u32*)(INITBUFFER + 0x10), 128);
	cardParamCommand (0xC1, 0x0FB00000, CARD_ACTIVATE | CARD_nRESET | CARD_BLK_SIZE(7) | BIT(17), (u32*)(INITBUFFER + 0x10), 128);
	cardParamCommand (0xC1, CardType, CARD_ACTIVATE | CARD_nRESET | CARD_BLK_SIZE(7) | BIT(17), (u32*)(INITBUFFER + 0x20), 128);
	
	CARD_CR2_D2 = CalcCARD_CR2_D2();
}

/*ITCM_CODE void cardreadpage(u32 addr, u32 dst, u8 cmd, u32 card_cr2) {
	cardParamCommand (cmd, addr, card_cr2, (u32*)dst, 128);
}

ITCM_CODE void nrio_readSector(void* destination, u32 rom_offset) {
	// cardreadpage(rom_offset, (u32)destination, NANDREADPAGE_D2, CalcCARD_CR2_D2());
	cardreadpage(rom_offset, (u32)destination, NANDREADPAGE_D2, CARD_CR2_D2);
}

ITCM_CODE void nrio_readSectorB7(void* destination, u32 rom_offset) {
	cardreadpage(rom_offset, (u32)destination, CARD_CMD_DATA_READ, CARDB7FLAGS);
}*/

