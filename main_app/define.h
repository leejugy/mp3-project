#ifndef __DEFINE__
#define __DEFINE__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>

#define printr(fmt, ...) printf("[\x1b[31m%s\x1b[0m]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#define printy(fmt, ...) printf("[\x1b[32m%s\x1b[0m]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#define printg(fmt, ...) printf("[\x1b[33m%s\x1b[0m]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#define printb(fmt, ...) printf("[\x1b[34m%s\x1b[0m]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)

#endif