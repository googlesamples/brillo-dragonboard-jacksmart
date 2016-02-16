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
#ifndef SOUND_H
#define SOUND_H
#include <media/stagefright/AudioPlayer.h>
#include <string>

using namespace android;

class Sound {
 public:
  // TODO(class): Implement these as interface.
  /**
   * Initializes the player stored in the object.
   */
  Sound();

  /**
   * Cleans up the player resources after waiting for the current sound to
   * finish playing.
   */
  ~Sound();

  /**
   * Uses the ALSA mixer to change the current sound configuration.
   * Note that this should only be used for testing.
   */
  void SetupMixer();

  /**
   * Detects the sound type using the file extension and plays it using
   * stagefright.
   *
   * @param filePath The path to the WAV or MP3 sound file you want to play.
   *
   * Note that this could segfault if the file is of the incorrect type.
   */
  status_t PlaySound(std::string filePath);


 private:
  AudioPlayer* player_;
  android::status_t status_ = android::UNKNOWN_ERROR;

  /**
   * Plays MP3 using Stagefright.
   *
   * @param filename Path to the MP3 file to play.
   */
  status_t PlayMp3Sound(const char* filename);

  /**
   * Plays WAV file using Stagefright.
   *
   * @param filename The path of the WAV file to play.
   */
  status_t PlayWavSound(const char* filename);
};
#endif
