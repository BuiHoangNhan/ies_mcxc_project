/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2018, 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "IO_driver.h"
#include "fsl_port.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if LCD_CONTROLER
gpio_pin_config_t ac_sensing_sel_config[2] = {
    {kGPIO_DigitalOutput, 0},
    {kGPIO_DigitalOutput, 0},
};

gpio_pin_config_t relay_config[3] = {
    {kGPIO_DigitalOutput, 0},
    {kGPIO_DigitalOutput, 0},
    {kGPIO_DigitalOutput, 0},
};

gpio_pin_config_t led_config[3] = {
    {kGPIO_DigitalOutput, 0},
    {kGPIO_DigitalOutput, 0},
    {kGPIO_DigitalOutput, 0},
};

gpio_pin_config_t button_config[5] = {
    {kGPIO_DigitalInput, 0},
    {kGPIO_DigitalInput, 0},
    {kGPIO_DigitalInput, 0},
    {kGPIO_DigitalInput, 0},
    {kGPIO_DigitalInput, 0},
};

void Ac_sensing_sel_init(void)
{
    // Update references to use the new array name
    GPIO_PinInit(GPIO_AC_SENSING_MUX_SELA_B_GPIO, GPIO_AC_SENSING_MUX_SELA_PIN, &ac_sensing_sel_config[0]);
    GPIO_PinInit(GPIO_AC_SENSING_MUX_SELA_B_GPIO, GPIO_AC_SENSING_MUX_SELB_PIN, &ac_sensing_sel_config[1]);
}

void gpio_relay_init(void)
{
    // Update references to use the new array name
#if MCXC144_BOARD
    GPIO_PinInit(GPIO_RELAY_GPIO, GPIO_GPIO_RELAY_1, &relay_config[0]);
#endif
    GPIO_PinInit(GPIO_RELAY_GPIO, GPIO_GPIO_RELAY_2, &relay_config[1]);
    GPIO_PinInit(GPIO_RELAY_GPIO, GPIO_GPIO_RELAY_3, &relay_config[2]);
}

void gpio_led_init(void)
{
    // Update references to use the new array name
#if MCXC144_BOARD
    GPIO_PinInit(GPIO_LED_GPIO, GPIO_LED_1, &led_config[0]);
#endif
    GPIO_PinInit(GPIO_LED_GPIO, GPIO_LED_2, &led_config[1]);
    GPIO_PinInit(GPIO_LED_GPIO, GPIO_LED_3, &led_config[2]);
}

void Button_init(void)
{   
    PORT_SetPinInterruptConfig(GPIO_BUTTON_PORT1, GPIO_BUTTON_1, kPORT_InterruptFallingEdge);
    PORT_SetPinInterruptConfig(GPIO_BUTTON_PORT1, GPIO_BUTTON_2, kPORT_InterruptFallingEdge);
    PORT_SetPinInterruptConfig(GPIO_BUTTON_PORT1, GPIO_BUTTON_3, kPORT_InterruptFallingEdge);
#if MCXC144_BOARD
    PORT_SetPinInterruptConfig(GPIO_BUTTON_PORT2, GPIO_BUTTON_4, kPORT_InterruptFallingEdge);
    PORT_SetPinInterruptConfig(GPIO_BUTTON_PORT2, GPIO_BUTTON_5, kPORT_InterruptFallingEdge);
#endif
    EnableIRQ(GPIO_BUTTON_1_IRQ);
    // Update references to use the new array name
    GPIO_PinInit(GPIO_BUTTON_GPIO1, GPIO_BUTTON_1, &button_config[0]);
    GPIO_PinInit(GPIO_BUTTON_GPIO1, GPIO_BUTTON_2, &button_config[1]);
    GPIO_PinInit(GPIO_BUTTON_GPIO1, GPIO_BUTTON_3, &button_config[2]);
#if MCXC144_BOARD
    GPIO_PinInit(GPIO_BUTTON_GPIO2, GPIO_BUTTON_4, &button_config[3]);
    GPIO_PinInit(GPIO_BUTTON_GPIO2, GPIO_BUTTON_5, &button_config[4]);
#endif
}

void PORTC_PORTD_IRQHandler(void)
{
	if(GPIO_PortGetInterruptFlags(GPIO_BUTTON_GPIO1)&1U<<GPIO_BUTTON_1)
    {
        GPIO_PortClearInterruptFlags(GPIO_BUTTON_GPIO1,1U<<GPIO_BUTTON_1);
    }
    else if(GPIO_PortGetInterruptFlags(GPIO_BUTTON_GPIO1)&1U<<GPIO_BUTTON_2)
    {
        GPIO_PortClearInterruptFlags(GPIO_BUTTON_GPIO1,1U<<GPIO_BUTTON_2);
    }
    else if(GPIO_PortGetInterruptFlags(GPIO_BUTTON_GPIO1)&1U<<GPIO_BUTTON_3)
    {
        GPIO_PortClearInterruptFlags(GPIO_BUTTON_GPIO1,1U<<GPIO_BUTTON_3);
    }
#if MCXC144_BOARD
    else if(GPIO_PortGetInterruptFlags(GPIO_BUTTON_GPIO2)&1U<<GPIO_BUTTON_4)
    {
        GPIO_PortClearInterruptFlags(GPIO_BUTTON_GPIO2,1U<<GPIO_BUTTON_4);
    }
    else if(GPIO_PortGetInterruptFlags(GPIO_BUTTON_GPIO2)&1U<<GPIO_BUTTON_5)
    {
        GPIO_PortClearInterruptFlags(GPIO_BUTTON_GPIO2,1U<<GPIO_BUTTON_5);
    }
#endif
    SDK_ISR_EXIT_BARRIER;
}


void Relay_set(uint8_t pin, uint8_t output)
{
    GPIO_PinWrite(GPIO_RELAY_GPIO, pin, output);
}


void Led_set(uint8_t pin, uint8_t output)
{
    GPIO_PinWrite(GPIO_LED_GPIO, pin, output);
}

void Ac_sensing_sel(uint8_t pin, uint8_t output)
{
    GPIO_PinWrite(GPIO_AC_SENSING_MUX_SELA_B_GPIO, pin, output);
}
#elif NODE_NEMA
gpio_pin_config_t DimOff_config =  {kGPIO_DigitalOutput, 0};
void DimOff_gpio_init(void)
{
    GPIO_PinInit(GPIO_DIM_GPIO, GPIO_DIM_PIN, &DimOff_config);
}

void DimOff_set(uint8_t output)
{
    GPIO_PinWrite(GPIO_DIM_GPIO, GPIO_DIM_PIN, output);
}

#endif

