#include "define.h"
#include "alsa_clt.h"

int main()
{
    alsa_ctl_t alsa_clt = {0, };
    alsa_set_t alsa_set = {0, };
    alsa_set.control = ALSA_CONTROL_CHANGE_MUSIC;
    sprintf(alsa_set.play_audio_path, "/root/F1.wav");
    alsa_init(&alsa_clt);
    alsa_control_set(&alsa_clt, &alsa_set);
    while(1)
    {
        alsa_control(&alsa_clt);
    }
}