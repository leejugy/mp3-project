#ifndef __ALSA_CTL__
#define __ALSA_CTL__

#include "define.h"
#include <alsa/asoundlib.h>

#define ALSA_SOUND_CARD_NAME "hw:wm8960soundcard"
#define ALSA_WRITEI_BUF_MAX_SIZE 4096
#define ALSA_VOLUME_INIT_VAL 50
#define ALSA_GET_WRITEI_SIZE(size) ALSA_WRITEI_BUF_MAX_SIZE/size

typedef enum
{
    ALSA_STATUS_IDLE,
    ALSA_STATUS_PLAYING,
    ALSA_STATUS_PAUSE,
    ALSA_STATUS_END,
}ALSA_STATUS;

typedef enum
{
    ALSA_CONTROL_NOTHING,
    ALSA_CONTROL_STOP,
    ALSA_CONTROL_RESTART,
    ALSA_CONTROL_CHANGE_MUSIC, 
    ALSA_CONTROL_PAUSE,
    ALSA_CONTROL_RESUME,
    ALSA_CONTROL_VOLUME_CONTROL,
}ALSA_CONTROL;

typedef struct
{
    snd_pcm_format_t format;
    unsigned int channel;
    unsigned int sample_rate;
    int fd;
    uint32_t data_size;
    uint32_t frame_size;
}audio_info_t;

typedef struct
{
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *hw_params;
    audio_info_t audio_info;
    ALSA_STATUS current_status;
    int end_count;
    char current_audio_path[PATH_MAX_LEN];
    uint32_t read_size;
    uint8_t volume;
}alsa_ctl_t;

typedef struct 
{
    ALSA_CONTROL control;
    char play_audio_path[PATH_MAX_LEN];
    uint8_t volume;
}alsa_set_t;


typedef struct
{
    uint8_t riff_chunk[4];
    uint32_t riff_chunk_size;
    uint8_t riff_format[4];
    uint8_t sub_chunk1_id[4];
    uint32_t sub_chunk1_size;
    uint16_t audio_format;
    uint16_t channel;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bit_per_sample_rate;
}wav_header_t;

typedef struct
{
    uint8_t sub_chunkx_id[4];
    uint32_t sub_chunkx_size;
}wav_dummy_t;


int alsa_init(alsa_ctl_t *alsa_ctl);
int alsa_control_set(alsa_ctl_t *alsa_ctl, alsa_set_t *alsa_set);
int alsa_control(alsa_ctl_t *alsa_ctl);
#endif