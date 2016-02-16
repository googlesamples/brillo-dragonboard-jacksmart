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
#include <linux/spi/spidev.h>

#include <string>

#include "include/peripherals/gpio/gpio.h"
#include "include/peripherals/sound/alsasound.h"
#include "include/peripherals/sound/sound.h"
#include "include/peripherals/spi/spi.h"
#include "include/peripherals/spi/apa102.h"

void TestAlsaSound();
void TestAPA102();
void TestGPIO();
void TestSPI();
void TestSound(std::string);


/**
 * Test application code.
 */
int main(int argc, char** argv) {
  if (argc <= 1) {
    printf("Usage: jacksmart-test test1 [... testN]\n");
    printf(" Options:\n");
    printf("  g - test GPIO\n");
    printf("  s - test Sound\n");
    printf("  S - test SPI\n");
    printf("  a - test APA102\n");
    printf("  A - test Alsa Sound\n");
    printf(" Examples: \n");
    printf("  jacksmart-test a \n");
    printf("  jacksmart-test g \n");
    printf("  jacksmart-test s a s a \n");
  }

  for (int i=1; i < argc; i++) {
    char mode = argv[i][0];

    // Run the tests as issued
    if (mode == 'g')
      TestGPIO();
    if (mode == 's') {
      if (argc > 2) {
        TestSound(argv[2]);
      } else {
        TestSound("/data/2553.mp3");
        TestSound("/data/boot.mp3");
      }
    }
    if (mode == 'S')
      TestSPI();
    if (mode == 'a')
      TestAPA102();
    if (mode == 'A')
      TestAlsaSound();
  }
}


/**
 * Tests MP3 sound playback via Stagefright library.
 *
 * @param filePath The path to the file to play.
 */
void TestSound(std::string filePath) {
  Sound sound;
  sound.SetupMixer();

  sound.PlaySound(filePath);
}


/**
 * Tests sound configuration and playback with Alsa library.
 */
void TestAlsaSound() {
  AlsaSound sound;

  fprintf(stderr, "Setting up mixer");
  sound.SetupMixer();

  fprintf(stderr, "Starting playback");
  sound.PlaySound(const_cast<char*>("/data/out.wav"));
}


/**
 * Tests GPIO functions.
 */
void TestGPIO() {
  SetupGPIO();
  bool isOn = true;

  fprintf(stderr, "Setting GPIO\n");
  for (int i=0; i < 10; i++) {
    WriteGPIO(GPIO::PIN_A, isOn);
    WriteGPIO(GPIO::PIN_B, isOn);
    WriteGPIO(GPIO::PIN_C, isOn);
    WriteGPIO(GPIO::PIN_D, isOn);
    usleep(1000000);
    isOn = !isOn;
  }
}


/**
 * Drives APA102's with the wrapper.
 */
void TestAPA102() {
  APA102 apa(6);

  int sleepMmsecs = 100000;

  // Taste the rainbow.
  apa.SetMode(APA102::Mode::BGR);

  apa.SetPixel(1, 0xFF, 0x00, 0x00, 4);  // Red
  apa.SetPixel(2, 0xFF, 0x8C, 0x00, 4);  // Orange
  apa.SetPixel(3, 0xFF, 0xFF, 0x00, 4);  // Yellow
  apa.SetPixel(4, 0x00, 0xFF, 0x00, 4);  // Green
  apa.SetPixel(5, 0x00, 0x00, 0xFF, 4);  // Blue
  apa.SetPixel(6, 0xD4, 0x00, 0xD3, 4);  // Violet
  apa.Display();

  usleep(sleepMmsecs);

  for (int i=0; i < 6; i++) {
    apa.SetPixel(i + 1, 0x00, 0x00, 0x00, 0);
    apa.Display();
    usleep(sleepMmsecs);
  }

  apa.SetPixel(6, 0xD4, 0x00, 0xD3, 4);  // Violet
  apa.Display();
  usleep(sleepMmsecs);

  apa.SetPixel(5, 0x00, 0x00, 0xFF, 4);  // Blue
  apa.Display();
  usleep(sleepMmsecs);

  apa.SetPixel(4, 0x00, 0xFF, 0x00, 4);  // Green
  apa.Display();
  usleep(sleepMmsecs);

  apa.SetPixel(3, 0xFF, 0xFF, 0x00, 4);  // Yellow
  apa.Display();
  usleep(sleepMmsecs);

  apa.SetPixel(2, 0xFF, 0x8C, 0x00, 4);  // Orange
  apa.Display();
  usleep(sleepMmsecs);

  apa.SetPixel(1, 0xFF, 0x00, 0x00, 4);  // Red
  apa.Display();
  usleep(sleepMmsecs);

  for (int i=0; i < 6; i++) {
    apa.SetPixel(i + 1, 0x00, 0x00, 0x00, 0);
    apa.Display();
    usleep(sleepMmsecs);
  }

  apa.SetAll(0xFF, 0x00, 0x00, 4);  // Red
  apa.Display();
  usleep(sleepMmsecs);
  apa.SetAll(0xFF, 0x8C, 0x00, 4);  // Orange
  apa.Display();
  usleep(sleepMmsecs);
  apa.SetAll(0xFF, 0xFF, 0x00, 4);  // Yellow
  apa.Display();
  usleep(sleepMmsecs);
  apa.SetAll(0x00, 0xFF, 0x00, 4);  // Green
  apa.Display();
  usleep(sleepMmsecs);
  apa.SetAll(0x00, 0x00, 0xFF, 4);  // Blue
  apa.Display();
  usleep(sleepMmsecs);
  apa.SetAll(0xD4, 0x00, 0xD3, 4);  // Violet
  apa.Display();
  usleep(sleepMmsecs);

  apa.Clear();
  apa.Display();
}


/**
 * Drives APA102's without the wrapper.
 */
void TestSPI() {
  // Setup SPI device for interfacing with DotStar LEDs.
  SPI spi;
  spi.SetMode(SPI_CS_HIGH | SPI_CPHA);
  spi.SetSpeed(1000000);
  spi.SetBits(32);
  spi.SetDelay(0);

  // TODO(class): Get writer object.
  spi.Init();

  // Write bright blue pixel data to SPI.
  uint8_t tx_blue[] = {
    0x00, 0x00, 0x00, 0x00,
    0x48, 0xFF, 0x00, 0x00,
    0x48, 0xFF, 0x00, 0x00,
    0x48, 0xFF, 0x00, 0x00,
    0x48, 0xFF, 0x00, 0x00,
    0x48, 0xFF, 0x00, 0x00,
    0x48, 0xFF, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF,
  };

  uint8_t tx_green[] = {
    0x00, 0x00, 0x00, 0x00,
    0x48, 0x00, 0xFF, 0x00,
    0x48, 0x00, 0xFF, 0x00,
    0x48, 0x00, 0xFF, 0x00,
    0x48, 0x00, 0xFF, 0x00,
    0x48, 0x00, 0xFF, 0x00,
    0x48, 0x00, 0xFF, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF,
  };

  uint8_t tx_red[] = {
    0x00, 0x00, 0x00, 0x00,
    0x48, 0x00, 0x00, 0xE8,
    0x48, 0x00, 0x00, 0xE8,
    0x48, 0x00, 0x00, 0xE8,
    0x48, 0x00, 0x00, 0xE8,
    0x48, 0x00, 0x00, 0xE8,
    0x48, 0x00, 0x00, 0xE8,
    0xFF, 0xFF, 0xFF, 0xE8,
  };

  spi.Write(tx_blue, 32);
  usleep(500000);
  spi.Write(tx_green, 32);
  usleep(500000);
  spi.Write(tx_red, 32);

  // TODO(class): Free writer object.
  spi.Uninit();
}


void TestFace() {
  // TODO(class): Test I2C
}
