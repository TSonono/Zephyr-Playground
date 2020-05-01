/**
 * @file main.c
 *
 * Description
 *
 */

/*======= Includes ==========================================================*/

#include <sys/printk.h>
#include <zephyr.h>

/*======= Local Macro Definitions ===========================================*/

#define GPIO_REGISTER_BASE 0x50000000

#define GPIO_CONFIG_OFFSET 0x700
#define GPIO_LED0_PIN      13
#define LED0_PIN13_OFFSET  0x04 * GPIO_LED0_PIN

#define GPIO_OUT_OFFSET    0x504

/*======= Type Definitions ==================================================*/
/*======= Local function prototypes =========================================*/
/*======= Local variable declarations =======================================*/
/*======= Thread prototypes =================================================*/
/*======= Kernal Services Initialization ====================================*/
/*======= Global function implementations ===================================*/
/*======= Local function implementations ====================================*/
/*======= Thread implementations ============================================*/

// System thread
void main(void)
{
    u32_t *baseAddressGpio = (u32_t *)GPIO_REGISTER_BASE;
    u32_t ledRegOffset = (GPIO_CONFIG_OFFSET + LED0_PIN13_OFFSET)/sizeof(u32_t);
    u32_t *ledReg = baseAddressGpio + ledRegOffset;

    *ledReg |= 1 << 0;
    *ledReg |= 1 << 1;

    u32_t *ledSet = (u32_t *)GPIO_REGISTER_BASE + (GPIO_OUT_OFFSET/sizeof(u32_t));

    bool current = true;
    while (1)
    {
        if (current)
        {
            *ledSet |= current << GPIO_LED0_PIN;
        }
        else
        {
            *ledSet &= current << GPIO_LED0_PIN;
        }

        current = !current;
        // From Zephyr:
        k_msleep(1000);
    }
}
