/**
 * @file app_gpio.c
 *
 * Description
 *
 */

/*======= Includes ==========================================================*/

#include "app_gpio.h"

/*======= Local Macro Definitions ===========================================*/

#define FLAGS_OR_ZERO(node)						\
	COND_CODE_1(DT_PHA_HAS_CELL(node, gpios, flags),		\
		    (DT_GPIO_FLAGS(node, gpios)),			\
		    (0))

#define SW0_NODE	DT_ALIAS(sw0)
#define SW1_NODE	DT_ALIAS(sw1)

#if DT_NODE_HAS_STATUS(SW0_NODE, okay)
#define SW0_GPIO_LABEL	DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN	DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_GPIO_FLAGS	(GPIO_INPUT | FLAGS_OR_ZERO(SW0_NODE))
#define SW1_GPIO_LABEL	DT_GPIO_LABEL(SW1_NODE, gpios)
#define SW1_GPIO_PIN	DT_GPIO_PIN(SW1_NODE, gpios)
#define SW1_GPIO_FLAGS	(GPIO_INPUT | FLAGS_OR_ZERO(SW1_NODE))
#else
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

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
        pin    = (gpio_pin_t)SW0_GPIO_PIN;
        flags  = (gpio_flags_t)SW0_GPIO_FLAGS;
        device = (char *)SW0_GPIO_LABEL;
        break;
    case 1:
        pin    = (gpio_pin_t)SW1_GPIO_PIN;
        flags  = (gpio_flags_t)SW1_GPIO_FLAGS;
        device = (char *)SW1_GPIO_LABEL;
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
