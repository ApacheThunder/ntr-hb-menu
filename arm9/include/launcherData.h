#ifndef LAUNCHERDATA_H
#define LAUNCHERDATA_H

#ifdef __cplusplus
extern "C" {
#endif

#define CartHeaderCopy 0x02000000
#define CartChipIDCopy 0x02000180

// #define LAUNCH_DATA 0x020007F0

// #define InitialCartBannerOffset 0x02FFC068
#define InitialCartHeaderTWL 0x02FFC000 // System Menu keeps cart's header here (if cart is present) on initial boot of any DSiWare!
#define InitialCartHeader 0x02FFFA80 // System Menu keeps cart's header here (if cart is present) on initial boot of any DSiWare!
#define InitialCartChipID 0x02FFFC00 // System Menu keeps cart's chip id here (if cart is present) on initial boot of any DSiWare!
#define InitialCartBannerOffset 0x02FFFAE8 


#ifdef __cplusplus
}
#endif

#endif // LAUNCHERDATA_H

