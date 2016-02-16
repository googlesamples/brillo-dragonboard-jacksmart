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
#ifndef BRILLO_SPI_H
#define BRILLO_SPI_H
#include <stdint.h>

class SPI {
 private:
  uint8_t mode_ = 0;
  uint8_t bits_ = 32;
  uint32_t speed_ = 1000000;
  uint16_t delay_ = 0;

  int spifd_ = -1;
  const char *device_ = "/dev/spidev1.0";


 public:
  /**
   * Sets bits per word.
   *
   * @param bpw The number of bits transmitted per word over SPI.
   */
  void SetBits(uint8_t bpw) { bits_ = bpw; }

  /**
   * Sets the speed (in hz) for SPI transfers.
   *
   * @param speed The speed (in hz);
   */
  void SetSpeed(uint32_t speed) { speed_ = speed; }

  /**
   * Sets the delay for writing to SPI.
   *
   * @param delay The amount to delay (in milliseconds).
   */
  void SetDelay(uint16_t delay) { delay_ = delay; }

  /**
   * Sets the mode for writing to SPI.
   *
   * @param mode The mode for writing to SPI.
   *
   * Note:
   *   The mode consists of a combination of the following flags:
   *   - mode |= SPI_LOOP
   *   - mode |= SPI_CPHA
   *   - mode |= SPI_SPOL
   *   - mode |= SPI_LSB_FIRST
   *   - mode |= SPI_CS_HIGH
   *   - mode |= SPI_3WIRE
   *   - mode |= SPI_NO_CS
   *   - mode |= SPI_SPI_READY
   */
  void SetMode(uint8_t mode) { mode_ = mode; }

  /**
   * Sets up the SPI device.
   */
  SPI();

  /**
   * Opens the device for writing.
   */
  bool Init();

  /**
   * Writes to the SPI device.
   *
   * @param tx Array of data to write.
   * @param size Size of tx data array.
   */
  int Write(const uint8_t tx[], int size);

  /**
   * Closes the device.
   */
  void Uninit();
};
#endif
