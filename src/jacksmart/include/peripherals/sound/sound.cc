/*
 * Copyright 2016 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <base/logging.h>
#include <media/stagefright/AudioPlayer.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/OMXClient.h>
#include <media/stagefright/SimpleDecodingSource.h>
#include <media/stagefright/ACodec.h>
#include <media/stagefright/foundation/AMessage.h>

#include "alsasound.h"
#include "include/MP3Extractor.h"
#include "include/WAVExtractor.h"

Sound::Sound() {
  player_ = new AudioPlayer(NULL);
}


Sound::~Sound() {
  // Wait for sound to finish playing
  while (!player_->reachedEOS(&status_)) {
    usleep(100000);
  }
  delete(player_);
}

void Sound::SetupMixer() {
  AlsaSound::SetupMixer();
}


status_t Sound::PlaySound(std::string filePath) {
  if (filePath.compare(filePath.length() - 3, 3, "mp3") == 0) {
    return this->PlayMp3Sound(filePath.c_str());
  }

  if (filePath.compare(filePath.length() - 3, 3, "wav") == 0) {
    return this->PlayWavSound(filePath.c_str());
  }

  return android::UNKNOWN_ERROR;
}


status_t Sound::PlayWavSound(const char* filename) {
  if (filename == NULL) {
    return -1;
  }

  OMXClient client;
  status_t status = client.connect();
  if (status != OK) {
    LOG(ERROR) << "Could not connect OMX client";
    return status;
  }

  FileSource* file_source = new FileSource(filename);
  status = file_source->initCheck();
  if (status != OK) {
    LOG(ERROR) << "Could not open the WAV file source.";
    return status;
  }

  // Extract track.
  sp<MediaExtractor> media_extractor = new WAVExtractor(file_source);
  sp<MediaSource> media_source = media_extractor->getTrack(0);

  // Decode wav.
  sp<MediaSource> decoded_source = SimpleDecodingSource::Create(media_source);

  // Play wav.
  player_->setSource(decoded_source);
  status = player_->start();
  if (status != OK) {
    LOG(ERROR) << "Could not start playing audio.";
    return status;
  }

  return status;
}


status_t Sound::PlayMp3Sound(const char* filename) {
  if (filename == NULL) {
    return -1;
  }
  OMXClient client;
  status_t status = client.connect();
  if (status != OK) {
    LOG(ERROR) << "Could not connect OMX client";
    return status;
  }

  FileSource* file_source = new FileSource(filename);
  status = file_source->initCheck();
  if (status != OK) {
    LOG(ERROR) << "Could not open the mp3 file source.";
    return status;
  }

  // Extract track.
  sp<AMessage> message = new AMessage();
  sp<MediaExtractor> media_extractor = new MP3Extractor(file_source, message);
  sp<MediaSource> media_source = media_extractor->getTrack(0);

  // Decode mp3.
  sp<MediaSource> decoded_source = SimpleDecodingSource::Create(media_source);

  // Play mp3.
  player_->setSource(decoded_source);
  status = player_->start();
  if (status != OK) {
    LOG(ERROR) << "Could not start playing audio.";
    return status;
  }
  return status;
}
