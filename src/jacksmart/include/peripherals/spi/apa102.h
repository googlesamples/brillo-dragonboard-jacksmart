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
#ifndef BRILLO_APA102_H
#define BRILLO_APA102_H
#include "spi.h"

/**
 * This class encapsulates access to APA102 (aka Dotstar) LED pixels.
 * Lots of information on the LED package can be found on Adafruit here:
 *    https://learn.adafruit.com/adafruit-dotstar-leds/overview
 * as well as here:
 *    https://cpldcpu.wordpress.com/2014/08/27/apa102/
 */
class APA102 {
 public:
  enum Mode { GRB, GBR, RGB, RBG, BGR, BRG };

 private:
  int size_;
  uint8_t* pixelData_;
  SPI spi_;
  Mode mode_ = BRG;

 public:
  /**
   * Constructs an APA102 strip with **size** pixels.
   *
   * @param size The number of pixels in the strip.
   */
  explicit APA102(int size);

  ~APA102();

  /**
   * Sets a pixel to the given RGBA value.
   *
   * @param pixelNumber The pixel offest (starting at 1) to set.
   * @param r The red value.
   * @param g The green value.
   * @param b The blue value.
   * @param a The alpha value.
   */
  void SetPixel(int pixelNumber, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

  /**
   * Sets all pixels to a given color.
   *
   * @param r The red value.
   * @param g The green value.
   * @param b The blue value.
   * @param a The alpha value.
   */
  void SetAll(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

  /**
   * Clears all pixel data.
   */
  void Clear();

  /**
   * Sets the pixel data mode (e.g. Red, Green, Blue).
   *
   * @param mode The mode setting for the APA102 pixels.
   */
  void SetMode(Mode mode) { mode_ = mode; }

  /**
   * Displays the current pixel buffer by writing to SPI.
   */
  void Display();

  /**
   * Dumps the internal pixel data to stdout.
   */
  void Dump();
};
#endif  // BRILLO_APA102_H
