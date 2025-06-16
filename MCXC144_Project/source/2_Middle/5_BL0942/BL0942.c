 /**
 * @file bl0942.c
 * @brief BL0942 energy sensor support. This library is a port from ESPHome repository
 * Link: https://github.com/esphome/esphome/tree/dev/esphome/components/bl0942
 *
 * @author Ly Hon Quang, Doan Quoc Huy
 * Group: Vetek Co. Ltd.
 *
 */


 #include <stdint.h>
 #include <stdio.h>
 #include "BL0942.h"

 /*******************************Definitions************************************/
#if LCD_CONTROLER

 #define LINE_FREQUENCY_50HZ         50
 #define LINE_FREQUENCY_60HZ         60

 // Datasheet: https://www.belling.com.cn/media/file_object/bel_product/BL0942/datasheet/BL0942_V1.06_en.pdf

 static const uint8_t BL0942_ADDRESS = 0;

 static const uint8_t BL0942_READ_COMMAND = 0x58;
 static const uint8_t BL0942_FULL_PACKET = 0xAA;
 static const uint8_t BL0942_PACKET_HEADER = 0x55;

 static const uint8_t BL0942_WRITE_COMMAND = 0xA8;

 static const uint8_t BL0942_REG_I_RMSOS = 0x12;
 static const uint8_t BL0942_REG_WA_CREEP = 0x14;
 static const uint8_t BL0942_REG_I_FAST_RMS_TH = 0x15;
 static const uint8_t BL0942_REG_I_FAST_RMS_CYC = 0x16;
 static const uint8_t BL0942_REG_FREQ_CYC = 0x17;
 static const uint8_t BL0942_REG_OT_FUNX = 0x18;
 static const uint8_t BL0942_REG_MODE = 0x19;
 static const uint8_t BL0942_REG_SOFT_RESET = 0x1C;
 static const uint8_t BL0942_REG_USR_WRPROT = 0x1D;
 static const uint8_t BL0942_REG_TPS_CTRL = 0x1B;

 static const uint32_t BL0942_REG_MODE_RESV = 0x03;
 static const uint32_t BL0942_REG_MODE_CF_EN = 0x04;
 static const uint32_t BL0942_REG_MODE_RMS_UPDATE_SEL = 0x08;
 static const uint32_t BL0942_REG_MODE_FAST_RMS_SEL = 0x10;
 static const uint32_t BL0942_REG_MODE_AC_FREQ_SEL = 0x20;
 static const uint32_t BL0942_REG_MODE_CF_CNT_CLR_SEL = 0x40;
 static const uint32_t BL0942_REG_MODE_CF_CNT_ADD_SEL = 0x80;
 static const uint32_t BL0942_REG_MODE_UART_RATE_19200 = 0x200;
 static const uint32_t BL0942_REG_MODE_UART_RATE_38400 = 0x300;
 static const uint32_t BL0942_REG_MODE_DEFAULT =
     BL0942_REG_MODE_RESV | BL0942_REG_MODE_CF_EN | BL0942_REG_MODE_CF_CNT_ADD_SEL;

 static const uint32_t BL0942_REG_SOFT_RESET_MAGIC = 0x5a5a5a;
 static const uint32_t BL0942_REG_USR_WRPROT_MAGIC = 0x55;

 /*************************Private function prototypes**************************/

 static uint32_t pu8tou24(uint8_t* val);
 static int32_t pu8toi24(uint8_t* val);

 // Low-level functions
 // void BL0942_writeByte(uint8_t val)
 // {
 //   printf("%x ", val);
 // }


 uint8_t *bl0942_buf;

 /*****************************Function definitions*****************************/

 bool BL0942_validate_checksum(uint8_t* packet, uint8_t len) {
   uint8_t checksum = BL0942_READ_COMMAND | BL0942_ADDRESS;
   // Whole package but checksum
   for (uint32_t i = 0; i < len - 1; i++) {
     checksum += packet[i];
   }
   checksum ^= 0xFF;
   return checksum == packet[len - 1];
 }

 void BL0942_write_reg(uint8_t reg, uint32_t val) {
   uint8_t pkt[6];

   pkt[0] = BL0942_WRITE_COMMAND | BL0942_ADDRESS;
   pkt[1] = reg;
   pkt[2] = (val & 0xff);
   pkt[3] = (val >> 8) & 0xff;
   pkt[4] = (val >> 16) & 0xff;
   pkt[5] = (pkt[0] + pkt[1] + pkt[2] + pkt[3] + pkt[4]) ^ 0xff;
   BL0942_writeStr(pkt, 6);
   BL0942_delay_ms(20);  // frame-to-frame delay
 }

 // int BL0942_read_reg_block(uint8_t reg) {
 //   union {
 //     uint8_t b[4];
 //     uint32_le_t le32;
 //   } resp;
 //   char req_packet[2] = {BL0942_READ_COMMAND | BL0942_ADDRESS, reg};
 //   BL0942_writeStr(req_packet, 2);

 // //   BL0942_writeByte(BL0942_READ_COMMAND | BL0942_ADDRESS);
 // //   BL0942_writeByte(reg);
 //   if (this->read_array(resp.b, 4) &&
 //       resp.b[3] ==
 //           (uint8_t) ((BL0942_READ_COMMAND + BL0942_ADDRESS + reg + resp.b[0] + resp.b[1] + resp.b[2]) ^ 0xff)) {
 //     resp.b[3] = 0;
 //     return resp.le32;
 //   }
 //   return -1;
 // }

 void BL0942_setup() {
   BL0942_write_reg(BL0942_REG_USR_WRPROT, BL0942_REG_USR_WRPROT_MAGIC);
   BL0942_write_reg(BL0942_REG_SOFT_RESET, BL0942_REG_SOFT_RESET_MAGIC);

   uint32_t mode = BL0942_REG_MODE_DEFAULT;
   mode |= BL0942_REG_MODE_RMS_UPDATE_SEL; /* 800ms refresh time */
   // if (this->line_freq_ == LINE_FREQUENCY_60HZ)
   //   mode |= BL0942_REG_MODE_AC_FREQ_SEL;
   BL0942_write_reg(BL0942_REG_MODE, mode);
   BL0942_write_reg(BL0942_REG_USR_WRPROT, 0);

   // if (BL0942_read_reg(BL0942_REG_MODE) != mode)
     // this->status_set_warning("BL0942 setup failed!");
 }

 void BL0942_requestFull(uint8_t* buf) {
   const char req_packet[2] = {BL0942_READ_COMMAND | BL0942_ADDRESS, BL0942_FULL_PACKET};
//   BL0942_readString(buf, BL0942_FULL_PACKET_RECEIVE_LEN);
   // BL0942_writeByte(BL0942_READ_COMMAND | BL0942_ADDRESS);
   // BL0942_writeByte(BL0942_FULL_PACKET);
   BL0942_writeStr((uint8_t*)req_packet, 2);
   BL0942_delay_ms(5);
 }

 bool BL0942_processFull(uint8_t *packet, uint8_t len, BL0942_Data* data) {
   // Bad header or checksum
   if (!BL0942_validate_checksum(packet, len) || \
       packet[0] != BL0942_PACKET_HEADER) return false;

   data->current = pu8tou24(packet + 1) / BL0942_IREF;
   data->voltage = pu8tou24(packet + 4) / BL0942_UREF;
   data->power = pu8toi24(packet + 10) / BL0942_PREF;
   data->energy = pu8tou24(packet + 13) / BL0942_EREF;
   data->frequency = 1000000.0f / pu8tou24(packet + 16);
   return true;
 }

 static uint32_t pu8tou24(uint8_t* val) {
   return (uint32_t)val[0] | (((uint32_t)val[1]) << 8) | \
     (((uint32_t)val[2]) << 16);
 }

 static int32_t pu8toi24(uint8_t* val) {
   int32_t tmp = (int32_t)val[0] | (((int32_t)val[1]) << 8) | \
     (((int32_t)val[2]) << 16);
   // Convert bytes to integer using bitwise operations
   // Check if the most significant bit is set (indicating a negative number)
   if (tmp & 0x800000) {
       // Convert to two's complement
       tmp = -((0xFFFFFF + 1) - tmp);
   }
   return tmp;
 }

 bool BL0942_process(BL0942_Data* data) {
   return BL0942_processFull(bl0942_buf, BL0942_FULL_PACKET_RECEIVE_LEN, data);
 }

 bool BL0942_request(void)
 {
     BL0942_requestFull(bl0942_buf);
 }
 /* End of bl0942.c */
#endif
