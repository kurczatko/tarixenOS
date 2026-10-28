#ifndef ALC650_H
#define ALC650_H

#include "../include/printf/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int alc650_init(void);
void alc650_shutdown(void);
int alc650_play_pcm(const void *samples, uint32_t byte_count, uint32_t sample_rate_hz, int loop);
int alc650_playing(void);
void alc650_stop(void);
void alc650_set_master_volume(unsigned int percent);
void alc650_set_pcm_volume(unsigned int percent);
void alc650_set_mic_volume(unsigned int percent);
void alc650_set_cd_volume(unsigned int percent);
void alc650_mute_output(int mute);
void alc650_mute_input(int mute);
unsigned int alc650_get_master_volume(void);
unsigned int alc650_get_pcm_volume(void);
const char *alc650_codec_name(void);
uint32_t alc650_get_sample_rate(void);
void alc650_poll(void);

#ifdef __cplusplus
}
#endif

#endif
