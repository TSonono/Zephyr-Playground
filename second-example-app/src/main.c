/**
 * @file main.c
 *
 * In this example, ISR handling in Zephyr is explored.
 *
 * An ISR is triggered when button 1 or button 2 on the board is pressed. When
 * this ISR is triggered, it will mock a sensor reading, and then signal to the
 * "data processing" thread that the data is ready to process. The signaling
 * will be done with a semaphore and the data will be put/retrieved in a stack.
 * Further more, one thread A has higher priority than the other. Give that both
 * threads are pre-emptable, thread A can take over while thread B is processing
 * if button 1 shortly after button 2 is pressed (while thread B is processing
 * data).
 *
 * The ISR that triggers thread A, does the (mock) sensor data measurement in
 * the ISR code since this sensor needs time to make a measurement. The ISR that
 * triggers thread B is a sensor that does not rely on time to get an accurate
 * measurement. Therefore, the thread can start the trigger measuring process.
 *
 * TODO: Mock something for thread B to process.
 *
 */

/*======= Includes ==========================================================*/

#include "app_gpio.h"
#include <inttypes.h>
#include <sys/printk.h>
#include <zephyr.h>

/*======= Local Macro Definitions ===========================================*/

#define STACK_SIZE 1024

#define PRIORITY_A 1
#define PRIORITY_B 2

#define ONE_SECOND_US 1000000

/*======= Type Definitions ==================================================*/
/*======= Local function prototypes =========================================*/

void button1PressedIsr(struct device *       port,
                       struct gpio_callback *cb,
                       gpio_port_pins_t      pins);

void button2PressedIsr(struct device *       port,
                       struct gpio_callback *cb,
                       gpio_port_pins_t      pins);

/*======= Local variable declarations =======================================*/

/*======= Thread prototypes =================================================*/

void processingThreadA(void *dummy1, void *dummy2, void *dummy3);
void processingThreadB(void *dummy1, void *dummy2, void *dummy3);

/*======= Kernal Services Initialization ====================================*/

K_THREAD_DEFINE(dataHandlerA,
                STACK_SIZE,
                processingThreadA,
                NULL,
                NULL,
                NULL,
                PRIORITY_A,
                0,
                0);

K_THREAD_DEFINE(dataHandlerB,
                STACK_SIZE,
                processingThreadB,
                NULL,
                NULL,
                NULL,
                PRIORITY_B,
                0,
                0);

K_SEM_DEFINE(dataHandler1Sem, 0, 1);
K_SEM_DEFINE(dataHandler2Sem, 0, 1);

K_STACK_DEFINE(mockDataA, 3);
K_STACK_DEFINE(mockDataB, 2);

/*======= Global function implementations ===================================*/
/*======= Local function implementations ====================================*/

void button1PressedIsr(struct device *       port,
                       struct gpio_callback *cb,
                       gpio_port_pins_t      pins)
{
    printk("Button 1 was pressed!\r\n");

    u8_t mockTemp     = 13;
    u8_t mockHumdity  = 30;
    u8_t mockPressure = 250;

    int status;
    status |= k_stack_push(&mockDataA, (stack_data_t)&mockTemp);
    status |= k_stack_push(&mockDataA, (stack_data_t)&mockHumdity);
    status |= k_stack_push(&mockDataA, (stack_data_t)&mockPressure);
    if (status != 0) {
        printk("A Stack operation failed");
        return;
    }

    printk("Measurement 1 done!\r\n");
    k_sem_give(&dataHandler1Sem);
}

void button2PressedIsr(struct device *       port,
                       struct gpio_callback *cb,
                       gpio_port_pins_t      pins)
{
    printk("Button 2 was pressed!\r\n");
    k_busy_wait(ONE_SECOND_US * 4);
    printk("Measurement 2 done!\r\n");
    k_sem_give(&dataHandler2Sem);
}

/*======= Thread implementations ============================================*/

void processingThreadA(void *dummy1, void *dummy2, void *dummy3)
{
    initButtonAsInterrupt(0, button1PressedIsr);
    u8_t *mockTemp;
    u8_t *mockHumdity;
    u8_t *mockPressure;
    while (1) {
        k_sem_take(&dataHandler1Sem, K_FOREVER);
        printk("Starting processing of Measurement 1 data \r\n");

        int status;
        status |= k_stack_pop(&mockDataA, (stack_data_t *)&mockPressure, K_FOREVER);
        status |= k_stack_pop(&mockDataA, (stack_data_t *)&mockHumdity, K_FOREVER);
        status |= k_stack_pop(&mockDataA, (stack_data_t *)&mockTemp, K_FOREVER);
        if (status != 0) {
            printk("A Stack operation failed");
            return;
        }

        printk("mockTemperature: %d\r\n", *mockTemp);
        printk("mockHumdity: %d\r\n", *mockHumdity);
        printk("mockPressure: %d\r\n", *mockPressure);

        k_busy_wait(ONE_SECOND_US * 2);
        printk("Finished processing of Measurement 1 data \r\n");
    }
}

void processingThreadB(void *dummy1, void *dummy2, void *dummy3)
{
    initButtonAsInterrupt(1, button2PressedIsr);
    while (1) {
        k_sem_take(&dataHandler2Sem, K_FOREVER);
        printk("Hello world from B!\r\n");
        k_busy_wait(4000000);
        printk("Finished processing in B\r\n");
    }
}
