/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FLASH_PARTITIONING_H_
#define _FLASH_PARTITIONING_H_

/*
  MCUBoot region       0x0     - 0x09000 : 36kB
  Primary slot         0x6000  - 0x23800 : 106kB (0x1A800 bytes)
  Secondary slot       0x23800 - 0x3e000 : 106kB (0x1A800 bytes)
  Optional user data   0x3e000 - 0x40000 : 8kB
*/

#define BOOT_FLASH_BASE 0x00000000
#define BOOT_FLASH_ACT_APP 0x00009000
#define BOOT_FLASH_CAND_APP 0x00023800
#define BOOT_FLASH_USER_DATA 0x0003e000

/*
  MCUBoot region       0x0     - 0x06000 : 24kB
  Primary slot         0x6000  - 0x22000 : 112kB (0x1C000 bytes)
  Secondary slot       0x22000 - 0x3e000 : 112kB (0x1C000 bytes)
  Optional user data   0x3e000 - 0x40000 : 8kB
*/

// #define BOOT_FLASH_BASE      0x00000000
// #define BOOT_FLASH_ACT_APP   0x00006000
// #define BOOT_FLASH_CAND_APP  0x00022000
// #define BOOT_FLASH_USER_DATA 0x0003e000

#endif
