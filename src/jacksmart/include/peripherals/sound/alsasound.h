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
#ifndef ALSASOUND_H
#define ALSASOUND_H
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Used for sound playback
#include <tinyalsa/asoundlib.h>

enum {
  ID_RIFF = 0x46464952,
  ID_WAVE = 0x45564157,
  ID_FMT  = 0x20746d66,
  ID_DATA = 0x61746164
};

/**
 * Note that the Alsa Sound settings are going to be device specific and
 * should only be used for testing.
 *
 * Functions copied straight from tinyplay.c, ported to C++ where necessary.
 * Note that this approach uses minimal Alsa playback.
 */
class AlsaSound {
 public:
  /**
   * Constructor for AlsaSound object.
   */
  AlsaSound();

  /**
   * Configures the mixer for basic line output using ALSA.
   *
   * Does the same as this:
   *   tinymix 'PRI_MI2S_RX Audio Mixer MultiMedia1' 1
   *   tinymix 'RX1 MIX1 INP1' 'RX1'
   *   tinymix 'RX2 MIX1 INP1' 'RX2'
   *   tinymix 'RDAC2 MUX' 'RX2'
   *   tinymix 'HPHL' 'Switch'
   *   tinymix 'HPHR' 'Switch'
   *   tinymix 'MI2S_RX Channels' 'Two'
   *   tinymix 'RX1 Digital Volume' 87
   *   tinymix_set_value(mixer, argv[1], &argv[2], argc - 2);
   */
  static void SetupMixer();

  /**
   * Plays file stored at the specified path.
   * Note that currently zero error handling / checking before playing.
   *
   * @param filename A string representing the path to the played file.
   */
  static void PlaySound(char* filename);

 /**
  * NOTE: The following functions are borrowed from tinyplay.c and tinymix.c.
  * TODO: OSS review.
  */
 private:
    struct riff_wave_header {
      uint32_t riff_id;
      uint32_t riff_size;
      uint32_t wave_id;
    };
    struct chunk_header {
      uint32_t id;
      uint32_t sz;
    };
    struct chunk_fmt {
      uint16_t audio_format;
      uint16_t num_channels;
      uint32_t sample_rate;
      uint32_t byte_rate;
      uint16_t block_align;
      uint16_t bits_per_sample;
    };

    static void stream_close(int sig);

    /**
     *  Helper for string processing taken from tinymix.c.
     */
    static int is_int(char *value);

    static int check_param(struct pcm_params *params, unsigned int param,
        unsigned int value, char *param_name, char *param_unit);

    /**
     *  Helper for string processing taken from tinymix.c.
     */
    static void tinymix_set_byte_ctl(struct mixer_ctl *ctl, char **values,
        unsigned int num_values);
    static int sample_is_playable(unsigned int card, unsigned int device,
        unsigned int channels, unsigned int rate, unsigned int bits,
        unsigned int period_size, unsigned int period_count);
    static void play_sample(FILE *file, unsigned int card, unsigned int device,
        unsigned int channels, unsigned int rate, unsigned int bits,
        unsigned int period_size, unsigned int period_count);
    static void tinymix_set_value(struct mixer *mixer, const char *control,
        char **values, unsigned int num_values);
    /**
     * End included code from tinymix.c
     */
};
#endif
