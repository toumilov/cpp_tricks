#ifndef LEDGPIO_H
#define LEDGPIO_H

#define DEVICE_NAME "ledblink" /* /dev/ledblink */
#define GPIO_MIN_PORT 2
#define GPIO_MAX_PORT 27

typedef enum {
    LED_OP_START = 1,
    LED_OP_STOP,
    LED_OP_SET,
    LED_OP_GET
} led_operation_t;

typedef struct {
    led_operation_t operation :6;
    unsigned        leds      :26;
} led_cmd_t;

#endif
