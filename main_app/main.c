#include "define.h"
#include "alsa_ctl.h"
#include "que_ctl.h"
#include "gpio_ctl.h"
#include "codec_ctl.h"
#include "sqlite3_ctl.h"

void main_init()
{
    gpio_init_all();
    que_init_all();
    sql_init_all();
}

void main_thread_init()
{
    start_thread_codec();
}

int main()
{
    main_init();
    main_thread_init();

    while(1);
}