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
#ifndef _INCLUDE_GPIO_H
#define _INCLUDE_GPIO_H

enum GPIO {
  PIN_A = 938,  // PIN GPIO_A 9th pin from bottom (23)
  PIN_B = 914,  // PIN GPIO_B 9th pin from bottom (24)
  PIN_C = 915,  // PIN GPIO_C 8th pin from bottom (25)
  PIN_D = 971   // PIN GPIO_D 8th pin from bottom (26)
};

/**
 * Sets up GPIO for enabling pins A..D.
 */
void SetupGPIO();

/**
 * Opens a GPIO pin for access.
 *
 * @param gpioAddress The address of the GPIO pin.
 * @param isOut Set to true if you are exporting the pin as out.
 */
void OpenGPIO(GPIO gpioAddress, bool isOut);

/**
 * Writes a state to a GPIO pin.
 *
 * @param gpioAddress The address of the GPIO pin.
 * @param isHigh Set to true if you are writing a high value to the pin.
 */
void WriteGPIO(GPIO gpioAddress, bool isHigh);
#endif  // _INCLUDE_GPIO_H
