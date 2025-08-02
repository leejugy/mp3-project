#include "codec_ctl.h"
#include "alsa_ctl.h"
#include "que_ctl.h"

static void thread_codec(void *arg)
{
    alsa_ctl_t alsa_ctl = {0, };
    alsa_set_t alsa_set = {0, };
    FLAG_INDEX codec_init = FLAG_OFF;

    while(1)
    {
        if(codec_init == FLAG_OFF)
        {
            if(alsa_init(&alsa_ctl) < 0)
            {
                printr("fail to init codec");
            }
            else
            {
                codec_init = FLAG_ON;
            }
        }
        else
        {
            if(que_pop_index(QUE_ALSA_SET, &alsa_set, sizeof(alsa_set)) > 0)
            {
                alsa_control_set(&alsa_ctl, &alsa_set);
            }

            alsa_control(&alsa_ctl);
        }

        usleep(1000);
    }
}

#define THREAD_TEST

#ifdef THREAD_TEST
static void codec_test_print_test_list()
{
    system("clear");
    printf("===========================\n");
    printf("1. change music \n");
    printf("2. restart \n");
    printf("3. pause \n");
    printf("4. resume \n");
    printf("5. stop \n");
    printf("6. volume control\n");
    printf("===========================\n");
    printf(">>");
}

static int codec_test_change_music(alsa_set_t *alsa_set)
{
    char path[PATH_MAX_LEN] = {0, };
    int ret = 0 ;

    printf("route : ");
    (void)fgets(path, sizeof(path), stdin);
    strtok(path, "\n");
    if((ret = access(path, F_OK)) < 0)
    {
        printr("fail to access file : %s", strerror(errno));
        return -1;
    }

    alsa_set->control = ALSA_CONTROL_CHANGE_MUSIC;
    strcpy(alsa_set->play_audio_path, path);
    return 1;
}

static int codec_test_restart(alsa_set_t *alsa_set)
{
    alsa_set->control = ALSA_CONTROL_RESTART;
    return 1;
}

static int codec_test_pause(alsa_set_t *alsa_set)
{
    alsa_set->control = ALSA_CONTROL_PAUSE;
    return 1;
}

static int codec_test_resume(alsa_set_t *alsa_set)
{
    alsa_set->control = ALSA_CONTROL_RESUME;
    return 1;
}

static int codec_test_stop(alsa_set_t *alsa_set)
{
    alsa_set->control = ALSA_CONTROL_STOP;
    return 1;
}

static int codec_test_volume_control(alsa_set_t *alsa_set)
{
    char input[INPUT_MAX_LEN] = {0, };
    int volume = 0;

    printf("volume(0 ~ 100) : ");
    (void)fgets(input, sizeof(input), stdin);
    strtok(input, "\n");

    volume = atoi(input);
    if(0 > volume || volume > 100)
    {
        printr("input is out of range : %d", volume);
        return -1;
    }
    alsa_set->control = ALSA_CONTROL_VOLUME_CONTROL;
    alsa_set->volume = volume;
    return 1;
}

static int codec_test_get_command(alsa_set_t *alsa_set)
{
    char cmd[LINUX_COMMAND_LEN] = {0, };
    int ret = 0;

    (void)fgets(cmd, sizeof(cmd), stdin);
    strtok(cmd, "\n");
    if(strlen(cmd) != 1)
    {
        printr("unkwon string : %s", cmd);
        return -1;
    }

    switch (atoi(cmd))
    {
    case 1:
        ret = codec_test_change_music(alsa_set);
        break;

    case 2:
        ret = codec_test_restart(alsa_set);
        break;

    case 3:
        ret = codec_test_pause(alsa_set);
        break;

    case 4:
        ret = codec_test_resume(alsa_set);
        break;

    case 5:
        ret = codec_test_stop(alsa_set);
        break;

    case 6:
        ret = codec_test_volume_control(alsa_set);
        break;
    
    default:
        printr("unkwon cmd : %d", atoi(cmd));
        ret = -1;
        break;
    }

    return ret;
}

static void thread_codec_test()
{
    alsa_set_t alsa_set = {0, };
    
    while(1)
    {
        codec_test_print_test_list();
        if(codec_test_get_command(&alsa_set) > 0)
        {
            que_push_index(QUE_ALSA_SET, &alsa_set, sizeof(alsa_set));
        }
    }
}
#endif

void start_thread_codec()
{
    pthread_t tid = 0;
    int ret = 0;

    if((ret = pthread_create(&tid, NULL, (void *)&thread_codec, NULL)) < 0)
    {
        printr("fail to create codec thread : %s", strerror(errno));
        exit(0);
    }

    if((ret = pthread_detach(tid)) < 0)
    {
        printr("fail to detach codec thread : %s", strerror(errno));
        exit(0);
    }

    #ifdef THREAD_TEST
    {
        if((ret = pthread_create(&tid, NULL, (void *)&thread_codec_test, NULL)) < 0)
        {
            printr("fail to create codec thread : %s", strerror(errno));
            exit(0);
        }

        if((ret = pthread_detach(tid)) < 0)
        {
            printr("fail to detach codec thread : %s", strerror(errno));
            exit(0);
        }
    }
    #endif
}