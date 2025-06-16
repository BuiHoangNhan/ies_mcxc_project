/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SYSFLASH_H__
#define __SYSFLASH_H__

//! A user-defined identifier for different storage mediums
//! (i.e internal flash, external NOR flash, eMMC, etc)
#define FLASH_DEVICE_INTERNAL_FLASH 0

//! An arbitrarily high slot ID we will use to indicate that
//! there is not slot
#define FLASH_SLOT_DOES_NOT_EXIST 255

//! NB: MCUboot expects this define to exist but it's only used
//! if MCUBOOT_SWAP_USING_SCRATCH=1 is set
#define FLASH_AREA_IMAGE_SCRATCH FLASH_SLOT_DOES_NOT_EXIST

//! The slot we will use to track the bootloader allocation
#define FLASH_AREA_BOOTLOADER 0

//! A mapping to primary and secondary/upgrade slot
//! given an image_index. We'll plan to use
#define FLASH_AREA_IMAGE_PRIMARY(i) ((i == 0) ? 0 : 255)
#define FLASH_AREA_IMAGE_SECONDARY(i) ((i == 0) ? 1 : 255)

#define MCUBOOT_IMAGE_SLOT_NUMBER 2
#define FLASH_DEVICE_ID 1

#endif /* __SYSFLASH_H__ */
