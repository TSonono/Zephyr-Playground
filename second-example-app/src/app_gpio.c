/**
 * @file app_gpio.c
 *
 * Description
 *
 */

/*======= Includes ==========================================================*/

#include "app_gpio.h"

/*======= Local Macro Definitions ===========================================*/
/*======= Type Definitions ==================================================*/
/*======= Local function prototypes =========================================*/
/*======= Local variable declarations =======================================*/

static struct device *button_device[2];

/*======= Thread prototypes =================================================*/
/*======= Kernal Services Initialization ====================================*/
/*======= Global function implementations ===================================*/

void initButtonAsInterrupt(u8_t buttonNumber, gpio_callback_handler_t buttonIsr)
{
    static struct gpio_callback button_cb[2];

    if (buttonNumber > 1) {
        return;
    }

    gpio_pin_t   pin;
    gpio_flags_t flags;
    char *       device;
    switch (buttonNumber) {
    case 0:
        pin    = (gpio_pin_t)DT_ALIAS_SW0_GPIOS_PIN;
        flags  = (gpio_flags_t)DT_ALIAS_SW0_GPIOS_FLAGS;
        device = (char *)DT_ALIAS_SW0_GPIOS_CONTROLLER;
        break;
    case 1:
        pin    = (gpio_pin_t)DT_ALIAS_SW1_GPIOS_PIN;
        flags  = (gpio_flags_t)DT_ALIAS_SW1_GPIOS_FLAGS;
        device = (char *)DT_ALIAS_SW1_GPIOS_CONTROLLER;
        break;

    default:
        return;
        break;
    }

    button_device[buttonNumber] = device_get_binding(device);
    gpio_pin_configure(button_device[buttonNumber],
                       pin,
                       GPIO_INPUT | GPIO_INT_DEBOUNCE | flags);
    gpio_pin_interrupt_configure(button_device[buttonNumber],
                                 pin,
                                 GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&button_cb[buttonNumber], buttonIsr, BIT(pin));
    gpio_add_callback(button_device[buttonNumber], &button_cb[buttonNumber]);
}

/*======= Local function implementations ====================================*/
/*======= Thread implementations ============================================*/
