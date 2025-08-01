#include "alsa_clt.h"

int alsa_init(alsa_ctl_t *alsa_ctl)
{   
    int ret = 0;
    if((ret = snd_pcm_open(&alsa_ctl->handle, ALSA_SOUND_CARD_NAME, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        printr("fail to open pcm : %s", snd_strerror(ret));
        return -1;
    }
    return 1;
}

static int alsa_set_hw_params(alsa_ctl_t *alsa_ctl)
{
    int ret = 0;
    int dir = 0;

    if((ret = snd_pcm_hw_params_malloc(&alsa_ctl->hw_params)) < 0)
    {
        printr("fail to malloc hw params : %s", snd_strerror(ret));
        return -1;
    }

    if((ret = snd_pcm_hw_params_any(alsa_ctl->handle, alsa_ctl->hw_params)) < 0)
    {
        snd_pcm_hw_params_free(alsa_ctl->hw_params);
        printr("fail to hw params any : %s", snd_strerror(ret));
        return -1;
    }

    if((ret = snd_pcm_hw_params_set_access(alsa_ctl->handle, alsa_ctl->hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        snd_pcm_hw_params_free(alsa_ctl->hw_params);
        printr("fail to set hw access : %s", snd_strerror(ret));
        return -1;  
    }

    if((ret = snd_pcm_hw_params_set_format(alsa_ctl->handle, alsa_ctl->hw_params, alsa_ctl->audio_info.format)) < 0)
    {
        snd_pcm_hw_params_free(alsa_ctl->hw_params);
        printr("fail to set hw format : %s", snd_strerror(ret));
        return -1;
    }

    if((ret = snd_pcm_hw_params_set_channels_near(alsa_ctl->handle, alsa_ctl->hw_params, &alsa_ctl->audio_info.channel)) < 0)
    {
        snd_pcm_hw_params_free(alsa_ctl->hw_params);
        printr("fail to set hw format : %s", snd_strerror(ret));
        return -1;
    }

    if((ret = snd_pcm_hw_params_set_rate_near(alsa_ctl->handle, alsa_ctl->hw_params, &alsa_ctl->audio_info.sample_rate, &dir)) < 0)
    {
        snd_pcm_hw_params_free(alsa_ctl->hw_params);
        printr("fail to set hw format : %s", snd_strerror(ret));
        return -1;
    }

    if((ret = snd_pcm_hw_params(alsa_ctl->handle, alsa_ctl->hw_params)) < 0)
    {
        snd_pcm_hw_params_free(alsa_ctl->hw_params);
        printr("fail to open pcm hw params : %s", snd_strerror(ret));
        return -1;
    }

    snd_pcm_hw_params_free(alsa_ctl->hw_params);
    return 1;
}

static snd_pcm_format_t alsa_get_audio_format(wav_header_t *wav_header)
{
    switch (wav_header->bit_per_sample_rate)
    {
    case 8:
        return SND_PCM_FORMAT_U8;

    case 16:
        return SND_PCM_FORMAT_S16_LE;

    case 24:
        return SND_PCM_FORMAT_S24_LE;

    case 32:
        return SND_PCM_FORMAT_S32_LE;
    
    default:
        return SND_PCM_FORMAT_S16_LE;
    }
}

#define alsa_get_frame_size(wav_header) (((wav_header_t *) wav_header)->bit_per_sample_rate / 8) * (((wav_header_t *) wav_header)->channel) 

static int alsa_get_audio_info(char *audio_path, audio_info_t *audio_info)
{
    wav_header_t wav_header = {0, };
    wav_dummy_t wav_dummy = {0,};
    int i = 0;

    if((audio_info->fd = open(audio_path, O_RDONLY)) < 0)
    {
        printr("fail to open [%s] : %s", audio_path, strerror(errno));
        return -1;
    }

    if(read(audio_info->fd, &wav_header, sizeof(wav_header)) < 0)
    {
        printr("fail to read [%s] : %s", audio_path, strerror(errno));
        return -1;
    }

    audio_info->channel = wav_header.channel;
    audio_info->format = alsa_get_audio_format(&wav_header);
    audio_info->sample_rate = wav_header.sample_rate;
    audio_info->frame_size = alsa_get_frame_size(&wav_header);

    for(i = 0; i < 256; i++)
    {
        if(read(audio_info->fd, wav_dummy.sub_chunkx_id, sizeof(wav_dummy.sub_chunkx_id)) < 0)
        {
            printr("fail to read [%s] : %s", audio_path, strerror(errno));
            return -1;
        }

        if(read(audio_info->fd, &wav_dummy.sub_chunkx_size, sizeof(wav_dummy.sub_chunkx_size)) < 0)
        {
            printr("fail to read [%s] : %s", audio_path, strerror(errno));
            return -1;
        }

        if(strncmp(wav_dummy.sub_chunkx_id, "data", strlen("data")) == 0)
        {
            audio_info->data_size = wav_dummy.sub_chunkx_size;
            return 1;
        }

        if(lseek(audio_info->fd, wav_dummy.sub_chunkx_size, SEEK_CUR) < 0)
        {
            printr("fail to lseek [%s] : %s", audio_path, strerror(errno));
            return -1; 
        }
    }

    printr("read timeout");
    return -1;
}

static int alsa_ctl_stop_all(alsa_ctl_t *alsa_ctl)
{
    int ret = 0;
    if((ret = snd_pcm_drop(alsa_ctl->handle)) < 0)
    {
        printr("fail to drop : %s", snd_strerror(ret));
        return -1;
    }
    if((ret = snd_pcm_hw_free(alsa_ctl->handle)) < 0)
    {
        printr("fail to free hw : %s", snd_strerror(ret));
        return -1;
    }
    close(alsa_ctl->audio_info.fd);
    return 1;
}

    
int alsa_control_set(alsa_ctl_t *alsa_ctl, alsa_set_t *alsa_set)
{
    switch (alsa_set->control)
    {
    case ALSA_CONTROL_NOTHING:
        break;

    case ALSA_CONTROL_STOP:
        if(alsa_ctl->current_status != ALSA_STATUS_IDLE)
        {
            if(alsa_ctl_stop_all(alsa_ctl) < 0)
            {
                printr("alsa control stop fail");
                return -1;
            }
            alsa_ctl->current_status = ALSA_STATUS_IDLE;
            alsa_ctl->end_count = 0;
        }
        break;
        
    case ALSA_CONTROL_RESTART:
        if(alsa_ctl->current_status == ALSA_STATUS_PLAYING)
        {
            if(alsa_ctl_stop_all(alsa_ctl) < 0)
            {
                printr("alsa control stop fail");
                return -1;
            }
        }

        if(alsa_get_audio_info(alsa_ctl->current_audio_path, &alsa_ctl->audio_info) < 0)
        {
            printr("alsa get audio info fail");
            return -1;
        }

        if(alsa_set_hw_params(alsa_ctl) < 0)
        {
            printr("alsa set hw params");
            return -1;
        }
        alsa_ctl->current_status = ALSA_STATUS_PLAYING;
        alsa_ctl->end_count = 0;
        break;

    case ALSA_CONTROL_CHANGE_MUSIC:
        if(alsa_ctl->current_status == ALSA_STATUS_PLAYING)
        {
            if(alsa_ctl_stop_all(alsa_ctl) < 0)
            {
                printr("alsa control stop fail");
                return -1;
            }
        }

        strcpy(alsa_ctl->current_audio_path, alsa_set->play_audio_path);
        if(alsa_get_audio_info(alsa_ctl->current_audio_path, &alsa_ctl->audio_info) < 0)
        {
            printr("alsa get audio info fail");
            return -1;
        }

        if(alsa_set_hw_params(alsa_ctl) < 0)
        {
            printr("alsa set hw params");
            return -1;
        }
        alsa_ctl->current_status = ALSA_STATUS_PLAYING;
        alsa_ctl->end_count = 0;
        break;

        
    case ALSA_CONTROL_PAUSE:
        if(alsa_ctl->current_status == ALSA_STATUS_PLAYING)
        {
            alsa_ctl->current_status = ALSA_STATUS_PAUSE;
        }
        break;

    default:
        printr("unkwon control set : %d", alsa_set->control);
        return -1;
    }

    memset(alsa_set->play_audio_path, 0, sizeof(alsa_set->play_audio_path));
    alsa_set->control = ALSA_CONTROL_NOTHING;
    return 1;
}

#define ALSA_GET_WRITEI_SIZE(size) ALSA_WRITEI_BUF_MAX_SIZE/size
static int alsa_write_frame(alsa_ctl_t *alsa_ctl)
{
    int ret = 0;
    uint8_t writei_buf[ALSA_WRITEI_BUF_MAX_SIZE] = {0, };
    static uint32_t read_size = 0;

    if(alsa_ctl->current_status != ALSA_STATUS_PLAYING)
    {
        printr("invalid status");
        return -1;
    }

    if(read(alsa_ctl->audio_info.fd, writei_buf, sizeof(writei_buf)) < 0)
    {
        printr("fail to read : %s", snd_strerror(ret));
        return -1;
    }

    read_size += sizeof(writei_buf);

    if((ret = snd_pcm_writei(alsa_ctl->handle, writei_buf, ALSA_GET_WRITEI_SIZE(alsa_ctl->audio_info.frame_size))) < 0)
    {
        printr("fail to writei : %s", snd_strerror(ret));
        if(snd_pcm_recover(alsa_ctl->handle, ret, 1) < 0)
        {
            printr("fail to recover : %s", snd_strerror(ret));
            return -1;
        }
        else
        {
            printb("maybe pcm corrupted...");
            return 0;
        }
        return -1;
    }

    if(read_size > alsa_ctl->audio_info.data_size)
    {
        return 1;
    }
    return 0;
}

int alsa_control(alsa_ctl_t *alsa_ctl)
{
    int ret = 0;

    switch (alsa_ctl->current_status)
    {
    case ALSA_STATUS_IDLE:
        break;

    case ALSA_STATUS_PLAYING:
        ret = alsa_write_frame(alsa_ctl);
        if(ret < 0)
        {
            printr("fail to write frame");
            return -1;
        }
        else if (ret == 1)
        {
            alsa_ctl->current_status = ALSA_STATUS_END;
        }
        break;
        
    case ALSA_STATUS_PAUSE:
        break;

    case ALSA_STATUS_END:
        if(alsa_ctl->end_count++ >= 50)
        {
            if(alsa_ctl_stop_all(alsa_ctl) < 0)
            {
                printr("alsa control stop fail");
                return -1;
            }
            alsa_ctl->current_status = ALSA_STATUS_IDLE;
            alsa_ctl->end_count = 0;
        }
        break;
    
    default:
        printr("unkwon alsa control : %d", alsa_ctl->current_status);
        return -1;
    }
    
    return 1;
}