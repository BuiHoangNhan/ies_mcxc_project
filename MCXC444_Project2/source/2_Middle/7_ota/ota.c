/**
* @file ota.c
* @brief 
*
* @details 
* @date 2024 Nov 28
* @version 1
* @author Duong Quoc Hoang
* Group: 
*
*/

#include <1_Hardware/7_Flash/flash_partitioning.h>
#include <fsl_adapter_flash.h>
#include "ota.h"

#include <ctype.h>
#include <stdbool.h>
#include "system_MCXC444.h"

#include "mcuboot_app_support.h"
#include "platform_bindings.h"
#include "fsl_clock.h"
#include "fsl_adapter_reset.h"

#define APP_IMAGE (0)
#ifdef OTA_ENABLE_DEBUG_LOG
#include "fsl_debug_console.h"

#define ota_log_info(...)    \
    do                       \
    {                        \
        printf("OTA [I]: "); \
        printf(__VA_ARGS__); \
    } while (0)

#define ota_log_error(...)   \
    do                       \
    {                        \
        printf("OTA [E]: "); \
        printf(__VA_ARGS__); \
    } while (0)

#define ota_log_debug(...)   \
    do                       \
    {                        \
        printf("OTA [D]: "); \
        printf(__VA_ARGS__); \
    } while (0)
#else

#define ota_log_info(...)
#define ota_log_error(...)
#define ota_log_debug(...)

#endif

/*****************************Static Variable*****************************/
static uint8_t OTA_progbuf[1024*5]; //buffer OTA progress
static hashctx_t OTA_sha256_xmodem_ctx;
static partition_t OTA_prt;
static strtOTA_State strOTAstate;
/*****************************Function definitions*****************************/
static int flash_sha256(uint32_t offset, size_t size, uint8_t sha256[32]);
static int process_received_data(uint32_t offset, uint32_t size);

/**
 * OTA_start. Init offset anh get partition info
 * @return 0: success, else fail
 */
static uint16_t frame_number =0;
int OTA_start(void)
{
    if (bl_get_update_partition_info(APP_IMAGE, &OTA_prt) != kStatus_Success)
    {
        ota_log_error("FAILED to determine address for download\n");
        return -1;
    }
    memset(&strOTAstate, 0, sizeof(strtOTA_State));
    memset(OTA_progbuf, 0xff, sizeof(OTA_progbuf));
	if(HAL_FlashEraseSector(OTA_prt.start, OTA_prt.size) != kStatus_HAL_Flash_Success){
        ota_log_error("Erase error\n");
        return -1;
	}
	if(HAL_FlashVerifyErase(OTA_prt.start, OTA_prt.size, kHAL_Flash_MarginValueNormal) != kStatus_HAL_Flash_Success){
        ota_log_error("Erase verify error\n");
        return -1;
	}
    ota_log_info("Started download into flash at 0x%X, size 0x%X\n", OTA_prt.start, OTA_prt.size);
    sha256_init(&OTA_sha256_xmodem_ctx);

    return 0;
}

/**
 * OTA_progress. program data to flash if buffer full
 * @param buff: pointer of buffer
 * @param size: size of buffer
 * @return 0: success, else fail
 */
int OTA_progress(uint8_t *buff, uint32_t size)
{
    int ret = 0;
    frame_number++;
    ota_log_info("Frame number %d\n", frame_number);
    for (int index = 0; index < size; index++)
    {
        uint8_t *buffer_ota = (uint8_t *)OTA_progbuf;
        buffer_ota[strOTAstate.OTA_buf_offset++] = buff[index];
        if (strOTAstate.OTA_buf_offset >= sizeof(OTA_progbuf))
        {
            if(process_received_data(strOTAstate.OTA_offset, strOTAstate.OTA_buf_offset) != kStatus_Success){
            	ota_log_info("faile at %02X %02X", strOTAstate.OTA_buf_offset, strOTAstate.OTA_offset);
                ret = -1;
                OTA_setState(eOTA_Idle);
                memset(OTA_progbuf, 0xff, sizeof(OTA_progbuf));
                break;
            }
            else{
                memset(OTA_progbuf, 0xff, sizeof(OTA_progbuf));
                strOTAstate.OTA_offset += strOTAstate.OTA_buf_offset;
                strOTAstate.OTA_buf_offset = 0;
            }
        }
    }
    return ret;
}

/**
 * OTA_done. Check data receiver and flash data
 * @return 0: reboot if success, else fail
 */
int OTA_done(void)
{
    static uint8_t sha256_recv[32], sha256_flash[32];
    ota_log_info("OTA Done\n");
    if (strOTAstate.OTA_buf_offset > 0)
    {
        process_received_data(strOTAstate.OTA_offset, strOTAstate.OTA_buf_offset);
        strOTAstate.OTA_offset += strOTAstate.OTA_buf_offset;
        strOTAstate.OTA_buf_offset = 0;
    }

    SDK_DelayAtLeastUs(100000, SystemCoreClock);

    if (strOTAstate.OTA_offset < 0)
    {
        ota_log_error("\nTransfer failed (%d)\n", strOTAstate.OTA_offset);
        return -1;
    }

    ota_log_info("\nReceived %u bytes\n", strOTAstate.OTA_offset);

    sha256_finish(&OTA_sha256_xmodem_ctx, sha256_recv);
    flash_sha256(OTA_prt.start, strOTAstate.OTA_offset, sha256_flash);
    int ret = 0;

    if(memcmp(sha256_recv, sha256_flash, sizeof(sha256_recv)) != 0){
        ret = 1;
        ota_log_info("SHA256 of received data: ");
        print_hash(sha256_recv, 10);
        ota_log_info("...\n");

        ota_log_info("SHA256 of flashed data:  ");
        print_hash(sha256_flash, 10);
        ota_log_info("...\n");
    }

    if (ret == 0 && bl_verify_image(OTA_prt.start, OTA_prt.size) == 1){
    	HAL_ResetMCU();
    }
    else{
        ret = 2;
    }

    return ret;
}

/*****************************Static Function*****************************/
static int flash_sha256(uint32_t offset, size_t size, uint8_t sha256[32])
{
    uint32_t buf[128 / sizeof(uint32_t)];
    status_t status;
    hashctx_t sha256ctx;

    sha256_init(&sha256ctx);

    while (size > 0)
    {
        size_t chunk = (size > sizeof(buf)) ? sizeof(buf) : size;
        if(HAL_FlashRead(offset, chunk, (uint8_t *)buf) != kStatus_HAL_Flash_Success)
		{
			return -1;
		}

        sha256_update(&sha256ctx, (unsigned char *)buf, chunk);

        size -= chunk;
        offset += chunk;
    }

    sha256_finish(&sha256ctx, sha256);

    return kStatus_Success;
}

static int process_received_data(uint32_t offset, uint32_t size)
{
    int ret;
    uint32_t *data = OTA_progbuf;
    uint32_t addr = OTA_prt.start + offset;

    if (HAL_FlashProgram(addr, size, (uint8_t *)data) != kStatus_HAL_Flash_Success)
	{
		return -1;
	}
    sha256_update(&OTA_sha256_xmodem_ctx, data, size);

    return 0;
}
eOTA_State_t OTA_getState(void)
{
    return strOTAstate.State;
} 

void OTA_setState(eOTA_State_t state)
{
    strOTAstate.State = state;
}
/* End of ota.c */
