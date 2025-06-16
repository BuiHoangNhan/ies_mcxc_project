/*

*/
#ifndef IO_DRIVER_H
#define IO_DRIVER_H

#include <stdint.h>
#include "fsl_gpio.h"
#include "board.h"

#if LCD_CONTROLER

#define GPIO_AC_SENSING_MUX_SELA_PIN     2
#define GPIO_AC_SENSING_MUX_SELB_PIN     3

#define GPIO_AC_SENSING_MUX_SELA_B_GPIO    GPIOB
#define GPIO_GPIO_RELAY_1   19
#define GPIO_GPIO_RELAY_2   20
#define GPIO_GPIO_RELAY_3   21

#define GPIO_LCD_ON_GPIO                   GPIOC
#define GPIO_LCD_ON_PIN                    2

#define GPIO_RELAY_GPIO    GPIOE

#define GPIO_LED_1   18
#define GPIO_LED_2   29
#define GPIO_LED_3   30

#define GPIO_LED_GPIO    GPIOE

#define GPIO_BUTTON_1 5
#define GPIO_BUTTON_2 6
#define GPIO_BUTTON_3 7
#define GPIO_BUTTON_4 0
#define GPIO_BUTTON_5 1

#define GPIO_BUTTON_GPIO1    GPIOC
#define GPIO_BUTTON_GPIO2    GPIOD

#define GPIO_BUTTON_PORT1    PORTC
#define GPIO_BUTTON_PORT2    PORTD

#define GPIO_BUTTON_1_IRQ    PORTC_PORTD_IRQn

#define GPIO_BUTTON_2_IRQ    PORTC_PORTD_IRQn

typedef enum __enum_GPIO_button_t
{
    GPIO_BUTTON_LEFT = 0,
    GPIO_BUTTON_UP,
    GPIO_BUTTON_RIGHT,
    GPIO_BUTTON_BOTTOM,
    GPIO_BUTTON_MIDDLE = 4 ,
    GPIO_BUTTON_PAIRING = 4,
} enum_GPIO_button_t;

void Ac_sensing_sel_init(void);

void Ac_sensing_sel(uint8_t pin, uint8_t output);

void gpio_relay_init(void);

void Relay_set(uint8_t pin, uint8_t output);

void gpio_led_init(void);

void Led_set(uint8_t pin, uint8_t output);

void Button_init(void);

void LCD_power(uint8_t state);
#elif NODE_NEMA

#include "fsl_dac.h"

/* Component ID definition, used by tools. */
#ifndef FSL_COMPONENT_ID
#define FSL_COMPONENT_ID "platform.drivers.dac"
#endif



#define GPIO_DIM_GPIO   GPIOB

#define GPIO_DIM_PIN    0



void DimOff_set(uint8_t output);


#endif
#define GPIO_PAIR_PIN   1
#define GPIO_PAIR_GPIO  GPIOD
#define GPIO_PAIR_PORT  PORTD
#define GPIO_PAIR_BUTTON_IRQ    PORTC_PORTD_IRQn

#define GPIO_BOOT_PIN	0
#define GPIO_BOOT_GPIO  GPIOC
#define GPIO_BOOT_PORT  PORTC

#define GPIO_LORA_RST_GPIO GPIOD
#define GPIO_LORA_RST_PIN  5

void Pair_gpio_init(void);
void Boot_gpio_init(void);
uint8_t Boot_button_read(void);
uint8_t Pair_button_read(void);
void Lora_rst_init(void);
#endif /* IO_DRIVER_H */
