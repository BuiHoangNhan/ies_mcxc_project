// mcu_port.c

#include <1_Hardware/7_Flash/flash_map_backend/flash_map_backend.h>
#include <fsl_adapter_flash.h>
#include <1_Hardware/7_Flash/os/os_malloc.h>
#include <1_Hardware/7_Flash/sysflash/sysflash.h>
#include <string.h>
#include <stdlib.h>


#define ALIGN_VAL 16

#define ERASED_VAL 0xFF
#define FLASH_SECTOR_SIZE 0x400

int flash_area_open(uint8_t id, const struct flash_area **area_outp) {
  uint32_t i = 0;

  for (i = 0; i < MCUBOOT_IMAGE_SLOT_NUMBER; i++)
  {
    if (boot_flash_map[i].fa_id == id)
    {
      *area_outp = &boot_flash_map[i];
      return 0;
    }
  }

  return -1;
}

void flash_area_close(const struct flash_area *area) {
}

int flash_area_read(const struct flash_area *fa, uint32_t off, void *dst,
                    uint32_t len) {
  int ret = 0;
  uint32_t addr = fa->fa_off + off;

  if (fa->fa_device_id != FLASH_DEVICE_ID)
  {
    return -1;
  }

  if (HAL_FlashRead(addr, len, dst) != kStatus_HAL_Flash_Success)
  {
    ret = -1;
  }

  return ret;
}

int flash_area_write(const struct flash_area *fa, uint32_t off, const void *src,
                     uint32_t len) {
  hal_flash_status_t status = kStatus_HAL_Flash_Success;
  uint8_t *src_ptr = (uint8_t *)src;
  uint32_t dst_addr = fa->fa_off + off;

  status = HAL_FlashProgram(dst_addr, len, src_ptr);
  if (status != kStatus_HAL_Flash_Success)
  {
    return -1;
  }

  return 0;
}

int flash_area_erase(const struct flash_area *fa, uint32_t off, uint32_t len) {
  uint32_t address = fa->fa_off + off;

  if (fa->fa_device_id != FLASH_DEVICE_ID)
  {
    return -1;
  }

  if ((address % FLASH_SECTOR_SIZE) || (len % FLASH_SECTOR_SIZE))
  {
    return -1;
  }
  status_t status = HAL_FlashEraseSector(address, len);
  if (status != kStatus_Success)
  {
    /* some callers check only for negative error codes, translate to -1 to stay on the safe side */
    return -1;
  }
  status = HAL_FlashVerifyErase(address, len, kHAL_Flash_MarginValueNormal);

  return 0;
}

uint8_t flash_area_align(const struct flash_area *area) {
  if (area->fa_device_id == FLASH_DEVICE_ID)
    return ALIGN_VAL;
  else
    return 0;
}

uint8_t flash_area_erased_val(const struct flash_area *area) {
  return ERASED_VAL;
}

int flash_area_get_sectors(int fa_id, uint32_t *count,
                           struct flash_sector *sectors) {
  const struct flash_area *fa;
  uint32_t max_cnt = *count;
  uint32_t rem_len;
  int rc = -1;

  if (flash_area_open(fa_id, &fa))
    goto out;

  if (*count < 1)
    goto fa_close_out;

  rem_len = fa->fa_size;
  *count = 0;
  while ((rem_len > 0) && (*count < max_cnt))
  {
    if (rem_len < FLASH_SECTOR_SIZE)
    {
      goto fa_close_out;
    }

    sectors[*count].fs_off = FLASH_SECTOR_SIZE * (*count);
    sectors[*count].fs_size = FLASH_SECTOR_SIZE;
    *count = *count + 1;
    rem_len -= FLASH_SECTOR_SIZE;
  }

  if (*count >= max_cnt)
  {
    goto fa_close_out;
  }

  rc = 0;

fa_close_out:
  flash_area_close(fa);
out:
  return rc;
}

int flash_area_id_from_multi_image_slot(int image_index, int slot) {
  switch (slot)
  {
  case 0:
    return FLASH_AREA_IMAGE_PRIMARY(image_index);

  case 1:
    return FLASH_AREA_IMAGE_SECONDARY(image_index);

  default:
    return -1; /* flash_area_open will fail on that */
  }
}

int flash_area_id_from_image_slot(int slot) {
  return flash_area_id_from_multi_image_slot(0, slot);
}

void example_assert_handler(const char *file, int line) {
}
