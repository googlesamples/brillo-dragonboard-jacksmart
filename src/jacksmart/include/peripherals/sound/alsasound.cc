/*
 * Copyright 2016 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "alsasound.h"
#include <errno.h>
#include <ctype.h>

// TODO(class): Remove and figure out another way to signal device playback.
int _alsaclose = 0;

AlsaSound::AlsaSound() {
  _alsaclose;
}


void AlsaSound::SetupMixer() {
  struct mixer *mixer;
  int card = 0;
  mixer = mixer_open(card);
  if (!mixer) {
    fprintf(stderr, "Failed to open mixer\n");
    // TODO(class): return error
    return;
  }

  char *values[1];
  char temp[64];
  values[0] = temp;

  // Turns on the mixer
  snprintf(temp, sizeof(temp), "1");
  tinymix_set_value(mixer, "PRI_MI2S_RX Audio Mixer MultiMedia1", values, 1);
  snprintf(temp, sizeof(temp), "RX1");
  tinymix_set_value(mixer, "RX1 MIX1 INP1", values, 1);
  snprintf(temp, sizeof(temp), "RX2");
  tinymix_set_value(mixer, "RX2 MIX1 INP1", values, 1);
  tinymix_set_value(mixer, "RDAC2 MUX", values, 1);

  // Turn on headphones (Left + Right)
  snprintf(temp, sizeof(temp), "Switch");
  tinymix_set_value(mixer, "HPHL", values, 1);
  tinymix_set_value(mixer, "HPHR", values, 1);

  snprintf(temp, sizeof(temp), "Two");
  tinymix_set_value(mixer, "MI2S_RX Channels", values, 1);

  // Set the volume
  snprintf(temp, sizeof(temp), "87");
  tinymix_set_value(mixer, "RX1 Digital Volume", values, 1);
  mixer_close(mixer);
}


void AlsaSound::PlaySound(char* filename) {
  FILE *file;
  struct riff_wave_header riff_wave_header;
  struct chunk_header chunk_header;
  struct chunk_fmt chunk_fmt;
  unsigned int device = 0;
  unsigned int card = 0;
  unsigned int period_size = 1024;
  unsigned int period_count = 4;
  int more_chunks = 1;

  fprintf(stderr, "opening the file\n");
  file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Unable to open file '%s'\n", filename);
    // TODO(class): return error
    return;
  }
  fprintf(stderr, "reading the chunks\n");

  fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);
  if ((riff_wave_header.riff_id != ID_RIFF) ||
      (riff_wave_header.wave_id != ID_WAVE)) {
    fprintf(stderr, "Error: '%s' is not a riff/wave file\n", filename);
    fclose(file);
    // TODO(class): return error
    return;
  }

  // Read to sample data...
  do {
    fread(&chunk_header, sizeof(chunk_header), 1, file);
    switch (chunk_header.id) {
      case ID_FMT:
        fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
        /* If the format header is larger, skip the rest */
        if (chunk_header.sz > sizeof(chunk_fmt))
          fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
        break;
      case ID_DATA:
        /* Stop looking for chunks */
        more_chunks = 0;
        break;
      default:
        /* Unknown chunk, skip bytes */
        fseek(file, chunk_header.sz, SEEK_CUR);
    }
  } while (more_chunks);

  fprintf(stderr, "Playing the sample\n");
  play_sample(
      file, card, device, chunk_fmt.num_channels, chunk_fmt.sample_rate,
      chunk_fmt.bits_per_sample, period_size, period_count);
  fprintf(stderr, "Closing the file\n");
  fclose(file);
}


void AlsaSound::stream_close(int sig) {
  /* allow the stream to be closed gracefully */
  signal(sig, SIG_IGN);
  _alsaclose = 1;
}


int AlsaSound::check_param(
    struct pcm_params *params, unsigned int param, unsigned int value,
    char *param_name, char *param_unit) {
  unsigned int min;
  unsigned int max;
  int is_within_bounds = 1;

  min = pcm_params_get_min(params, (pcm_param)param);
  if (value < min) {
      fprintf(stderr, "%s is %u%s, device only supports >= %u%s\n",
          param_name, value, param_unit, min, param_unit);
      is_within_bounds = 0;
  }

  max = pcm_params_get_max(params, (pcm_param)param);
  if (value > max) {
      fprintf(stderr, "%s is %u%s, device only supports <= %u%s\n", param_name,
          value, param_unit, max, param_unit);
      is_within_bounds = 0;
  }

  return is_within_bounds;
}


int AlsaSound::sample_is_playable(
    unsigned int card, unsigned int device, unsigned int channels,
    unsigned int rate, unsigned int bits, unsigned int period_size,
    unsigned int period_count) {
  struct pcm_params *params;
  int can_play;

  params = pcm_params_get(card, device, PCM_OUT);
  if (params == NULL) {
      fprintf(stderr, "Unable to open PCM device %u.\n", device);
      return 0;
  }

  can_play = check_param(
      params, PCM_PARAM_RATE, rate, const_cast<char*>("Sample rate"),
      const_cast<char*>("Hz"));
  can_play &= check_param(
      params, PCM_PARAM_CHANNELS, channels, const_cast<char*>("Sample"),
      const_cast<char*>(" channels"));
  can_play &= check_param(
      params, PCM_PARAM_SAMPLE_BITS, bits, const_cast<char*>("Bitrate"),
      const_cast<char*>(" bits"));
  can_play &= check_param(
      params, PCM_PARAM_PERIOD_SIZE, period_size,
      const_cast<char*>("Period size"), const_cast<char*>("Hz"));
  can_play &= check_param(
      params, PCM_PARAM_PERIODS, period_count,
      const_cast<char*>("Period count"), const_cast<char*>("Hz"));

  pcm_params_free(params);

  return can_play;
}


void AlsaSound::play_sample(
    FILE *file, unsigned int card, unsigned int device, unsigned int channels,
    unsigned int rate, unsigned int bits, unsigned int period_size,
    unsigned int period_count) {
  struct pcm_config config;
  struct pcm *pcm;
  char *buffer;
  int size;
  int num_read;

  memset(&config, 0, sizeof(config));
  config.channels = channels;
  config.rate = rate;
  config.period_size = period_size;
  config.period_count = period_count;

  if (bits == 32)
    config.format = PCM_FORMAT_S32_LE;
  else if (bits == 16)
    config.format = PCM_FORMAT_S16_LE;

  config.start_threshold = 0;
  config.stop_threshold = 0;
  config.silence_threshold = 0;

  if (!sample_is_playable(
      card, device, channels, rate, bits, period_size, period_count)) {
    return;
  }
  fprintf(stderr, "Sample playable, going ahead.\n");

  pcm = pcm_open(card, device, PCM_OUT, &config);
  if (!pcm || !pcm_is_ready(pcm)) {
    fprintf(stderr, "Unable to open PCM device %u (%s)\n", device,
        pcm_get_error(pcm));
    return;
  }
  fprintf(stderr, "PCM openable, going ahead.\n");

  size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
  buffer = reinterpret_cast<char*>(malloc(size));
  if (!buffer) {
    fprintf(stderr, "Unable to allocate %d bytes\n", size);
    free(buffer);
    pcm_close(pcm);
    return;
  }
  fprintf(stderr, "Bytes allocated, going ahead.\n");

  /* catch ctrl-c to shutdown cleanly */
  signal(SIGINT, stream_close);

  do {
    num_read = fread(buffer, 1, size, file);
    if (num_read > 0) {
      if (pcm_write(pcm, buffer, num_read)) {
        fprintf(stderr, "Error playing sample\n");
        break;
      }
    }
  } while (!_alsaclose && num_read > 0);

  free(buffer);
  pcm_close(pcm);
}


int AlsaSound::is_int(char *value) {
    char* end;
    int64_t result;

    errno = 0;
    result = strtol(value, &end, 10);

    if (result == LONG_MIN || result == LONG_MAX) {
      return 0;
    }

    return errno == 0 && *end == '\0';
}


void AlsaSound::tinymix_set_byte_ctl(
    struct mixer_ctl *ctl, char **values, unsigned int num_values) {
  int ret;
  char *buf;
  char *end;
  unsigned int i;
  int64_t n;

  buf = reinterpret_cast<char*>(calloc(1, num_values));
  if (buf == NULL) {
    fprintf(stderr, "set_byte_ctl: Failed to alloc mem for bytes %d\n",
        num_values);
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < num_values; i++) {
    errno = 0;
    n = strtol(values[i], &end, 0);
    if (*end) {
      fprintf(stderr, "%s not an integer\n", values[i]);
      goto fail;
    }
    if (errno) {
      fprintf(stderr, "strtol: %s: %s\n", values[i], strerror(errno));
      goto fail;
    }
    if (n < 0 || n > 0xff) {
      fprintf(stderr, "%s should be between [0, 0xff]\n", values[i]);
      goto fail;
    }
    buf[i] = n;
  }

  ret = mixer_ctl_set_array(ctl, buf, num_values);
  if (ret < 0) {
    fprintf(stderr, "Failed to set binary control\n");
    goto fail;
  }

  free(buf);
  return;

fail:
  free(buf);
  exit(EXIT_FAILURE);
}


void AlsaSound::tinymix_set_value(
    struct mixer *mixer, const char *control, char **values,
    unsigned int num_values) {
  struct mixer_ctl *ctl;
  enum mixer_ctl_type type;
  unsigned int num_ctl_values;
  unsigned int i;

  if (isdigit(control[0]))
    ctl = mixer_get_ctl(mixer, atoi(control));
  else
    ctl = mixer_get_ctl_by_name(mixer, control);

  if (!ctl) {
    fprintf(stderr, "Invalid mixer control\n");
    return;
  }

  type = mixer_ctl_get_type(ctl);
  num_ctl_values = mixer_ctl_get_num_values(ctl);

  if (type == MIXER_CTL_TYPE_BYTE) {
    tinymix_set_byte_ctl(ctl, values, num_values);
    return;
  }

  if (is_int(values[0])) {
    if (num_values == 1) {
      /* Set all values the same */
      int value = atoi(values[0]);

      for (i = 0; i < num_ctl_values; i++) {
        if (mixer_ctl_set_value(ctl, i, value)) {
          fprintf(stderr, "Error: invalid value\n");
          return;
        }
      }
    } else {
      /* Set multiple values */
      if (num_values > num_ctl_values) {
        fprintf(stderr, "Error: %d values given, but control only takes %d\n",
            num_values, num_ctl_values);
        return;
      }
      for (i = 0; i < num_values; i++) {
        if (mixer_ctl_set_value(ctl, i, atoi(values[i]))) {
          fprintf(stderr, "Error: invalid value for index %d\n", i);
          return;
        }
      }
    }
  } else {
    if (type == MIXER_CTL_TYPE_ENUM) {
      if (num_values != 1) {
        fprintf(stderr, "Enclose strings in quotes and try again\n");
        return;
      }
      if (mixer_ctl_set_enum_by_string(ctl, values[0])) {
        fprintf(stderr, "Error: invalid enum value\n");
      }
    } else {
      fprintf(stderr, "Error: only enum types can be set with strings\n");
    }
  }
}


