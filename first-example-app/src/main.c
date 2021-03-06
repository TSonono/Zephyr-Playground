/**
 * @file main.c
 *
 * Description
 *
 */

/*======= Includes ==========================================================*/

#include <drivers/gpio.h>
#include <sys/printk.h>
#include <zephyr.h>

/*======= Local Macro Definitions ===========================================*/

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay) && DT_NODE_HAS_STATUS(LED1_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED1	DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN0	DT_GPIO_PIN(LED0_NODE, gpios)
#define PIN1	DT_GPIO_PIN(LED1_NODE, gpios)
#if DT_PHA_HAS_CELL(LED0_NODE, gpios, flags) && DT_PHA_HAS_CELL(LED1_NODE, gpios, flags)
#define FLAGS0	DT_GPIO_FLAGS(LED0_NODE, gpios)
#define FLAGS1	DT_GPIO_FLAGS(LED1_NODE, gpios)
#endif
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led devicetree alias is not defined"
#endif

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

/* delay between greetings (in ms) */
#define SLEEPTIME 500

/*======= Type Definitions ==================================================*/
/*======= Local function prototypes =========================================*/

/*
 * @param my_name      thread identification string
 * @param my_sem       thread's own semaphore
 * @param other_sem    other thread's semaphore
 */
static void helloLoop(const char *  my_name,
                      struct k_sem *my_sem,
                      struct k_sem *other_sem,
                      gpio_pin_t    led_pin);

/*======= Local variable declarations =======================================*/

static struct k_thread threadB_data;

/*======= Thread prototypes =================================================*/

void threadA(void *dummy1, void *dummy2, void *dummy3);
void threadB(void *dummy1, void *dummy2, void *dummy3);

/*======= Kernal Services Initialization ====================================*/

/* define semaphores */

K_SEM_DEFINE(threadA_sem, 1, 1); /* starts off "available" */
K_SEM_DEFINE(threadB_sem, 0, 1); /* starts off "not available" */

K_THREAD_DEFINE(thread_a, STACKSIZE, threadA, NULL, NULL, NULL, PRIORITY, 0, 0);

K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);

/*======= Global function implementations ===================================*/
/*======= Local function implementations ====================================*/

static void helloLoop(const char *  my_name,
                      struct k_sem *my_sem,
                      struct k_sem *other_sem,
                      gpio_pin_t    led_pin)
{
    const char *   tname;
    struct device *dev_led;

    dev_led = device_get_binding(LED0);

    while (1) {
        /* take my semaphore */
        k_sem_take(my_sem, K_FOREVER);

        /* say "hello" */
        tname = k_thread_name_get(k_current_get());
        if (tname == NULL) {
            printk("%s: Hello World from %s!\n", my_name, CONFIG_BOARD);
        } else {
            printk("%s: Hello World from %s!\n", tname, CONFIG_BOARD);
        }

        gpio_pin_set(dev_led, led_pin, 1);

        /* wait a while, then let other thread have a turn */
        k_msleep(SLEEPTIME);
        gpio_pin_set(dev_led, led_pin, 0);
        k_sem_give(other_sem);
    }
}

/*======= Thread implementations ============================================*/

// System thread
void main(void)
{
    struct device *dev;
    int            ret;

    dev = device_get_binding(LED0);
    if (dev == NULL) {
        printk("Error, could not bind device\r\n");
        return;
    }

    ret = gpio_pin_configure(dev,
                             PIN0,
                             GPIO_OUTPUT_HIGH);
    if (ret < 0) {
        printk("Error, could configure pin %d\r\n", PIN0);
    }

    ret = gpio_pin_configure(dev,
                             PIN1,
                             GPIO_OUTPUT_HIGH);
    if (ret < 0) {
        printk("Error, could configure pin %d\r\n", PIN0);
    }
    return;
}

void threadB(void *dummy1, void *dummy2, void *dummy3)
{
    ARG_UNUSED(dummy1);
    ARG_UNUSED(dummy2);
    ARG_UNUSED(dummy3);

    /* invoke routine to ping-pong hello messages with threadA */
    helloLoop(__func__, &threadB_sem, &threadA_sem, PIN1);
}

/* threadA is a static thread that is spawned automatically */

void threadA(void *dummy1, void *dummy2, void *dummy3)
{
    ARG_UNUSED(dummy1);
    ARG_UNUSED(dummy2);
    ARG_UNUSED(dummy3);

    /* spawn threadB */
    k_tid_t tid = k_thread_create(&threadB_data,
                                  threadB_stack_area,
                                  STACKSIZE,
                                  threadB,
                                  NULL,
                                  NULL,
                                  NULL,
                                  PRIORITY,
                                  0,
                                  K_NO_WAIT);

    k_thread_name_set(tid, "thread_b");

    /* invoke routine to ping-pong hello messages with threadB */
    helloLoop(__func__, &threadA_sem, &threadB_sem, PIN0);
}
