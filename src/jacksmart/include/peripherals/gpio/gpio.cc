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
#include "gpio.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void SetupGPIO() {
  OpenGPIO(GPIO::PIN_A, true);
  OpenGPIO(GPIO::PIN_B, true);
  OpenGPIO(GPIO::PIN_C, true);
  OpenGPIO(GPIO::PIN_D, true);
}


void OpenGPIO(GPIO gpioAddress, bool isOut) {
  int fd;
  char buf[128];

  // Enable GPIO for pin
  fd = open("/sys/class/gpio/export", O_WRONLY);
  snprintf(buf, sizeof(buf), "%d", gpioAddress);
  write(fd, buf, strlen(buf));
  close(fd);

  // Set the output mode for the GPIO pin.
  snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpioAddress);
  fd = open(buf, O_WRONLY);

  if (isOut) {
    write(fd, "out", 3);
  } else {
    write(fd, "in", 2);
  }

  close(fd);
}


void WriteGPIO(GPIO gpioAddress, bool isHigh) {
  int fd;
  char buf[128];

  // Write to the pin.
  snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpioAddress);
  fd = open(buf, O_WRONLY);
  if (isHigh) {
    snprintf(buf, sizeof(buf), "%d", 1);
  } else {
    snprintf(buf, sizeof(buf), "%d", 0);
  }
  write(fd, buf, strlen(buf));
  close(fd);
}
