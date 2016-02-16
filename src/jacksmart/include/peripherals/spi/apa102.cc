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
#include "apa102.h"
#include <linux/spi/spidev.h>
#include <stdio.h>

APA102::APA102(int size) {
  size_ = size;
  pixelData_ = new uint8_t[(size + 2) * 4];

  for (int i = 0; i < 4; i++) {
    pixelData_[i] = 0x00;
  }

  for (int i = 0; i < 4; i++) {
    pixelData_[((size + 1) * 4) + i] = 0xFF;
  }

  // Setup SPI For APA102c
  spi_.SetMode(SPI_CS_HIGH | SPI_CPHA);
  spi_.SetSpeed(1000000);
  spi_.SetBits(32);
  spi_.SetDelay(0);
}


APA102::~APA102() {
  delete[] pixelData_;
}


void APA102::SetAll(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  // Clear all pixel data
  for (int i = 1; i <= size_; i++) {
    SetPixel(i, r, g, b, a);
  }
}


void APA102::Clear() {
  SetAll(0, 0, 0, 0);
}


void APA102::SetPixel(int pixelNumber, uint8_t r, uint8_t g, uint8_t b,
    uint8_t a) {
  // Don't set invalid pixels
  if (pixelNumber < 1 || pixelNumber > size_) {
    return;
  }

  int rootIndex = pixelNumber * 4;  // 4 bytes per pixel

  // Brightness is 5 bits of the "global" frame
  if (a > 32) {
    // Default brighness to 25%
    a = 8;
  }
  // 3 bits of "global" = 111, rest is alpha
  pixelData_[rootIndex] = a | 0xE0;

  switch (mode_) {
    case BGR:
      pixelData_[rootIndex + 1] = b;
      pixelData_[rootIndex + 2] = g;
      pixelData_[rootIndex + 3] = r;
      break;
    case BRG:
      pixelData_[rootIndex + 1] = b;
      pixelData_[rootIndex + 2] = r;
      pixelData_[rootIndex + 3] = g;
      break;
    case RGB:
      pixelData_[rootIndex + 1] = r;
      pixelData_[rootIndex + 2] = g;
      pixelData_[rootIndex + 3] = b;
      break;
    case RBG:
      pixelData_[rootIndex + 1] = r;
      pixelData_[rootIndex + 2] = b;
      pixelData_[rootIndex + 3] = g;
      break;
    case GBR:
      pixelData_[rootIndex + 1] = g;
      pixelData_[rootIndex + 2] = b;
      pixelData_[rootIndex + 3] = r;
      break;
    default:
      // TODO(class): Fail, unknown mode
      break;
  }
}


void APA102::Dump() {
  for (int i = 0; i < ((size_ + 1) * 4); i++) {
    printf("%d ", pixelData_[i]);
    if ((i + 1) % 4 == 0 && (i / 4 < size_)) {
      printf("\n [%d]: ", (i / 4) + 1);
    }
  }
  printf("\n");
}


void APA102::Display() {
  spi_.Init();  // TODO(class): should we be closing and opening the device?

  spi_.Write(pixelData_, (size_ + 2) * 4);

  spi_.Uninit();  // TODO(class): should we be closing and opening the device?
}
