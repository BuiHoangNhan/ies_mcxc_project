/**
 * @file atm90e26.c
 * @brief
 *
 * @details
 * @date 2025 Mar 18
 * @version 1
 * @author Tran Minh Sang
 * Group:
 *
 */

#include "atm90e26.h"
#include <stdint.h>
#include <1_Hardware/2_LCD/Uart_LCD.h>
#include <1_Hardware/0_Timer/timer.h>
#if NODE_NEMA
#define SoftReset 0x00 //Software Reset
#define SysStatus 0x01 //System Status
#define FuncEn 0x02 //Function Enable
#define SagTh 0x03 //Voltage Sag Threshold
#define SmallPMod 0x04 //Small-Power Mode
#define LastData 0x06 //Last Read/Write SPI/UART Value
#define LSB 0x08 //RMS/Power 16-bit LSB
#define CalStart 0x20 //Calibration Start Command
#define PLconstH 0x21 //High Word of PL_Constant
#define PLconstL 0x22 //Low Word of PL_Constant
#define Lgain 0x23 //L Line Calibration Gain
#define Lphi 0x24 //L Line Calibration Angle
#define Ngain 0x25 //N Line Calibration Gain
#define Nphi 0x26 //N Line Calibration Angle
#define PStartTh 0x27 //Active Startup Power Threshold
#define PNolTh 0x28 //Active No-Load Power Threshold
#define QStartTh 0x29 //Reactive Startup Power Threshold
#define QNolTh 0x2A //Reactive No-Load Power Threshold
#define MMode 0x2B //Metering Mode Configuration
#define CSOne 0x2C //Checksum 1
#define AdjStart 0x30 //Measurement Calibration Start Command
#define Ugain 0x31 //Voltage rms Gain
#define IgainL 0x32 //L Line Current rms Gain
#define IgainN 0x33 //N Line Current rms Gain
#define Uoffset 0x34 //Voltage Offset
#define IoffsetL 0x35 //L Line Current Offset
#define IoffsetN 0x36 //N Line Current Offse
#define PoffsetL 0x37 //L Line Active Power Offset
#define QoffsetL 0x38 //L Line Reactive Power Offset
#define PoffsetN 0x39 //N Line Active Power Offset
#define QoffsetN 0x3A //N Line Reactive Power Offset
#define CSTwo 0x3B //Checksum 2
#define APenergy 0x40 //Forward Active Energy
#define ANenergy 0x41 //Reverse Active Energy
#define ATenergy 0x42 //Absolute Active Energy
#define RPenergy 0x43 //Forward (Inductive) Reactive Energy
#define Rnenerg 0x44 //Reverse (Capacitive) Reactive Energy
#define Rtenergy 0x45 //Absolute Reactive Energy
#define EnStatus 0x46 //Metering Status
#define Irms 0x48 //L Line Current rms
#define Urms 0x49 //Voltage rms
#define Pmean 0x4A //L Line Mean Active Power
#define Qmean 0x4B //L Line Mean Reactive Power
#define Freq 0x4C //Voltage Frequency
#define PowerF 0x4D //L Line Power Factor
#define Pangle 0x4E //Phase Angle between Voltage and L Line Current
#define Smean 0x4F //L Line Mean Apparent Power
#define IrmsTwo 0x68 //N Line Current rms
#define PmeanTwo 0x6A //N Line Mean Active Power
#define QmeanTwo 0x6B //N Line Mean Reactive Power
#define PowerFTwo 0x6D //N Line Power Factor
#define PangleTwo 0x6E //Phase Angle between Voltage and N Line Current
#define SmeanTwo 0x6F //N Line Mean Apparent Power

#define UgainValue 0x64D1
#define IgainLValue 0xC263
#define UoffsetValue 0x0000
#define IoffsetLValue 0x0000
#define PoffsetLValue 0x0000
#define QoffsetLValue 0x0000
uart_func_t g_uart_func;

/*****************************Function definitions*****************************/
static uint16_t calculate_CSTwo(uint16_t *registers, size_t count) {
    uint16_t sum = 0;
    uint8_t xor_result = 0;
    
    for(size_t i = 0; i < count; i++) {
        uint8_t high_byte = registers[i] >> 8;
        uint8_t low_byte = registers[i] & 0xFF;
        
        // Calculate sum for low byte
        sum += high_byte + low_byte;
        
        // Calculate XOR for high byte
        xor_result ^= high_byte;
        xor_result ^= low_byte;
    }
    
    // Combine results into final checksum
    return ((uint16_t)xor_result << 8) | (sum & 0xFF);
}
void ATM90E26_Init(void)
{
    Power_uart_init();

    g_uart_func.uart_write_byte = POWER_Sendcmd;
    g_uart_func.uart_read_byte = Power_uart_get_byte;
    g_uart_func.uart_delay = System_DelayMs;

    InitEnergyIC();
}
static unsigned short CommEnergyIC(unsigned char RW, unsigned char address, unsigned short val)
{
    uint8_t CommEnergyIC_buffer[10] = {0};
    uint8_t lenbyte = 0;
    unsigned short output;
    // Set read write flag
    address |= RW << 7;

    unsigned char host_chksum = address;
    if (!RW) // Si es una operacion de escritura en registro
    {
        unsigned short chksum_short = (val >> 8) + (val & 0xFF) + address;
        host_chksum = chksum_short & 0xFF;
    }

    // begin UART command
    CommEnergyIC_buffer[lenbyte++] = 0xFE;
    CommEnergyIC_buffer[lenbyte++] = address;

    if (!RW) // Si es una operacion de escritura en registro
    {
        unsigned char MSBWrite = val >> 8;
        unsigned char LSBWrite = val & 0xFF;

        CommEnergyIC_buffer[lenbyte++] = MSBWrite;
        CommEnergyIC_buffer[lenbyte++] = LSBWrite;
    }
    CommEnergyIC_buffer[lenbyte++] = host_chksum;
    g_uart_func.uart_write_byte(CommEnergyIC_buffer, lenbyte);
    g_uart_func.uart_delay(10);

    // Read register only
    if (RW) // Operacion de lectura
    {
        unsigned char MSByte;
        unsigned char LSByte;
        unsigned char atm90_chksum;

        g_uart_func.uart_read_byte(&MSByte);
        g_uart_func.uart_read_byte(&LSByte);
        g_uart_func.uart_read_byte(&atm90_chksum);

        if (atm90_chksum == ((LSByte + MSByte) & 0xFF))
        {
            output = (MSByte << 8) | LSByte; // join MSB and LSB;
            return output;
        }
        // g_uart_func.uart_delay(20);
        return 0xFFFF;
    }
    // Write register only
    else // Operacion de escritura
    {
        unsigned char atm90_chksum;
        g_uart_func.uart_read_byte(&atm90_chksum);

        if (atm90_chksum != host_chksum)
        {
            // g_uart_func.uart_delay(20);// Delay from failed transaction
        }
    }
    return 0xFFFF;
}

int InitEnergyIC(void)
{

    unsigned short systemstatus = 0;

	CommEnergyIC(0,SoftReset,0x789A); //Perform soft reset
	CommEnergyIC(0,FuncEn,0x0030); //Voltage sag irq=1, report on warnout pin=1, energy dir change irq=0
	CommEnergyIC(0,SagTh,0x1F2F); //Voltage sag threshhold


	//Set metering calibration values
	CommEnergyIC(0,CalStart,0x5678); //Metering calibration startup command. Register 21 to 2B need to be set
	CommEnergyIC(0,PLconstH,0x00B9); //PL Constant MSB
	CommEnergyIC(0,PLconstL,0xC1F3); //PL Constant LSB
	CommEnergyIC(0,Lgain,0x1D39); 	//Line calibration gain
	CommEnergyIC(0,Lphi,0x0000); //Line calibration angle
	CommEnergyIC(0,PStartTh,0x08BD); //Active Startup Power Threshold
	CommEnergyIC(0,PNolTh,0x0000); //Active No-Load Power Threshold
	CommEnergyIC(0,QStartTh,0x0AEC); //Reactive Startup Power Threshold
	CommEnergyIC(0,QNolTh,0x0000); //Reactive No-Load Power Threshold
	CommEnergyIC(0,MMode,0x9422); //Metering Mode Configuration. All defaults. See pg 31 of datasheet.
	CommEnergyIC(0,CSOne,0x4A34); //Write CSOne, as self calculated
    uint16_t crc_cal[10] ;
    uint16_t *p_crc_cal = crc_cal;
	//Set measurement calibration values
	CommEnergyIC(0,AdjStart,0x5678); //Measurement calibration startup command, registers 31-3A
	CommEnergyIC(0,Ugain,UgainValue);   //Voltage rms gain
    *(p_crc_cal++) = UgainValue;
	CommEnergyIC(0,IgainL,IgainLValue);   //L line current gain
    *(p_crc_cal++) = IgainLValue;
    *(p_crc_cal++) =  CommEnergyIC(1,IgainN,0xFFFF);
	CommEnergyIC(0,Uoffset,UoffsetValue);  //Voltage offset
    *(p_crc_cal++) = UoffsetValue;
    *(p_crc_cal++) = CommEnergyIC(1,IoffsetN,0xFFFF);
    CommEnergyIC(0,IoffsetL,IoffsetLValue); //L line current offset
    *(p_crc_cal++) = IoffsetLValue;
	CommEnergyIC(0,PoffsetL,PoffsetLValue); //L line active power offset
    *(p_crc_cal++) = PoffsetLValue;
	CommEnergyIC(0,QoffsetL,QoffsetLValue); //L line reactive power offset
    *(p_crc_cal++) = QoffsetLValue;
    *(p_crc_cal++) = CommEnergyIC(1,PoffsetN,0xFFFF);  //N Line Active Power Offset
    *(p_crc_cal++) = CommEnergyIC(1,QoffsetN,0xFFFF);
    uint16_t CS2 = calculate_CSTwo(crc_cal, 10); //Calculate checksum for registers 31-3A
    CommEnergyIC(0,CSTwo,CS2); //Write CSTwo, as self calculated

	CommEnergyIC(0,CalStart,0x8765); //Checks correctness of 21-2B registers and starts normal metering if ok
	CommEnergyIC(0,AdjStart,0x8765); //Checks correctness of 31-3A registers and starts normal measurement  if ok

	systemstatus = GetSysStatus();

    return systemstatus;
}

float  GetLineVoltage(){
	unsigned short voltage=CommEnergyIC(1,Urms,0xFFFF);
	return (float)voltage/100;
}

float GetLineCurrent(){
	unsigned short current=CommEnergyIC(1,Irms,0xFFFF);
    return (float)current/1000;
}

float GetActivePower(){
	short int apower= (short int)CommEnergyIC(1,Pmean,0xFFFF); //Complement, MSB is signed bit
	return (float)apower;
}

float GetFrequency(){
	unsigned short freq=CommEnergyIC(1,Freq,0xFFFF);
	return (float)freq/100;
}

unsigned short GetSysStatus(){
	return CommEnergyIC(1,SysStatus,0xFFFF);
}
#endif
/* End of atm90e26.c */
