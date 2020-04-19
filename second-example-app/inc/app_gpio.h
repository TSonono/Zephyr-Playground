#ifndef _app_gpio_H_
#define _app_gpio_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file app_gpio.h
 *
 * Description
 *
 */

/*======= Includes ==========================================================*/

#include <drivers/gpio.h>
#include <inttypes.h>

/*======= Public macro definitions ==========================================*/
/*======= Type Definitions and declarations =================================*/
/*======= Public variable declarations ======================================*/
/*======= Public function declarations ======================================*/

void initButtonAsInterrupt(u8_t                    buttonNumber,
                           gpio_callback_handler_t buttonIsr);

#ifdef __cplusplus
}
#endif

#endif /* _app_gpio_H_ */