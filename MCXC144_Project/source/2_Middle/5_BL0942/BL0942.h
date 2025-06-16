 /**
 * @file bl0942.h
 * @brief BL0942 energy sensor support. This library is a port from ESPHome repository
 * Link: https://github.com/esphome/esphome/tree/dev/esphome/components/bl0942
 *
 * @author Ly Hon Quang, Doan Quoc Huy
 * Group: Vetek Co. Ltd.
 *
 */

 #ifndef BL0942_H_
 #define BL0942_H_

 #ifdef __cplusplus
 extern "C" {
 #endif

 #include <stdint.h>
 #include <stdbool.h>
 #include <1_Hardware/5_Uart_BL0942/Uart_BL0942.h>
 /*******************************Definitions************************************/

 #define BL0942_IREF                     251213.46469622;  // 305978/1.218
 #define BL0942_UREF                     15873.35944299;   // should be 73989/1.218
 #define BL0942_PREF                     596;              // taken from tasmota
 #define BL0942_EREF                     3304.61127328;    // Measured

 // calibration method: measure reference (real) value, read raw value, REF_FACTOR = raw_value / ref_value
 // Power UIcos(phi), led driver max power cos(phi) 0.62
 // self-calibrated with shunt 7.5 ohm
 // #define BL0942_IREF                     162717.5
 // #define BL0942_UREF                     15873.35944299
 // #define BL0942_PREF                     408
 // #define BL0942_EREF                     3304.61127328

 #define BL0942_FULL_PACKET_RECEIVE_LEN  23
typedef enum __enum_BL0942_status {
BL0942_NONE = 0,
BL0942_PROGRESS = 1,
BL0942_DONE = 2, 
} enum_BL0942_status;

#pragma pack(1)
 typedef struct {
   float voltage;
   float current;
   float power;
   float energy;
   float frequency;
   enum_BL0942_status  status;
 } BL0942_Data;
#pragma pack()
 #if LCD_CONTROLER
 typedef enum {
     BL0942_CHANNEL1 = 0,
     BL0942_CHANNEL2 = 1,
     BL0942_CHANNEL3 = 2,
 } enum_BL0942_Channel;
 #endif
 /****************************Function prototypes*******************************/

 void BL0942_setup();
 void BL0942_requestFull(uint8_t* buf);
 bool BL0942_processFull(uint8_t *packet, uint8_t len, BL0942_Data* data);

 bool BL0942_validate_checksum(uint8_t* packet, uint8_t len);
 // int BL0942_read_reg_block(uint8_t reg);
 void BL0942_write_reg(uint8_t reg, uint32_t val);

 bool BL0942_process(BL0942_Data* data);

 bool BL0942_request(void);
 /******************************************************************************/

 #ifdef __cplusplus
 }
 #endif

 #endif /* BL0942_H_ */

 /* End of bl0942.h */
