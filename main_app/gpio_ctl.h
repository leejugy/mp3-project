#ifndef __GPIO_CTL__
#define __GPIO_CTL__

#include "define.h"
#include <linux/gpio.h>
#include <sys/ioctl.h>

#define GPIO_MAX_LINE_NUM 1
#define GPIO_MASK_1BIT 0x01
#define GPIO_CODEC_SW_MUTE_LINE 5

typedef enum
{
    GPIO_HIGH = 1,
    GPIO_LOW = 0,
}GPIO_VALUE;
typedef enum
{
    GPIO_CHIP0,
    GPIO_CHIP1,
    GPIO_CHIP2,
    GPIO_CHIP3,
    GPIO_CHIP4,
    GPIO_CHIP5,
}GPIO_CHIP;
typedef enum
{
    GPIO_CODEC_SW_MUTE,
    GPIO_INDEX_MAX,
}GPIO_INDEX;
typedef struct
{
    GPIO_CHIP gpio_chip;
    struct gpio_v2_line_request req;
}gpio_init_t;

typedef struct
{
    int fd;
    struct gpio_v2_line_values val;
}gpio_ctl_t;

int gpio_init_all();
int gpio_write_index(GPIO_INDEX index, int *val);
int gpio_read_index(GPIO_INDEX index, int *val);
#endif