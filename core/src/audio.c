#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "audio.h"
#include "core.h"

#include "util/miniaudio.h"

#define sample_format ma_format_f32
#define channel_count 2
#define sample_rate   48000
#define buffer_size   4096
#define buffer_cap    buffer_size / channel_count

#define max_clips 32

static void data_callback(ma_device* device, void* out, const void* in, u32 frame_count);

struct audio {
	ma_device device;
	ma_device_config device_config;

	struct audio_clip* clips[max_clips];
	u32 clip_count;

	f32* buffer;
} audio;

struct audio_clip {
	ma_decoder decoder;
	ma_decoder_config decoder_config;

	u8* data;
	u64 data_size;

	bool playing;
	bool loop;

	u32 id;

	f32 volume;
};

void audio_init() {
	memset(&audio, 0, sizeof(struct audio));

	audio.device_config = ma_device_config_init(ma_device_type_playback);
	audio.device_config.playback.format = sample_format;
	audio.device_config.playback.channels = channel_count;
	audio.device_config.sampleRate = sample_rate;
	audio.device_config.dataCallback = data_callback;
	audio.device_config.pUserData = null;

	ma_device_init(null, &audio.device_config, &audio.device);
	ma_device_start(&audio.device);

	audio.buffer = core_alloc(4096 * sizeof(f32));
}

void audio_deinit() {
	ma_device_stop(&audio.device);

	for (u32 i = 0; i < audio.clip_count; i++) {
		stop_audio_clip(audio.clips[i]);
	}

	audio.clip_count = 0;

	ma_device_uninit(&audio.device);

	core_free(audio.buffer);
}

void audio_update() {
	for (u32 i = 0; i < audio.clip_count; i++) {
		struct audio_clip* clip = audio.clips[i];

		if (!clip->playing) {
			stop_audio_clip(clip);
			if (clip->loop) {
				play_audio_clip(clip);
			}
		}
	}
}

static u32 read_and_mix(ma_decoder* decoder, f32* out, u32 frame_count) {
	u32 read = 0;

	while (read < frame_count) {
		u32 remaining = frame_count - read;
		u32 to_read = buffer_cap;

		if (to_read > remaining) {
			to_read = remaining;
		}

		u64 frames_read;
		ma_result r = ma_decoder_read_pcm_frames(decoder, audio.buffer, to_read, (ma_uint64*)&frames_read);
		if (r != MA_SUCCESS || frames_read == 0) {
			break;
		}

		for (u32 i = 0; i < frames_read * channel_count; i++) {
			out[read * channel_count + i] += audio.buffer[i] * ((struct audio_clip*)decoder->pUserData)->volume;
		}

		read += frames_read;

		if (frames_read < to_read) {
			break;
		}
	}

	return read;
}

static void data_callback(ma_device* device, void* out, const void* in, u32 frame_count) {
	assert(device->playback.format == sample_format);

	for (u32 i = 0; i < audio.clip_count; i++) {
		struct audio_clip* clip = audio.clips[i];

		if (clip->playing) {
			u32 read = read_and_mix(&clip->decoder, (f32*)out, frame_count);
			if (read < frame_count) {
				clip->playing = false;
			}
		}
	}

	(void)in;
}

struct audio_clip* new_audio_clip(u8* data, u64 size) {
	assert(audio.clip_count < max_clips && "Too many audio clips.");

	struct audio_clip* clip = core_calloc(1, sizeof(struct audio_clip));

	clip->data = data;
	clip->data_size = size;

	clip->volume = 1.0f;

	clip->decoder_config = ma_decoder_config_init(sample_format, channel_count, sample_rate);
	ma_result r = ma_decoder_init_memory(clip->data, clip->data_size, &clip->decoder_config, &clip->decoder);
	if (r != MA_SUCCESS) {
		fprintf(stderr, "Failed to create audio clip.\n");
		ma_decoder_uninit(&clip->decoder);
		core_free(clip);

		return null;
	}

	clip->decoder.pUserData = clip;

	return clip;
}

void free_audio_clip(struct audio_clip* clip) {
	if (clip->playing) {
		stop_audio_clip(clip);
	}

	ma_decoder_uninit(&clip->decoder);

	core_free(clip->data);
	core_free(clip);
}

void play_audio_clip(struct audio_clip* clip) {
	for (u32 i = 0; i < audio.clip_count; i++) {
		if (audio.clips[i] == clip) {
			stop_audio_clip(audio.clips[i]);
			break;
		}
	}

	ma_decoder_seek_to_pcm_frame(&clip->decoder, 0);

	clip->playing = true;
	audio.clips[audio.clip_count] = clip;
	clip->id = audio.clip_count++;
}

void stop_audio_clip(struct audio_clip* clip) {
	clip->playing = false;

	audio.clips[clip->id]     = audio.clips[audio.clip_count - 1];
	audio.clips[clip->id]->id = clip->id;
	audio.clip_count--;
}

void loop_audio_clip(struct audio_clip* clip, bool loop) {
	clip->loop = loop;
}

void set_audio_clip_volume(struct audio_clip* clip, f32 volume) {
	clip->volume = volume / 100.0f;
}
