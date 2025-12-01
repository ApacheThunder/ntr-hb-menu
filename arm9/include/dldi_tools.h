#include <nds/arm9/dldi.h>

typedef signed int addr_t;
typedef unsigned char data_t;

void dldiLoadFromBin (const u8 dldiAddr[]);
void myDldiLoadFromFile (const char* filepath);
void dldiRelocateBinary (data_t *binData, size_t dldiFileSize);

void ntrCardReset();

const DISC_INTERFACE *dldiGet(void);

