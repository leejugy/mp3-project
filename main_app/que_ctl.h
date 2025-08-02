#ifndef __QUE_CTL__
#define __QUE_CTL__

#include "define.h"
typedef enum
{
    QUE_ALSA_SET,
    QUE_INDEX_MAX,
}QUE_INDEX;

typedef struct
{
    void *buf;
    int buf_size;
    int que_number;
    int front;
    int rear;
    sem_t sem;
}que_t;

typedef struct
{
    int buf_size;
    int que_number;
}que_init_t;

int que_init_all();
int que_push_index(QUE_INDEX index, void *buf, int buf_size);
int que_pop_index(QUE_INDEX index, void *buf, int buf_size);
#endif