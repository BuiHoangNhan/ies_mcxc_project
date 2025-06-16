/**
* @file app_properties.c
* @brief 
*
* @details 
* @date 2024 Nov 28
* @version 1
* @author Duong Quoc Hoang
* Group: 
*
*/

#include <1_Hardware/7_Flash/flash_map_backend/flash_map.h>
#include <fsl_adapter_flash.h>
#include "app_properties.h"

#include <2_Middle/7_ota/mcuboot_app_support.h>

#include <stdio.h>
#include <stdint.h>

extern struct flash_area boot_flash_map[2];

/*****************************Function definitions*****************************/

/**
 * AppProperties_getVersion. get version of image
 * @return version of image
 */
const char *AppProperties_getVersion(void)
{
    static char version[16];
    uint32_t offset = boot_flash_map[0].fa_off;
    struct image_header hdr;
    HAL_FlashRead(offset, sizeof(hdr), (uint8_t *)&hdr);

    sprintf(version, "%d.%d.%d", hdr.ih_ver.iv_major, hdr.ih_ver.iv_minor, hdr.ih_ver.iv_revision);
    return version;
}

/* End of app_properties.c */
