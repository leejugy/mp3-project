#include "gpio_ctl.h"
gpio_ctl_t gpio_ctl[GPIO_INDEX_MAX] = {0, };

static int gpio_init(gpio_ctl_t *gpio_ctl, gpio_init_t *gpio_it)
{
    int ret = 0;
    int fd = 0;
    char route[PATH_MAX_LEN] = {0, };

    sprintf(route, "/dev/gpiochip%d", gpio_it->gpio_chip);
    if((ret = open(route, O_RDWR)) < 0)
    {
        printr("fail to init gpiochip%d", gpio_it->gpio_chip);
        return -1;
    }

    fd = ret;

    if((ret = ioctl(fd, GPIO_V2_GET_LINE_IOCTL, &gpio_it->req)) < 0)
    {
        printr("fail to get gpio ioctl line : %s", strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);
    gpio_ctl->fd = gpio_it->req.fd;
    return 1;
}

static int gpio_read(gpio_ctl_t *gpio_ctl, int *val)
{
    int ret = 0;

    gpio_ctl->val.mask = GPIO_MASK_1BIT;
    if((ret = ioctl(gpio_ctl->fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &gpio_ctl->val)) < 0)
    {
        printr("fail to get gpio ioctl line value : %s", strerror(errno));
        return -1;
    }
    *val = gpio_ctl->val.bits;
    return 1;
}

static int gpio_write(gpio_ctl_t *gpio_ctl, int *val)
{
    int ret = 0;

    gpio_ctl->val.mask = GPIO_MASK_1BIT;
    gpio_ctl->val.bits = *val;
    if((ret = ioctl(gpio_ctl->fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &gpio_ctl->val)) < 0)
    {
        printr("fail to set gpio ioctl line value : %s", strerror(errno));
        return -1;
    }
    return 1;
}

int gpio_read_index(GPIO_INDEX index, int *val)
{
    return gpio_read(&gpio_ctl[index], val);
}

int gpio_write_index(GPIO_INDEX index, int *val)
{
    return gpio_write(&gpio_ctl[index], val);
}

int gpio_init_all()
{
    gpio_init_t gpio_it = {0, };
    
    gpio_it.gpio_chip = GPIO_CHIP0;
    gpio_it.req.num_lines = GPIO_MAX_LINE_NUM;
    gpio_it.req.offsets[0] = GPIO_CODEC_SW_MUTE_LINE;
    gpio_it.req.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
    gpio_it.req.config.num_attrs = GPIO_MAX_LINE_NUM;
    gpio_it.req.config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES;
    gpio_it.req.config.attrs[0].attr.values = GPIO_LOW;
    gpio_it.req.config.attrs[0].mask = GPIO_MASK_1BIT;
    gpio_it.req.event_buffer_size = 0;
    sprintf(gpio_it.req.consumer, "CODEC_SW_MUTE");
    gpio_init(&gpio_ctl[GPIO_CODEC_SW_MUTE], &gpio_it);
}