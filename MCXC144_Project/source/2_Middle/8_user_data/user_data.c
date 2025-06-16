/**
* @file user_data.c
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
#include "user_data.h"

#ifdef USER_DATA_ENABLE_LOG_DEBUG
#include "fsl_debug_console.h"

#define user_data_log_info(...)   \
	do                       \
	{                        \
		PRINTF("USER DATA [I]: "); \
		PRITNF(__VA_ARGS__); \
	} while (0)

#define user_data_log_error(...)  \
	do                           \
	{                            \
		PRITNF("USER DATA [E]: "); \
		PRITNF(__VA_ARGS__);     \
	} while (0)

#define user_data_log_debug(...) \
	do                           \
	{                            \
		PRITNF("USER DATA [D]: "); \
		PRITNF(__VA_ARGS__);     \
	} while (0)
#else

#define user_data_log_info(...)
#define user_data_log_error(...)
#define user_data_log_debug(...)

#endif


#define USER_DATA_OFFSET BOOT_FLASH_USER_DATA
#define USER_DATA_SIZE (0x40000 - BOOT_FLASH_USER_DATA)

static uint8_t UserData_magic[USER_DATA_MAGIC_SIZE] = USER_DATA_MAGIC_CODE;

UserData g_userData ;
/*****************************Function definitions*****************************/

static int _save(UserData *user_data);
static int _reload(UserData *user_data);
static void set_default(UserData *user_data);

#ifdef USER_DATA_ENABLE_LOG_DEBUG
static void UserData_print(UserData *user_data);
#endif
/**
 * @brief UserData_isMagicValid: Check user have magic code
 * @param user_data: UserData check
 * @return true: if magic code is valid, false: otherwise
 */
bool UserData_isMagicValid(UserData *user_data){
    for (int index = 0; index < USER_DATA_MAGIC_SIZE; index++){
        if (user_data->magic_code[index] != UserData_magic[index]){
            return false;
        }
    }

    return true;
}

/**
 * @brief UserData_setMagicCode: set magic code to user data
 * @param user_data: UserData want to set magic code
 * @return void
 */
void UserData_setMagicCode(UserData *user_data){
    for (int index = 0; index < USER_DATA_MAGIC_SIZE; index++)
    {
        user_data->magic_code[index] = UserData_magic[index];
    }
}

/**
 * @brief UserData_load: Load user data from flash
 * @param user_data: UserData want to load
 * @return 0: success, otherwise: fail
 */
int UserData_reload(void)
{
	return _reload(&g_userData);
}

/**
 * @brief UserData_save: Save user data to flash
 * @param user_data: UserData want to save
 * @return 0: success, otherwise: fail
 */
int UserData_save(void)
{
	if(g_userData.SystemData.Provision_state != PROVISION_SUCCESS)
	{g_userData.SystemData.Provision_state = PROVISION_NONE;}
	return _save(&g_userData);
}

int UserData_init(void){
    int major = 0, minor = 0, patch = 0;
	int ret = _reload(&g_userData);
#ifdef USER_DATA_ENABLE_LOG_DEBUG
	UserData_print(&g_userData);
#endif
	if (sscanf((char*)FW_VERSION, "%d.%d.%d", &major, &minor, &patch) == 3) {
		g_userData.SystemData.Fw_ver_major = (uint8_t)(major & 0xFF);
		g_userData.SystemData.Fw_ver_minor = (uint8_t)(minor & 0xFF);
		g_userData.SystemData.Fw_ver_path = (uint8_t)(patch & 0xFF);
	} else {
		// Default values if parsing fails
		g_userData.SystemData.Fw_ver_major = 01;
		g_userData.SystemData.Fw_ver_minor = 01;
		g_userData.SystemData.Fw_ver_path = 01;
	}
	UserData_save();
	return ret;
}

/*****************************Static Function definitions*****************************/

static int _save(UserData *user_data){
	UserData_setMagicCode(user_data);
	if(HAL_FlashEraseSector(USER_DATA_OFFSET, USER_DATA_SIZE) != kStatus_HAL_Flash_Success){
		user_data_log_error("USERDATA [E]: Erase user data error\n");
		return -1;
	}
	if(HAL_FlashVerifyErase(USER_DATA_OFFSET, USER_DATA_SIZE, kHAL_Flash_MarginValueNormal) != kStatus_HAL_Flash_Success){
		user_data_log_error("USERDATA [E]: Erase verify user data error\n");
		return -1;
	}
	if (HAL_FlashProgram(USER_DATA_OFFSET, sizeof(UserData), (uint8_t *)user_data) != kStatus_HAL_Flash_Success)
	{
		user_data_log_error("USERDATA [E]: Save user data error\n");
		return -1;
	}
	user_data_log_info("save user data\n");
#ifdef USER_DATA_ENABLE_LOG_DEBUG
	UserData_print(user_data);
#endif

	return 0;
	}

static int _reload(UserData *user_data){
	int ret = 0;
	if(HAL_FlashRead(USER_DATA_OFFSET, sizeof(UserData), (uint8_t *)user_data) != kStatus_HAL_Flash_Success)
	{
		user_data_log_error("USERDATA [E]: Flash read error\n");
		ret = -1;
	}

	if(!UserData_isMagicValid(user_data)){
		user_data_log_error("USERDATA [E]: Magic code invalid\n");
		ret = -2;
	}
	if(ret != 0){
		set_default(user_data);
	}

	return ret;
}

static void set_default(UserData *user_data){
	UserData_setMagicCode(user_data);}
UserData* UserData_getPointer(void)
{
	return &g_userData;
}
#ifdef USER_DATA_ENABLE_LOG_DEBUG
static void UserData_print(UserData *user_data){
	user_data_log_info("-----------------------------------------------------\n");
	user_data_log_info("data_period = %d\n", user_data->data_period);
	user_data_log_info("max_TX_power = %d\n", user_data->max_TX_power);
	user_data_log_info("------------------------------------------------------\n");
}
#endif

/* End of user_data.c */
