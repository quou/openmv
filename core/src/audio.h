#pragma once

#include "common.h"

struct audio_clip;

API void audio_init();
API void audio_deinit();
API void audio_update();
API struct audio_clip* new_audio_clip(u8* data, u64 size);
API void free_audio_clip(struct audio_clip* clip);
API void play_audio_clip(struct audio_clip* clip);
API void stop_audio_clip(struct audio_clip* clip);
API void loop_audio_clip(struct audio_clip* clip, bool loop);

API void set_audio_clip_volume(struct audio_clip* clip, float volume);
