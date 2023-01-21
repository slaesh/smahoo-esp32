
#ifndef __BOARD_H__
#define __BOARD_H__

#define BOARD_IWR_OLD_WO_INPUT (1)
#define BOARD_IWR              (2)

#ifndef BOARD  // could be defined via build-script
#define BOARD (BOARD_IWR)
#warning defaulting 'BOARD' to BOARD_IWR
#endif

#endif  // __BOARD_H__
