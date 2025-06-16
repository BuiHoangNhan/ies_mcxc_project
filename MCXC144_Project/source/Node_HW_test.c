#include <3_Application/Sys_Process.h>


uint8_t Hw_log_test_buff[128];

static void HW_test_log(char* log, float param);
static void HW_test_init(void);
static void Lora_test(void);
static void Sensor_test(void);
static void Dim_test(uint16_t Dim_value);
static void Dimmer_Test(void);
static void Atm90e_test(void);
static void LCD_test(void);
static uint16_t CurDim_value = 0;

#if LCD_CONTROLER
static void LCD_test(void);
#elif NODE_NEMA
static void Dimmer_Test(void);
#endif
void HW_Test(void)
{
    HW_test_init();
    Lora_test();
    Sensor_test();
#if NODE_NEMA
    Dimmer_Test();
    Atm90e_test();
#elif LCD_CONTROLER
    LCD_test();
#endif
}
static void HW_test_log(char* log, float param)
{
    memset(Hw_log_test_buff, 0, sizeof(Hw_log_test_buff));
    uint16_t Src_addr = UserData_getPointer()->SystemData.Src_addr;
    if(param!=0xffffffff)
    {sprintf((char*)Hw_log_test_buff, " %s %d voi dia chi la :%04X \n\n", log, (uint32_t)param, Src_addr);}
    else
    {sprintf((char*)Hw_log_test_buff, " %s voi dia chi la :%04X\n\n", log, Src_addr);}
    Lora_SendMsg((uint8_t*)Hw_log_test_buff, strlen(Hw_log_test_buff));
}

static void HW_test_init(void)
{
    Lora_SetChannel(10);
    Lora_SetPainID(10);
    Lora_ChangMode(Broadcast);
    System_DelayMs(1000);
    HW_test_log("Tien hanh kiem tra thiet bi", 0xffffffff);
    uint8_t Mac_strBuff[32];
    sprintf((char*)Mac_strBuff, "Dia chi MAC la: %08X", UserData_getPointer()->SystemData.Mac );
    HW_test_log((char*)Mac_strBuff, 0xffffffff);
}

static void Lora_test(void)
{
    HW_test_log("Kiem tra tin hieu module Lora ok", 0xffffffff);
}

static void Sensor_test(void)
{
#if NODE_NEMA
    Sensor_ADC_request(SENSOR_ADC16_TEMPERATURE_CHANNEL);
    HW_test_log(" Testing temp sensor",0xffffffff);
    System_DelayMs(500);
    if(p_Sensor_ADC_get(SENSOR_ADC16_TEMPERATURE_CHANNEL)->AdcStatus)
    {
        HW_test_log("cam bien nhiet do co gia tri: ", p_Sensor_ADC_get(SENSOR_ADC16_TEMPERATURE_CHANNEL)->AdcValue);
    }
    else
    {
        HW_test_log("Cam bien nhiet do bi loi", 0xffffffff);
    }
#elif LCD_CONTROLER
    Sensor_t *Sensor = Sensor_get_value();
    Sensor->BL0942_CurChannel = 2;
    HW_test_log("Tien hanh kiem tra dien ap cac pha", 0xffffffff);
    Proc_bl0942_request_data();
    System_DelayMs(500);
    Proc_bl0942_process_data();
    HW_test_log("Kiem tra dien ap phase 1 la: ",Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].voltage);
    HW_test_log("Kiem tra dong dien phase 1 la: ",Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].current);
    HW_test_log("Kiem tra cong suat phase 1 la: ",Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].power);
    Proc_bl0942_request_data();
    HW_test_log("Bat relay 1: ",0xffffffff);
    Relay_set(GPIO_GPIO_RELAY_1, 1);
    System_DelayMs(500);
    Proc_bl0942_process_data();
    HW_test_log("Kiem tra dien ap phase 2 la: ",Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].voltage);
    HW_test_log("Kiem tra dong dien phase 2 la: ",Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].current);
    HW_test_log("Kiem tra cong suatphase 2 la: ",Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].power);
    Proc_bl0942_request_data();
    Relay_set(GPIO_GPIO_RELAY_2, 1);
    HW_test_log("Bat relay 2: ",0xffffffff);
    System_DelayMs(500);
    Proc_bl0942_process_data();
    HW_test_log("Kiem tra dien ap phase 3 la: ",Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].voltage);
    HW_test_log("Kiem tra dong dien phase 3 la: ",Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].current);
    HW_test_log("Kiem tra dien ap phase 3 la: ",Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].power);
    Relay_set(GPIO_GPIO_RELAY_3, 1);
    HW_test_log("Bat relay 3: ",0xffffffff);
    System_DelayMs(500);

#endif

}
#if NODE_NEMA
static void Atm90e_test(void)
{

    uint8_t tempBuf[30];
    HW_test_log("Atm90e kiem tra", 0xffffffff);
    float voltage = GetLineVoltage();
    sprintf(tempBuf, "Dien ap: %f", voltage);
    HW_test_log(tempBuf, 0xffffffff);
    memset(tempBuf, 0, sizeof(tempBuf));
    float current = GetLineCurrent();
    sprintf(tempBuf, "Dong dien: %f", current);
    HW_test_log(tempBuf, 0xffffffff);
    memset(tempBuf, 0, sizeof(tempBuf));
    float power = GetActivePower();
    sprintf(tempBuf, "Cong suat: %f", power);
    HW_test_log(tempBuf, 0xffffffff);
    memset(tempBuf, 0, sizeof(tempBuf));
    System_DelayMs(500);
    HW_test_log("Atm90e kiem tra", 0xffffffff);
     voltage = GetLineVoltage();
    sprintf(tempBuf, "Dien ap: %f", voltage);
    HW_test_log(tempBuf, 0xffffffff);
    memset(tempBuf, 0, sizeof(tempBuf));
     current = GetLineCurrent();
    sprintf(tempBuf, "Dong dien: %f", current);
    HW_test_log(tempBuf, 0xffffffff);
    memset(tempBuf, 0, sizeof(tempBuf));
     power = GetActivePower();
    sprintf(tempBuf, "Cong suat: %f", power);
    HW_test_log(tempBuf, 0xffffffff);
    memset(tempBuf, 0, sizeof(tempBuf));
    System_DelayMs(500);
    HW_test_log("Atm90e kiem tra", 0xffffffff);
     voltage = GetLineVoltage();
    sprintf(tempBuf, "Dien ap: %f", voltage);
    HW_test_log(tempBuf, 0xffffffff);
    memset(tempBuf, 0, sizeof(tempBuf));
     current = GetLineCurrent();
    sprintf(tempBuf, "Dong dien: %f", current);
    HW_test_log(tempBuf, 0xffffffff);
    memset(tempBuf, 0, sizeof(tempBuf));
     power = GetActivePower();
    sprintf(tempBuf, "Cong suat: %f", power);
    HW_test_log(tempBuf, 0xffffffff);
    memset(tempBuf, 0, sizeof(tempBuf));
    HW_test_log("Atm90e xong ", 0xffffffff);


}
static void Dim_test(uint16_t Dim_value)
{
    DimOff_set(Dim_value!=0);
    bool DimStatus = false;
    uint16_t DimThreshold = Dim_value; 
    while(!DimStatus)
    {
        if(CurDim_value == DimThreshold)
        {
            DimStatus = true;
        }
        else if(CurDim_value < DimThreshold)
        {
            CurDim_value++;
            Dimmer_set((uint16_t)(CurDim_value*V_LED_DIMMER_SCALE));
        }
        else
        {
            CurDim_value--;
            Dimmer_set((uint16_t)(CurDim_value*V_LED_DIMMER_SCALE));
        }
        System_DelayMs(2);
    }
    CurDim_value = DimThreshold;

}
static void Dimmer_Test(void)
{

    HW_test_log("TIEN HANH KIEM TRA BO DIMMER", 0xffffffff);
    for(uint16_t i = 0; i < 3; i++)
    {
        HW_test_log("% DIMMER ", i*50);
        Dim_test(i*10);
        System_DelayMs(500);
    }
    
    HW_test_log("Dim thanh cong ", 0xffffffff);
}
#elif LCD_CONTROLER
static void LCD_test(void)
{
    Proc_DWIN(GOTO_PAGE, 0,0, 0,0x0);
    uint8_t LCDtemp_buff[18] = {11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11};
    Proc_DWIN(WRITE_LCD_INIT_INFO, LCD_VERSIOM_MAJOR,(uint8_t*)LCDtemp_buff, 0,0x01);
}
#endif

