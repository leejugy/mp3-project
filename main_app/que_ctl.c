#include "que_ctl.h"
#include "alsa_ctl.h"

que_t q[QUE_INDEX_MAX] = {0, };

static int que_init(que_t *que, que_init_t *que_it)
{
    int ret = 0;
    if(que_it->buf_size <= 0)
    {
        printr("invalid buf size");
        return -1;
    }

    if(que_it->que_number <= 0)
    {
        printr("invalid que number");
        return -1;
    }

    que->buf_size = que_it->buf_size;
    que->que_number = que_it->que_number;
    que->buf = calloc(que->buf_size, que->que_number);
    que->front = 0;
    que->rear = 0;
    assert(que->buf);

    if((ret = sem_init(&que->sem, 0, 1)) < 0)
    {
        printr("fail to init sem : %s", strerror(errno));
        return -1;
    }
}

static int que_pop(que_t *que, void *buf, int buf_size)
{
    if(buf_size < que->buf_size)
    {
        printr("que buf size is invalid");
        return -1;
    }

    if(buf == NULL)
    {
        printr("buf is null");
        return -1;
    }

    if(que->front == que->rear)
    {
        return -1;
    }

    memcpy(buf, que->buf + que->rear * que->buf_size, buf_size);
    que->rear = (que->rear + 1) % que->que_number;
    return buf_size;
}

static int que_push(que_t *que, void *buf, int buf_size)
{
    if(buf_size > que->buf_size)
    {
        printr("que buf size is invalid");
        return -1;
    }

    if(buf == NULL)
    {
        printr("buf is null");
        return -1;
    }

    if((que->front + 1) % que->que_number == que->rear)
    {
        printr("fail to push que : que is full");
        return -1;
    }

    memcpy(que->buf + que->front * que->buf_size, buf, buf_size);
    que->front = (que->front + 1) % que->que_number;
    return buf_size;
}

int que_init_all()
{
    que_init_t que_it = {0, };
    
    que_it.buf_size = sizeof(alsa_set_t);
    que_it.que_number = 64;
    que_init(&q[QUE_ALSA_SET], &que_it);
}

int que_push_index(QUE_INDEX index, void *buf, int buf_size)
{
    int ret = 0;
    sem_wait(&q[index].sem);
    ret = que_push(&q[index], buf, buf_size);
    sem_post(&q[index].sem);
    return ret;
}

int que_pop_index(QUE_INDEX index, void *buf, int buf_size)
{
    int ret = 0;
    sem_wait(&q[index].sem);
    ret = que_pop(&q[index], buf, buf_size);
    sem_post(&q[index].sem);
    return ret;
}