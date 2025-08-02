#ifndef __DEFINE__
#define __DEFINE__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>

typedef enum
{
    FLAG_OFF,
    FLAG_ON,
}FLAG_INDEX;

#define LINUX_COMMAND_LEN 1024
#define PATH_MAX_LEN 1024
#define INPUT_MAX_LEN 1024

#define printr(fmt, ...) printf("[\x1b[31m%s\x1b[0m]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#define printy(fmt, ...) printf("[\x1b[32m%s\x1b[0m]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#define printg(fmt, ...) printf("[\x1b[33m%s\x1b[0m]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#define printb(fmt, ...) printf("[\x1b[34m%s\x1b[0m]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)

#endif