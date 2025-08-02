#include "alsa_ctl.h"
#include "gpio_ctl.h"
#include "sqlite3_ctl.h"

int alsa_init(alsa_ctl_t *alsa_ctl)
{   
    int ret = 0;

    if((ret = snd_pcm_open(&alsa_ctl->handle, ALSA_SOUND_CARD_NAME, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        printr("fail to open pcm : %s", snd_strerror(ret));
        return -1;
    }

    alsa_ctl->volume = sql_get_settings_integer(SQL_SETTINGS_VOLUME_NAME);
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
        close(alsa_ctl->audio_info.fd);
        return -1;
    }
    if((ret = snd_pcm_hw_free(alsa_ctl->handle)) < 0)
    {
        printr("fail to free hw : %s", snd_strerror(ret));
        close(alsa_ctl->audio_info.fd);
        return -1;
    }
    close(alsa_ctl->audio_info.fd);
    return 1;
}

    
int alsa_control_set(alsa_ctl_t *alsa_ctl, alsa_set_t *alsa_set)
{
    int val = 0;
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
            alsa_ctl->read_size = 0;
        }
        val = GPIO_LOW;
        gpio_write_index(GPIO_CODEC_SW_MUTE, &val);
        break;
        
    case ALSA_CONTROL_RESTART:
        if(alsa_ctl->current_status != ALSA_STATUS_IDLE)
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
        val = GPIO_HIGH;
        gpio_write_index(GPIO_CODEC_SW_MUTE, &val);
        alsa_ctl->current_status = ALSA_STATUS_PLAYING;
        alsa_ctl->end_count = 0;
        alsa_ctl->read_size = 0;
        break;

    case ALSA_CONTROL_CHANGE_MUSIC:
        if(alsa_ctl->current_status != ALSA_STATUS_IDLE)
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
        val = GPIO_HIGH;
        gpio_write_index(GPIO_CODEC_SW_MUTE, &val);
        alsa_ctl->current_status = ALSA_STATUS_PLAYING;
        alsa_ctl->end_count = 0;
        alsa_ctl->read_size = 0;
        break;

        
    case ALSA_CONTROL_PAUSE:
        if(alsa_ctl->current_status == ALSA_STATUS_PLAYING)
        {
            val = GPIO_LOW;
            gpio_write_index(GPIO_CODEC_SW_MUTE, &val);
            alsa_ctl->current_status = ALSA_STATUS_PAUSE;
        }
        break;
    
    case ALSA_CONTROL_RESUME:
        if(alsa_ctl->current_status == ALSA_STATUS_PAUSE)
        {
            val = GPIO_HIGH;
            gpio_write_index(GPIO_CODEC_SW_MUTE, &val);
            alsa_ctl->current_status = ALSA_STATUS_PLAYING;
        }
        break;

    case ALSA_CONTROL_VOLUME_CONTROL:
        alsa_ctl->volume = alsa_set->volume;
        sql_set_settings_integer(SQL_SETTINGS_VOLUME_NAME, alsa_ctl->volume);
        break;

    default:
        val = GPIO_LOW;
        gpio_write_index(GPIO_CODEC_SW_MUTE, &val);
        printr("unkwon control set : %d", alsa_set->control);
        return -1;
    }

    memset(alsa_set->play_audio_path, 0, sizeof(alsa_set->play_audio_path));
    alsa_set->volume = 0;
    alsa_set->control = ALSA_CONTROL_NOTHING;
    return 1;
}



static void alsa_volume_control(alsa_ctl_t *alsa_ctl, void *buf, int buf_size)
{
    int i = 0;
    int j = 0;
    switch (alsa_ctl->audio_info.format)
    {
    case SND_PCM_FORMAT_U8:
    {
        uint8_t *u8_data = buf;
        for(i = 0; i < buf_size; i += sizeof(uint8_t))
        {
            u8_data[j] = u8_data[j] * ((float)alsa_ctl->volume / 100.0);
            j++;
        }
        break;
    }

    case SND_PCM_FORMAT_S16_LE:
    {
        int16_t *s16_data = buf;
        for(i = 0; i < buf_size; i += sizeof(int16_t))
        {
            s16_data[j] = s16_data[j] * ((float)alsa_ctl->volume / 100.0);
            j++;
        }
        break;
    }

    case SND_PCM_FORMAT_S24_LE:
    {
        uint8_t s24_buf[3] = {0, };
        int32_t s24_data = 0;
        for(i = 0; i < buf_size; i += sizeof(s24_buf))
        {
            s24_data = 0;
            memcpy(s24_buf, buf + i, sizeof(s24_buf));
            /* signed extension */
            s24_data |= s24_buf[2] << 24;
            s24_data |= s24_buf[1] << 16;
            s24_data |= s24_buf[0] << 8;
            s24_data = s24_data >> 8;

            s24_data = s24_data * ((float)alsa_ctl->volume / 100.0);

            s24_buf[0] = (s24_data >> 16) & 0xff;
            s24_buf[1] = (s24_data >> 8) & 0xff;
            s24_buf[2] = (s24_data) & 0xff;
            memcpy(buf + i, s24_buf, sizeof(s24_buf));
        }
        break;
    }

    case SND_PCM_FORMAT_S32_LE:
    {
        int32_t *s32_data = buf;
        for(i = 0; i < buf_size; i += sizeof(int32_t))
        {
            s32_data[j] = s32_data[j] * ((float)alsa_ctl->volume / 100.0);
            j++;
        }
        break;
    }

    default:
    {
        int16_t *s16_data = buf;
        for(i = 0; i < buf_size; i += sizeof(int16_t))
        {
            s16_data[j] = s16_data[j] * ((float)alsa_ctl->volume / 100.0);
            j++;
        }
        break;
    }
    }
}

static int alsa_write_frame(alsa_ctl_t *alsa_ctl)
{
    int ret = 0;
    uint8_t writei_buf[ALSA_WRITEI_BUF_MAX_SIZE] = {0, };

    if(alsa_ctl->current_status != ALSA_STATUS_PLAYING)
    {
        printr("invalid status");
        return -1;
    }

    if((ret = read(alsa_ctl->audio_info.fd, writei_buf, sizeof(writei_buf))) < 0)
    {
        printr("fail to read : %s", strerror(errno));
        return -1;
    }

    alsa_ctl->read_size += sizeof(writei_buf);
    alsa_volume_control(alsa_ctl, writei_buf, ret);

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

    if(alsa_ctl->read_size > alsa_ctl->audio_info.data_size)
    {
        alsa_ctl->read_size = 0;
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
        /* wait 500ms to finish codec playing */
        if(alsa_ctl->end_count++ >= 500)
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