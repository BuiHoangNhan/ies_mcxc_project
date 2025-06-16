
/**
 * @file ota.h
 * @brief
 *
 * @details
 * @date 2024 Nov 28
 * @version 1
 * @author Duong Quoc Hoang
 * Group:
 *
 */

#ifndef OTA_H_
#define OTA_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

typedef enum __eOTA_State
{
    eOTA_Idle = 0,
    eOTA_start,
    eOTA_Progress
} eOTA_State_t;
typedef struct __strtOTA_State
{
    uint32_t OTA_buf_offset;
    long OTA_offset;
    eOTA_State_t State;
} strtOTA_State;
    /****************************Function prototypes*******************************/
    int OTA_start(void);
    int OTA_progress(uint8_t *buff, uint32_t size);
    int OTA_done(void);
    eOTA_State_t OTA_getState(void); 
    void OTA_setState(eOTA_State_t state);
    /******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* OTA_H_ */

/* End of ota.h */
